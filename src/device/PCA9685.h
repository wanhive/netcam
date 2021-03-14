/*
 * PCA9685.h
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

#ifndef DEVICE_PCA9685_H_
#define DEVICE_PCA9685_H_
#include "../interface/I2C.h"

namespace wanhive {
/**
 * C++ implementation of user space PCA9685 driver
 * REF: http://www.nxp.com/documents/data_sheet/PCA9685.pdf
 */
class PCA9685: protected I2C {
public:
	PCA9685(unsigned int bus, unsigned int device = 0x40);
	PCA9685(const char *path, unsigned int device = 0x40);
	~PCA9685();
	/*
	 * Simple PWM control which sets on-tick to 0 and off-tick to value.
	 * If value is = 0, full-off will be enabled
	 * If value is >= PWM_MAX, full-on will be enabled
	 * Every value in between enables PWM output
	 */
	void pwmWrite(unsigned int pin, unsigned int value);
	/*
	 * Simple full-on and full-off control
	 * If value is false, full-off will be enabled
	 * If value is true, full-on will be enabled
	 */
	void digitalWrite(unsigned int pin, bool value);
	/*
	 * Sets the frequency of PWM signals.
	 * Frequency will be capped to range [40, 1000] Hz.
	 * Returns the frequency been set.
	 */
	unsigned int setFrequency(unsigned int frequency);
	/*
	 * Restarts the controller
	 */
	void restart();
	/*
	 * Puts the controller into sleep
	 */
	void sleep();
	/*
	 * Wakes up the controller
	 */
	void wakeUp();
	/*
	 * Write on and off ticks manually to a pin
	 * (Deactivates full-on and full-off)
	 */
	void write(unsigned int pin, unsigned short on, unsigned short off);
	/*
	 * Reads both on and off registers as 16 bit of data
	 * To get PWM: mask each value with 0xFFF
	 * To get full-on or full-off bit: mask with 0x1000
	 */
	void read(unsigned int pin, unsigned short &on, unsigned short &off);
	/*
	 * Activates or deactivates full-on
	 * flag = true: full-on
	 * flag = false: according to PWM
	 */
	void fullOn(unsigned int pin, bool flag);
	/*
	 * Activates or deactivates full-off
	 * flag = true: full-off
	 * flag = false: according to PWM or full-on
	 */
	void fullOff(unsigned int pin, bool flag);
	/*
	 * Sets the output mode. If <invert> is true then the output logic state is
	 * inverted. If <openDrain> is true then the outputs are configured with a
	 * open drain structure (totem pole otherwise).
	 */
	void setOutputMode(bool invert, bool openDrain);
private:
	void setup();
public:
	static constexpr unsigned int PWM_MAX = 4096;
	static constexpr unsigned int ALL_LED = 16;
	//Frequency range: [40-1000 Hz]
	static constexpr unsigned int MIN_FREQUENCY = 40;
	static constexpr unsigned int MAX_FREQUENCY = 1000;
};

} /* namespace wanhive */

#endif /* DEVICE_PCA9685_H_ */
