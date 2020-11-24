/*
 * PCA9685.cpp
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

#include "PCA9685.h"
#include <wanhive/wanhive-base.h>

#define LED0_ON_L 0x6     //First LED
#define ALL_LED_ON_L 0xFA //All LED
#define PIN_COUNT 16      //Total number of pins

namespace {
/**
 * Locates the correct LEDX_ON_L register for the pin number starting at 0.
 */
unsigned int baseRegister(unsigned int pin) {
	if (pin < PIN_COUNT) {
		return LED0_ON_L + (pin * 4);
	} else {
		return ALL_LED_ON_L;
	}
}

}  // namespace

namespace wanhive {

PCA9685::PCA9685(unsigned int bus, unsigned int device) :
		I2C(bus, device) {
	setup();
}

PCA9685::PCA9685(const char *path, unsigned int device) :
		I2C(path, device) {
	setup();
}

PCA9685::~PCA9685() {

}

void PCA9685::pwmWrite(unsigned int pin, unsigned int value) {
	if (value >= PWM_MAX) {
		fullOn(pin, true);
	} else if (value > 0) {
		write(pin, 0, value);
	} else {
		fullOff(pin, true);
	}
}

void PCA9685::digitalWrite(unsigned int pin, bool value) {
	if (value) {
		fullOn(pin, true);
	} else {
		fullOff(pin, true);
	}
}

unsigned int PCA9685::setFrequency(unsigned int frequency) {
	/*
	 * Using the internal oscillator
	 * Limit the frequency to the range [MIN_FREQUENCY, MAX_FREQUENCY]
	 * Data sheet says 24Hz to 1526Hz
	 */
	frequency = (
			frequency < MIN_FREQUENCY ?
					MIN_FREQUENCY :
					(frequency > MAX_FREQUENCY ? MAX_FREQUENCY : frequency));
	unsigned char prescale = (unsigned char) (((OSCILLATOR_FREQUENCY
			/ (frequency * 4096)) + 0.5) - 1);

	if (prescale < PRESCALE_MIN) { //Just in case
		prescale = PRESCALE_MIN;
	}

	unsigned char state;
	I2C::read(MODE1_REG, state);
	//Clear the restart bit
	state &= ~RESTART_MASK;
	//Go to sleep (set the sleep bit)
	state |= SLEEP_MASK;
	I2C::write(MODE1_REG, state);
	//Set prescale
	I2C::write(PRESCALE_REG, prescale);
	//Wake up (clear the sleep bit)
	state &= ~SLEEP_MASK;
	I2C::write(MODE1_REG, state);
	//Allow the oscillator to stabilize
	Timer::sleep(1);
	//Restart PWM
	state |= (RESTART_MASK | AI_MASK);
	I2C::write(MODE1_REG, state);

	return frequency;
}

void PCA9685::restart() {
	unsigned char state;
	I2C::read(MODE1_REG, state);
	if (state & RESTART_MASK) {
		state &= ~RESTART_MASK;
		state &= ~SLEEP_MASK;
		I2C::write(MODE1_REG, state);
		Timer::sleep(1);
	}

	state |= RESTART_MASK;
	I2C::write(MODE1_REG, state);
}

void PCA9685::sleep() {
	unsigned char state;
	I2C::read(MODE1_REG, state);
	state |= SLEEP_MASK;
	I2C::write(MODE1_REG, state);
}

void PCA9685::wakeUp() {
	unsigned char state;
	I2C::read(MODE1_REG, state);
	state &= ~SLEEP_MASK;
	I2C::write(MODE1_REG, state);
	Timer::sleep(1);
}

void PCA9685::write(unsigned int pin, unsigned short on, unsigned short off) {
	//Use only the lowest 12 bits, clear the full-on and full-off bits.
	on &= 0x0FFF;
	off &= 0x0FFF;

	//Write to on and off registers
	auto reg = baseRegister(pin);
	I2C::write(reg, on);      //LEDX_ON
	I2C::write(reg + 2, off); //LEDX_OFF
}

void PCA9685::read(unsigned int pin, unsigned short &on, unsigned short &off) {
	auto reg = baseRegister(pin);
	I2C::read(reg, on);
	I2C::read(reg + 2, off);
}

void PCA9685::fullOn(unsigned int pin, bool flag) {
	auto reg = baseRegister(pin) + 1; //LEDX_ON_H
	unsigned char state;
	I2C::read(reg, state);
	state = Twiddler::mask(state, FULL_MASK, flag);
	I2C::write(reg, state);

	//Because full-off takes precedence
	if (flag) {
		fullOff(pin, false);
	}
}

void PCA9685::fullOff(unsigned int pin, bool flag) {
	auto reg = baseRegister(pin) + 3; //LEDX_OFF_H
	unsigned char state;
	I2C::read(reg, state);
	state = Twiddler::mask(state, FULL_MASK, flag);
	I2C::write(reg, state);
}

void PCA9685::setup() {
	unsigned char state;
	I2C::read(MODE1_REG, state);
	//Enable register auto-increment
	state = (state & ~RESTART_MASK) | AI_MASK;
	I2C::write(MODE1_REG, state);
}

} /* namespace wanhive */
