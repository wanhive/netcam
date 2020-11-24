/*
 * Gimbal.h
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

#ifndef DEVICE_GIMBAL_H_
#define DEVICE_GIMBAL_H_

#include "Servo.h"

namespace wanhive {
/**
 * 3-axis gimbal implementation
 * Uses standard servo to fix the orientation.
 * PCA9685 pin map: Pan(0), Roll(2), Tilt(4)
 */
class Gimbal: private Servo {
public:
	Gimbal();
	virtual ~Gimbal();

	unsigned int getPan() const noexcept;
	void setPan(unsigned int pan);
	unsigned int getRoll() const noexcept;
	void setRoll(unsigned int roll);
	unsigned int getTilt() const noexcept;
	void setTilt(unsigned int tilt);

	//Centers the gimbal
	void reset();
public:
	static constexpr unsigned int PAN_MIN = 0;
	static constexpr unsigned int PAN_MAX = 180;
	static constexpr unsigned int ROLL_MIN = 0;
	static constexpr unsigned int ROLL_MAX = 180;
	static constexpr unsigned int TILT_MIN = 0;
	static constexpr unsigned int TILT_MAX = 180;
private:
	unsigned int pan { 0 };
	unsigned int roll { 0 };
	unsigned int tilt { 0 };
};

} /* namespace wanhive */

#endif /* DEVICE_GIMBAL_H_ */
