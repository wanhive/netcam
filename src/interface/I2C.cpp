/*
 * I2C.cpp
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

#include "I2C.h"
#include <wanhive/wanhive-base.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
extern "C" {
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}

namespace wanhive {

I2C::I2C(unsigned int bus, unsigned int device) :
		fd(-1) {
	char path[32];
	snprintf(path, sizeof(path), "/dev/i2c-%u", bus);
	open(path, device);
}

I2C::I2C(const char *path, unsigned int device) :
		fd(-1) {
	open(path, device);
}

I2C::~I2C() {
	close();
}

unsigned int I2C::read(unsigned char command, unsigned int count,
		void *buffer) {
	auto rv = i2c_smbus_read_i2c_block_data(fd, command, count,
			(unsigned char*) buffer);
	if (rv < 0) {
		throw SystemException();
	} else {
		return rv;
	}
}

void I2C::write(unsigned char command, unsigned int count, const void *buffer) {
	if (i2c_smbus_write_i2c_block_data(fd, command, count,
			(const unsigned char*) buffer) < 0) {
		throw SystemException();
	}
}

void I2C::read(unsigned char command, unsigned char &value) {
	auto rv = i2c_smbus_read_byte_data(fd, command);
	if (rv < 0) {
		throw SystemException();
	} else {
		value = rv & 0xFF;
	}
}

unsigned char I2C::readByte(unsigned char command) {
	auto rv = i2c_smbus_read_byte_data(fd, command);
	if (rv < 0) {
		throw SystemException();
	} else {
		return (rv & 0xFF);
	}
}

void I2C::write(unsigned char command, unsigned char value) {
	if (i2c_smbus_write_byte_data(fd, command, value) < 0) {
		throw SystemException();
	}
}

void I2C::read(unsigned char command, unsigned short &value) {
	auto rv = i2c_smbus_read_word_data(fd, command);
	if (rv < 0) {
		throw SystemException();
	} else {
		value = rv & 0xFFFF;
	}
}

unsigned short I2C::readWord(unsigned char command) {
	auto rv = i2c_smbus_read_word_data(fd, command);
	if (rv < 0) {
		throw SystemException();
	} else {
		return (rv & 0xFFFF);
	}
}

void I2C::write(unsigned char command, unsigned short value) {
	if (i2c_smbus_write_word_data(fd, command, value) < 0) {
		throw SystemException();
	}
}

void I2C::read(unsigned char &value) {
	auto rv = i2c_smbus_read_byte(fd);
	if (rv < 0) {
		throw SystemException();
	} else {
		value = rv & 0xFF;
	}
}

unsigned char I2C::read() {
	auto rv = i2c_smbus_read_byte(fd);
	if (rv < 0) {
		throw SystemException();
	} else {
		return (rv & 0xFF);
	}
}

void I2C::write(unsigned char value) {
	if (i2c_smbus_write_byte(fd, value) < 0) {
		throw SystemException();
	}
}

void I2C::quickWrite(unsigned char value) {
	if (i2c_smbus_write_quick(fd, value) < 0) {
		throw SystemException();
	}
}

unsigned int I2C::process(unsigned char command, unsigned int count,
		void *buffer) {
	auto rv = i2c_smbus_block_process_call(fd, command, count,
			(unsigned char*) buffer);
	if (rv < 0) {
		throw SystemException();
	} else {
		return rv;
	}
}

unsigned short I2C::process(unsigned char command, unsigned short value) {
	auto rv = i2c_smbus_process_call(fd, command, value);
	if (rv < 0) {
		throw SystemException();
	} else {
		return (rv & 0xFFFF);
	}
}

void I2C::open(const char *path, unsigned int device) {
	static_assert(((sizeof(unsigned int) == 4) && (CHAR_BIT == 8)),
			"Unsupported platform");
	try {
		close(); //Just in case
		if ((fd = ::open(path, O_RDWR)) == -1) {
			throw SystemException();
		} else if (ioctl(fd, I2C_SLAVE, device) == -1) {
			throw SystemException();
		} else {
			//success
		}
	} catch (BaseException &e) {
		close();
		throw;
	}
}

void I2C::close() noexcept {
	if (fd != -1) {
		::close(fd);
	}
	fd = -1;
}

} /* namespace wanhive */
