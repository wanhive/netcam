/*
 * GPS.h
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

#ifndef DEVICE_GPS_H_
#define DEVICE_GPS_H_
#include <gps.h>

namespace wanhive {
struct GeoLocation {
	unsigned int mode; //2D lock (2); 3D Lock (3)
	double timestamp; //Unix timestamp
	double latitude; //Latitude
	double longitude; //Longitude
	double altitude; //Altitude over mean sea level (meter)
	double speed; //Speed (meter/second)
	double heading; //Heading wrt true North
	double climb; //Climb (meter/second)
};
/**
 * GPS driver (uses libgps and gpsd)
 */
class GPS {
public:
	GPS() noexcept;
	virtual ~GPS();

	/*
	 * Reads data from the gpsd service into the <location> structure.
	 * Returns true on success, false otherwise.
	 */
	bool read(GeoLocation &location) noexcept;
	/*
	 * Resets the object (disconnects from the gpsd service).
	 */
	void reset() noexcept;
private:
	void getData(GeoLocation &location) const noexcept;
	bool hasData() noexcept;
	bool isConnected() noexcept;
	bool connect() noexcept;
	void disconnect() noexcept;
private:
	bool connected;
	gps_data_t data;
	char message[8192];
};

} /* namespace wanhive */

#endif /* DEVICE_GPS_H_ */
