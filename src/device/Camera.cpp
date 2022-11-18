/*
 * Camera.cpp
 *
 * Copyright (C) 2020 Wanhive Systems Private Limited (info@wanhive.com)
 *
 * SPDX License Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "Camera.h"
#include <wanhive/wanhive-base.h>

namespace wanhive {

Camera::Camera(const char *name) noexcept {
	device.width = 0;
	device.height = 0;
	device.index = 0;
	device.name = name;
}

Camera::Camera(int index) noexcept {
	device.width = 0;
	device.height = 0;
	device.index = index;
	device.name = nullptr;
}

Camera::~Camera() {

}

void Camera::read(unsigned int quality) {
	try {
		open();
		device.vcap >> frame;
		if (!frame.empty()) {
			device.params[0] = cv::IMWRITE_JPEG_QUALITY;
			device.params[1] = (quality <= 100 ? quality : 100);
			cv::imencode(".jpg", frame, buffer, device.params);
		} else {
			throw Exception(EX_RESOURCE);
		}
	} catch (BaseException &e) {
		close();
		throw;
	} catch (...) {
		close();
		throw Exception(EX_RESOURCE);
	}
}

unsigned const char* Camera::getFrame(unsigned int &bytes) const noexcept {
	if (device.vcap.isOpened() && buffer.size()) {
		bytes = buffer.size();
		return buffer.data();
	} else {
		bytes = 0;
		return nullptr;
	}
}

void Camera::setResolution(unsigned int width, unsigned int height) {
	try {
		if (!device.vcap.isOpened()) {
			device.width = width;
			device.height = height;
		} else {
			if (width && height) {
				//Try to set new resolution
				device.vcap.set(cv::CAP_PROP_FRAME_WIDTH, width);
				device.vcap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
			}
			//Get current resolution
			device.width = device.vcap.get(cv::CAP_PROP_FRAME_WIDTH);
			device.height = device.vcap.get(cv::CAP_PROP_FRAME_HEIGHT);
		}
	} catch (...) {
		device.width = 0;
		device.height = 0;
		throw Exception(EX_RESOURCE);
	}
}

unsigned int Camera::getWidth() const noexcept {
	if (device.vcap.isOpened()) {
		return device.width;
	} else {
		return 0;
	}
}

unsigned int Camera::getHeight() const noexcept {
	if (device.vcap.isOpened()) {
		return device.height;
	} else {
		return 0;
	}
}

void Camera::reset() noexcept {
	this->close();
}

void Camera::open() {
	try {
		if (device.vcap.isOpened()) {
			return;
		} else if (device.name && device.vcap.open(device.name)) {
			device.vcap.set(cv::CAP_PROP_BUFFERSIZE, 3);
			setResolution(device.width, device.height);
		} else if (!device.name && device.vcap.open(device.index)) {
			device.vcap.set(cv::CAP_PROP_BUFFERSIZE, 3);
			setResolution(device.width, device.height);
		} else {
			throw Exception(EX_RESOURCE);
		}
	} catch (BaseException &e) {
		close();
		throw;
	} catch (...) {
		close();
		throw Exception(EX_RESOURCE);
	}
}

void Camera::close() noexcept {
	try {
		device.vcap.release();
	} catch (...) {
	}
}

} /* namespace wanhive */
