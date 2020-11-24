/*
 * I2C.h
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

#ifndef INTERFACE_I2C_H_
#define INTERFACE_I2C_H_

namespace wanhive {
/**
 * User space I2C bus driver (uses libi2c)
 */
class I2C {
public:
	//Initializes an I2C device at the given bus number
	I2C(unsigned int bus, unsigned int device);
	//Initialize an I2C device at the given pathname
	I2C(const char *path, unsigned int device);
	~I2C();

	/**
	 * Block read and write, maximum number of bytes are restricted
	 * by the SMBus specification.
	 */
	//Returns the number of read bytes
	unsigned int read(unsigned char command, unsigned int count, void *buffer);
	void write(unsigned char command, unsigned int count, const void *buffer);

	/**
	 * Read or write a byte
	 */
	void read(unsigned char command, unsigned char &value);
	unsigned char readByte(unsigned char command);
	void write(unsigned char command, unsigned char value);

	/**
	 * Read or write a word
	 */
	void read(unsigned char command, unsigned short &value);
	unsigned short readWord(unsigned char command);
	void write(unsigned char command, unsigned short value);

	/**
	 * Read and write for simple devices which don't have registers
	 */
	void read(unsigned char &value);
	unsigned char read();
	void write(unsigned char value);

	/**
	 * Quick write (for probing)
	 */
	void quickWrite(unsigned char value);
	/**
	 * Process calls
	 */
	//Returns number of read bytes
	unsigned int process(unsigned char command, unsigned int count,
			void *buffer);
	//Returns the unsigned word received from the device
	unsigned short process(unsigned char command, unsigned short value);
private:
	void open(const char *path, unsigned int device);
	void close() noexcept;
private:
	int fd;
};

} /* namespace wanhive */

#endif /* INTERFACE_I2C_H_ */
