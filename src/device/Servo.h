/*
 * Servo.h
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

#ifndef DEVICE_SERVO_H_
#define DEVICE_SERVO_H_
#include "PCA9685.h"

namespace wanhive {
/**
 * PCA9685 servo controller
 */
class Servo: private PCA9685 {
public:
	Servo(unsigned int adapter, unsigned int device);
	Servo(const char *path, unsigned int device);
	~Servo();
	//1.5ms pulse centers the servo
	void sendPulse(unsigned int pin, float millis = 1.5);
public:
	static constexpr unsigned int FREQUENCY = 50;
};

} /* namespace wanhive */

#endif /* DEVICE_SERVO_H_ */
