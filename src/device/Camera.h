/*
 * Camera.h
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

#ifndef DEVICE_CAMERA_H_
#define DEVICE_CAMERA_H_
#include <opencv2/opencv.hpp>

namespace wanhive {
/**
 * USB Webcam capture driver based on opencv
 */
class Camera {
public:
	Camera(const char *name) noexcept;
	Camera(int index) noexcept;
	virtual ~Camera();

	void read(unsigned int quality);
	unsigned const char* getFrame(unsigned int &bytes) const noexcept;
	void setResolution(unsigned int width, unsigned int height);
	unsigned int getWidth() const noexcept;
	unsigned int getHeight() const noexcept;
	void reset() noexcept;
private:
	void open();
	void close() noexcept;

private:
	struct {
		cv::VideoCapture vcap;
		std::vector<int> params { 2 };
		unsigned int width;
		unsigned int height;
		int index;
		const char *name;
	} device;

	cv::Mat frame;
	std::vector<uchar> buffer;
};

} /* namespace wanhive */

#endif /* DEVICE_CAMERA_H_ */
