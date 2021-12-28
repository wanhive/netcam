/*
 * Viewer.cpp
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

/*
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * SPDX-License-Identifier: BSD-2-clause
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 *
 */

#include "Viewer.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace {
/**
 * Copied from the GPSD project
 */
char* unixToIso8601(double fixtime, char isotime[], size_t len) {
	struct tm when;
	double integral, fractional;
	time_t intfixtime;
	char timestr[30];
	char fractstr[10];

	if (!std::isfinite(fixtime)) {
		return strncpy(isotime, "NaN", len);
	}
	fractional = modf(fixtime, &integral);
	/* snprintf rounding of %3f can get ugly, so pre-round */
	if (0.999499999 < fractional) {
		/* round up */
		integral++;
		/* give the fraction a nudge to ensure rounding */
		fractional += 0.0005;
	}
	intfixtime = (time_t) integral;

	(void) gmtime_r(&intfixtime, &when);
	(void) strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", &when);
	/*
	 * Do not mess casually with the number of decimal digits in the
	 * format!  Most GPSes report over serial links at 0.01s or 0.001s
	 * precision.
	 */
	(void) snprintf(fractstr, sizeof(fractstr), "%.3f", fractional);
	/* add fractional part, ignore leading 0; "0.2" -> ".2" */
	(void) snprintf(isotime, len, "%s%sZ", timestr, strchr(fractstr, '.'));
	return isotime;
}

}  // namespace

namespace wanhive {

Viewer::Viewer(unsigned long long uid, unsigned long long streamerId,
		const char *path) noexcept :
		ClientHub(uid, path) {
	clear();
	peer.id = streamerId;
	flow.setSource(uid);
}

Viewer::~Viewer() {

}

void Viewer::configure(void *arg) {
	try {
		ClientHub::configure(arg);
		sink.writeVideo = getConfiguration().getBoolean("NETCAM", "writeVideo");
		sink.fourcc = getConfiguration().getString("NETCAM", "fourcc", "MJPG");
		WH_LOG_DEBUG("Viewer settings:\n""WRITE_VIDEO=%s, FORMAT=%s",
				(sink.writeVideo ? "YES" : "NO"), sink.fourcc);
	} catch (BaseException &e) {
		WH_LOG_EXCEPTION(e);
		throw;
	}
}

void Viewer::cleanup() noexcept {
	clear();
	ClientHub::cleanup();
}

void Viewer::route(Message *message) noexcept {
	if (!isConnected()) {
		ClientHub::route(message);
		return;
	}

	auto sequenceNo = message->getSequenceNumber();
	auto session = message->getSession();
	auto cmd = message->getCommand();
	auto qlf = message->getQualifier();
	auto status = message->getStatus();
	//UID is the sink
	message->setDestination(getUid());

	switch (session) {
	case 0:
		if (cmd == 0 && qlf == 0 && status == WH_AQLF_ACCEPTED) {
			handlePairingResponse(message);
		}
		break;
	case 1:
		if (message->getSource() != image.source) {
			return;
		} else if (cmd == 0 && qlf == 0 && status == WH_AQLF_REQUEST) {
			processImage(); //Process the image received in previous cycle
			resetFrame(message->getData32(0),
					message->getData32(sizeof(uint32_t)),
					message->getData32(2 * sizeof(uint32_t)), sequenceNo);
		} else if (cmd == 0 && qlf == 1 && status == WH_AQLF_REQUEST
				&& image.sequence == sequenceNo) {
			populateFrame(message->getBytes(0), message->getPayloadLength());
		}
		break;
	default:
		break;
	}
}

void Viewer::maintain() noexcept {
	if (!isConnected()) {
		ClientHub::maintain();
	} else {
		return;
	}
}

void Viewer::processClockNotification(unsigned long long uid,
		unsigned long long ticks) noexcept {
	if (!isConnected()) {
		hideWindow();
		return;
	}

	//-----------------------------------------------------------------
	unsigned int expiration = 0;
	unsigned int interval = 0;
	getClockSettings(expiration, interval);
	if (interval) {
		WH_LOG_DEBUG("Current frame rate: %f frames/s",
				((double )image.frames * 1000) / interval);
		if ((unsigned int) image.frames == 0) {
			hideWindow();
		}
		image.frames = 0; //Reset for the next cycle
	}

	unsigned int frames = (((double) image.frameRate) * interval) / 825;
	sendHeartbeat(peer.id, frames);
}

void Viewer::sendHeartbeat(unsigned long long id, unsigned int frames) noexcept {
	Message *message = Message::create();
	if (message) {
		peer.sequence = flow.nextSequenceNumber();
		message->putHeader(0, id, Message::HEADER_SIZE, peer.sequence, 0, 0, 0,
				WH_AQLF_REQUEST);
		message->appendData32(frames); //Request new frames
		message->setDestination(0); //Route via overlay network
		sendMessage(message);
	}
}

bool Viewer::resetSource(unsigned long long id, unsigned int frameRate,
		unsigned int sequence) noexcept {
	if (!(id == peer.id && peer.sequence == sequence)) {
		return false;
	} else if (!(image.source == id && image.frameRate == frameRate)) {
		//Source or frame rate changed
		memset(&image, 0, sizeof(image));
		image.source = id;
		image.frameRate = frameRate;

		memset(&gimbal, 0, sizeof(gimbal));
		sink.reset = true;
		return true;
	} else {
		//image.frames = 0;
		return true;
	}
}

void Viewer::resetFrame(unsigned int size, unsigned int width,
		unsigned int height, unsigned int sequenceNumber) noexcept {
	++image.frames;
	if (size > sizeof(image.data) || !sequenceNumber) {
		image.sequence = 0;
		image.size = 0;
		WH_LOG_DEBUG("Invalid frame");
	} else {
		image.sequence = sequenceNumber;
		image.size = size;
		image.bytes = 0;

		if (image.width != width || image.height != height) {
			//Image dimensions changed
			image.width = width;
			image.height = height;
			sink.reset = true;
		}
	}
}

void Viewer::populateFrame(const unsigned char *data,
		unsigned int bytes) noexcept {
	if (data && (image.bytes + bytes) <= image.size) {
		memcpy(image.data + image.bytes, data, bytes);
		image.bytes += bytes;
	}
}

void Viewer::processImage() noexcept {
	try {
		if (image.size && (image.size == image.bytes)) {
			resetSink();
			std::vector<uchar> jpeg(image.data, image.data + image.bytes);
			cv::Mat img = cv::imdecode(jpeg, cv::IMREAD_ANYCOLOR);

			if (sink.writer.isOpened()) {
				sink.writer.write(img);
			}

			char text[256];
			if (location.show) {
				unixToIso8601(location.timestamp, text, sizeof(text));
				cv::putText(img, text, cv::Point2f(10, 100),
						cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6,
						cv::Scalar(225, 80, 80));
				snprintf(text, sizeof(text), "Latitude: %f", location.latitude);
				cv::putText(img, text, cv::Point2f(10, 120),
						cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6,
						cv::Scalar(0, 0, 255));
				snprintf(text, sizeof(text), "Longitude: %f",
						location.longitude);
				cv::putText(img, text, cv::Point2f(10, 140),
						cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6,
						cv::Scalar(0, 0, 255));
			} else {
				snprintf(text, sizeof(text), "%llu @ %ufps", image.source,
						image.frameRate);
				cv::putText(img, text, cv::Point2f(10, 100),
						cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6,
						cv::Scalar(225, 80, 80));
				snprintf(text, sizeof(text), "[%d x %d]", img.cols, img.rows);
				cv::putText(img, text, cv::Point2f(10, 120),
						cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6,
						cv::Scalar(0, 0, 255));
			}
			cv::imshow(sink.name, img);
			auto keyCode = cv::waitKey(10) & 0xFF;
			processKeyPress(keyCode);
		}
	} catch (BaseException &e) {
		WH_LOG_EXCEPTION(e);
		WH_LOG_DEBUG("Failed to process the image");
	} catch (...) {
		WH_LOG_EXCEPTION_U();
		WH_LOG_DEBUG("Failed to process the image");
	}
}

int Viewer::handlePairingResponse(Message *message) noexcept {
	if (message->getPayloadLength() >= sizeof(uint32_t)
			&& resetSource(message->getSource(), message->getData32(0),
					message->getSequenceNumber())) {
		WH_LOG_DEBUG("Source %llu streaming at %u frames/s", image.source,
				image.frameRate);
		if (message->getPayloadLength() >= sizeof(uint32_t) + 60) {
			location.timestamp = message->getDouble(8);
			location.latitude = message->getDouble(16);
			location.longitude = message->getDouble(24);
			WH_LOG_DEBUG("Source %llu reported TS:%f, LAT:%f, LONG:%f",
					image.source, location.timestamp, location.latitude,
					location.longitude);
		}
	}
	return 0;
}

void Viewer::resetSink() {
	try {
		if (sink.reset) {
			hideWindow();
			memset(sink.name, 0, sizeof(sink.name));
			snprintf(sink.name, sizeof(sink.name),
					"Stream %llu [%u x %u] @%u frames/s", image.source,
					image.width, image.height, image.frameRate);

			if (!sink.writeVideo) {
				sink.reset = false;
				return;
			}

			if (!sink.fourcc || strlen(sink.fourcc) != 4) {
				throw Exception(EX_INVALIDPARAM);
			}

			char t[32];
			memset(t, 0, sizeof(t));
			if (!Timer::print(t, sizeof(t))) {
				throw Exception(EX_INVALIDOPERATION);
			}

			memset(sink.fileName, 0, sizeof(sink.fileName));
			snprintf(sink.fileName, sizeof(sink.fileName),
					"c%llu-r%u_x_%u-f%u-t%s.avi", image.source, image.width,
					image.height, image.frameRate, t);

			if (!sink.writer.open(sink.fileName,
					sink.writer.fourcc(sink.fourcc[0], sink.fourcc[1],
							sink.fourcc[2], sink.fourcc[3]), image.frameRate,
					cv::Size(image.width, image.height), true)) {
				throw Exception(EX_INVALIDOPERATION);
			}
			sink.reset = false;
		}
	} catch (BaseException &e) {
		throw;
	} catch (...) {
		throw Exception(EX_INVALIDOPERATION);
	}
}

void Viewer::processKeyPress(int keyCode) {
	if (keyCode == 255) {
		return;
	}

	switch (keyCode) {
	case 'w': //UP
	case 'W':
		gimbal.tilt += 5;
		if (gimbal.tilt > 60) {
			gimbal.tilt = 60;
			return;
		}
		break;
	case 's': //DOWN
	case 'S':
		gimbal.tilt -= 5;
		if (gimbal.tilt < -60) {
			gimbal.tilt = -60;
			return;
		}
		break;
	case 'd': //RIGHT
	case 'D':
		gimbal.pan -= 5;
		if (gimbal.pan < -85) {
			gimbal.pan = -85;
			return;
		}
		break;
	case 'a': //LEFT
	case 'A':
		gimbal.pan += 5;
		if (gimbal.pan > 85) {
			gimbal.pan = 85;
			return;
		}
		break;
	case 'l':
	case 'L':
		location.show = location.show ? false : true; //toggle
		return;
	default:
		return;
	}

	Message *message = Message::create();
	if (message) {
		message->putHeader(0, peer.id, Message::HEADER_SIZE,
				flow.nextSequenceNumber(), 0, 0, 1, WH_AQLF_REQUEST);
		message->appendData32(gimbal.pan + 90);
		message->appendData32(gimbal.tilt + 90);
		message->setDestination(0); //Route via overlay network
		sendMessage(message);
	}
}

void Viewer::hideWindow() noexcept {
	try {
		if (sink.name[0]) {
			cv::destroyWindow(sink.name);
		}
	} catch (...) {
	}
}

void Viewer::clear() noexcept {
	memset(&peer, 0, sizeof(peer));
	memset(&image, 0, sizeof(image));
	memset(&gimbal, 0, sizeof(gimbal));
	memset(&location, 0, sizeof(location));
}

} /* namespace wanhive */
