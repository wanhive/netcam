/*
 * Gimbal.cpp
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

#include "Gimbal.h"

#ifndef WH_GIMBAL_ADAPTER
#define WH_GIMBAL_ADAPTER 1
#endif

#ifndef WH_GIMBAL_DEVICE
#define WH_GIMBAL_DEVICE 0x40
#endif

namespace wanhive {

Gimbal::Gimbal() :
		Servo(WH_GIMBAL_ADAPTER, WH_GIMBAL_DEVICE) {
}

Gimbal::~Gimbal() {

}

unsigned int Gimbal::getPan() const noexcept {
	return pan;
}

void Gimbal::setPan(unsigned int pan) {
	if (this->pan != pan && pan <= PAN_MAX) {
		sendPulse(0, (((float) pan) / 90) + 0.5);
		this->pan = pan;
	}
}

unsigned int Gimbal::getRoll() const noexcept {
	return roll;
}

void Gimbal::setRoll(unsigned int roll) {
	if (this->roll != roll && roll <= ROLL_MAX) {
		sendPulse(2, (((float) roll) / 90) + 0.5);
		this->roll = roll;
	}
}

unsigned int Gimbal::getTilt() const noexcept {
	return tilt;
}

void Gimbal::setTilt(unsigned int tilt) {
	if (this->tilt != tilt && tilt <= TILT_MAX) {
		sendPulse(4, (((float) tilt) / 90) + 0.5);
		this->tilt = tilt;
	}
}

void Gimbal::reset() {
	setPan(90);
	setRoll(90);
	setTilt(90);
}

} /* namespace wanhive */
