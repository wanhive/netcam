/*
 * Streamer.cpp
 *
 * Copyright (C) 2020 Wanhive Systems Private Limited (info@wanhive.com)
 * This file is part of Wanhive Netcam.
 *
 * Wanhive Netcam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Wanhive Netcam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wanhive Netcam. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "Streamer.h"

namespace wanhive {

Streamer::Streamer(unsigned long long uid, const char *path) noexcept :
		ClientHub(uid, path) {
	memset(&devices, 0, sizeof(devices));
	clear();
	flow.setSource(uid);
}

Streamer::~Streamer() {

}

void Streamer::configure(void *arg) {
	try {
		ClientHub::configure(arg);

		ctx.cameraName = getConfiguration().getString("NETCAM", "cameraName");
		ctx.jpegQuality = getConfiguration().getNumber("NETCAM", "jpegQuality",
				60);
		ctx.jpegQuality = (ctx.jpegQuality > 100) ? 100 : ctx.jpegQuality;

		ctx.gps = getConfiguration().getBoolean("NETCAM", "gps");
		ctx.servo = getConfiguration().getBoolean("NETCAM", "servo");

		WH_LOG_DEBUG(
				"Streamer settings:\n""CAMERA=%s, JPEGQUALITY=%u, GPS=%s, SERVO=%s",
				ctx.cameraName, ctx.jpegQuality, (ctx.gps ? "YES" : "NO"),
				(ctx.servo ? "YES" : "NO"));
		initDevices();
	} catch (BaseException &e) {
		WH_LOG_EXCEPTION(e);
		throw;
	} catch (...) {
		WH_LOG_EXCEPTION_U();
		throw Exception(EX_INVALIDOPERATION);
	}
}

void Streamer::cleanup() noexcept {
	clear();
	ClientHub::cleanup();
}

void Streamer::route(Message *message) noexcept {
	if (!isConnected()) {
		ClientHub::route(message);
		return;
	}

	auto session = message->getSession();
	auto cmd = message->getCommand();
	auto qlf = message->getQualifier();
	auto status = message->getStatus();
	//Always do this (UID is the sink)
	message->setDestination(getUid());

	//All external requests must have [session = 0]
	if (session != 0) {
		return;
	} else if (cmd == 0 && qlf == 0 && status == WH_AQLF_REQUEST) {
		handlePairingRequest(message); //Stream request
	} else if (cmd == 0 && qlf == 1 && status == WH_AQLF_REQUEST) {
		handlePositionRequest(message); //Pan/Tilt update request
	}
}

void Streamer::maintain() noexcept {
	if (!isConnected()) {
		ClientHub::maintain();
	} else {
		return;
	}
}

void Streamer::processClockNotification(unsigned long long uid,
		unsigned long long ticks) noexcept {
	try {
		if (isConnected() && peer.id && peer.frames) {
			sendImage(); //For tighter timing
			devices.camera->read(ctx.jpegQuality);
			updateGeoLocation();
		} else {
			resetGPS();
		}
	} catch (...) {
		WH_LOG_DEBUG("Capture device not ready");
		cancel();
	}
}

void Streamer::sendImage() noexcept {
	unsigned int bytes = 0;
	auto data = devices.camera->getFrame(bytes);

	//Number of messages needed
	unsigned int count = (bytes + Message::PAYLOAD_SIZE - 1)
			/ Message::PAYLOAD_SIZE;
	if (!count || !Message::available(count + 1)) { //+1 for metadata
		return;
	}
	//-----------------------------------------------------------------
	auto sequenceNo = flow.nextSequenceNumber();
	//-----------------------------------------------------------------
	/**
	 * JPEG frames sent on session 1
	 */
	Message *message = Message::create(); //Frame metadata
	MessageHeader header(0, peer.id, Message::HEADER_SIZE, sequenceNo, 1, 0, 0,
			WH_AQLF_REQUEST); //Frame metadata context
	message->putHeader(header);
	message->appendData32(bytes);
	message->appendData32(devices.camera->getWidth());
	message->appendData32(devices.camera->getHeight());
	message->setDestination(0); //Route via overlay network
	sendMessage(message);

	header.setContext( { 0, 1, WH_AQLF_REQUEST }); //Frame data context
	for (; count != 0; --count) {
		auto toSend = Twiddler::min(bytes, Message::PAYLOAD_SIZE);
		message = Message::create();
		message->putHeader(header);
		message->appendBytes(data, toSend);
		data += toSend;
		bytes -= toSend;

		message->setDestination(0); //Route via overlay network
		sendMessage(message);
	}
	--peer.frames;
}

int Streamer::handlePairingRequest(Message *message) noexcept {
	if (message->getPayloadLength() < sizeof(uint32_t)) {
		return -1;
	}
	peer.id = message->getSource();
	peer.frames = message->getData32(0);
	WH_LOG_DEBUG("Node %llu requested %u jpeg frames", peer.id, peer.frames);
	//-----------------------------------------------------------------
	//resetGPS();
	//-----------------------------------------------------------------
	/*
	 * Send acknowledgement
	 */
	unsigned int expiration = 0;
	unsigned int interval = 0;
	getClockSettings(expiration, interval);
	if (interval) {
		message->setData32(0, 1000 / interval);
	} else {
		message->setData32(0, 0);
	}
	message->putLength(Message::HEADER_SIZE + sizeof(uint32_t));
	//-----------------------------------------------------------------
	/*
	 * Append GPS data if available
	 */
	if (location.mode == 2 || location.mode == 3) {
		message->appendData32(location.mode);
		message->appendDouble(location.timestamp);
		message->appendDouble(location.latitude);
		message->appendDouble(location.longitude);
		message->appendDouble(location.altitude);
		message->appendDouble(location.speed);
		message->appendDouble(location.heading);
		message->appendDouble(location.climb);
		location.mode = 0;
	}
	//-----------------------------------------------------------------
	message->updateDestination(peer.id);
	message->setDestination(0);
	message->putStatus(WH_AQLF_ACCEPTED);
	return 0;
}

int Streamer::handlePositionRequest(Message *message) noexcept {
	if (message->getPayloadLength() < sizeof(uint32_t) * 2) {
		return -1;
	}

	auto pan = message->getData32(0);
	auto tilt = message->getData32(sizeof(uint32_t));
	updatePanTilt(pan, tilt);
	return 0; //no response sent back
}

bool Streamer::updateGeoLocation() noexcept {
	return (ctx.gps && devices.gps && devices.gps->read(location));
}

void Streamer::resetGPS() noexcept {
	if (ctx.gps && devices.gps) {
		devices.gps->reset();
		location.mode = 0;
	}
}

bool Streamer::updatePanTilt(unsigned int pan, unsigned int tilt) noexcept {
	try {
		WH_LOG_DEBUG("PAN: %u, TILT: %u", pan, tilt);
		if (!ctx.servo || !devices.gimbal) {
			return false;
		} else if (pan > 180 || tilt > 180) {
			return false;
		} else {
			devices.gimbal->setPan(pan);
			devices.gimbal->setTilt(tilt);
			return true;
		}
	} catch (BaseException &e) {
		WH_LOG_EXCEPTION(e);
		delete devices.gimbal;
		devices.gimbal = nullptr;
		ctx.servo = false;
		return false;
	} catch (...) {
		WH_LOG_EXCEPTION_U();
		delete devices.gimbal;
		devices.gimbal = nullptr;
		ctx.servo = false;
		return false;
	}
}

void Streamer::initDevices() {
	try {
		if (ctx.cameraName != nullptr) {
			devices.camera = new Camera(ctx.cameraName);
		} else {
			devices.camera = new Camera(-1);
		}

		WH_LOG_DEBUG("Camera installed");

		//Enable GPS
		if (ctx.gps) {
			devices.gps = new GPS();
			WH_LOG_DEBUG("GPS installed");
		}

		if (ctx.servo) {
			devices.gimbal = new Gimbal();
			devices.gimbal->reset();
			WH_LOG_DEBUG("Gimbal installed");
		}
	} catch (BaseException &e) {
		WH_LOG_EXCEPTION(e);
		throw;
	} catch (...) {
		WH_LOG_EXCEPTION_U();
		throw Exception(EX_ALLOCFAILED);
	}
}

void Streamer::clear() noexcept {
	delete devices.camera;
	delete devices.gps;
	delete devices.gimbal;

	memset(&devices, 0, sizeof(devices));
	memset(&peer, 0, sizeof(peer));
	memset(&location, 0, sizeof(GeoLocation));
	memset(&ctx, 0, sizeof(ctx));
}

} /* namespace wanhive */
