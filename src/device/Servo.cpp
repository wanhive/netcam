/*
 * Servo.cpp
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

#include "Servo.h"

namespace wanhive {

Servo::Servo(unsigned int adapter, unsigned int device) :
		PCA9685(adapter, device) {
	setFrequency(FREQUENCY);
}

Servo::Servo(const char *path, unsigned int device) :
		PCA9685(path, device) {
	setFrequency(FREQUENCY);
}

Servo::~Servo() {

}

void Servo::sendPulse(unsigned int pin, float millis) {
	auto period = 1000.0f / FREQUENCY;
	int value = (PWM_MAX * millis / period + 0.5f);
	value = (value >= 0) ? value : 0;
	pwmWrite(pin, value);
}

} /* namespace wanhive */
