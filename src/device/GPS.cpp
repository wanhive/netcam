/*
 * GPS.cpp
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

#include "GPS.h"
#include <math.h>

#if GPSD_API_MAJOR_VERSION >= 7
#define WH_GPS_READ(X, Y, Z) gps_read(X, Y, Z)
#else
#define WH_GPS_READ(X, Y, Z) gps_read(X)
#endif

namespace wanhive {

GPS::GPS() noexcept :
		connected(false) {
}

GPS::~GPS() {
	disconnect();
}

bool GPS::read(GeoLocation &location) noexcept {
	int nRead = 0;
	message[0] = 0;
	if (!connected && !connect()) {
		perror(nullptr);
		return false;
	} else if ((nRead = WH_GPS_READ(&data, message, sizeof(message))) == -1) {
		perror(nullptr);
		disconnect();
		return false;
	} else if (nRead == 0) {
		return false;
	} else if (hasData()) {
		getData(location);
		return true;
	} else {
		return false;
	}
}

void GPS::reset() noexcept {
	disconnect();
}

void GPS::getData(GeoLocation &location) const noexcept {
	const gps_fix_t &fix = data.fix;
#if GPSD_API_MAJOR_VERSION >= 9
	location.timestamp = fix.time.tv_sec + (fix.time.tv_nsec / 1000000000.0);
#else
		if (!isnan(fix.time)) {
			location.timestamp = fix.time;
		}
#endif
	if (!isnan(fix.latitude)) {
		location.latitude = fix.latitude;
	}
	if (!isnan(fix.longitude)) {
		location.longitude = fix.longitude;
	}
#if GPSD_API_MAJOR_VERSION >= 9
	if (!isnan(fix.altMSL)) {
		location.altitude = fix.altMSL;
	}
#else
		if (!isnan(fix.altitude)) {
			location.altitude = fix.altitude;
		}
#endif
	if (!isnan(fix.speed)) {
		location.speed = fix.speed;
	}
	if (!isnan(fix.track)) {
		location.heading = fix.track;
	}
	if (!isnan(fix.climb)) {
		location.climb = fix.climb;
	}
	location.mode = fix.mode;
}

bool GPS::hasData() noexcept {
	return isConnected() && data.set && (data.status != STATUS_NO_FIX)
			&& (data.fix.mode == MODE_2D || data.fix.mode == MODE_3D);
}

bool GPS::isConnected() noexcept {
	return connected;
}

bool GPS::connect() noexcept {
	disconnect();
	if (gps_open("localhost", DEFAULT_GPSD_PORT, &data) == -1) {
		return false;
	} else {
		gps_stream(&data, WATCH_ENABLE, nullptr);
		connected = true;
		return true;
	}
}

void GPS::disconnect() noexcept {
	if (connected) {
		gps_close(&data);
		connected = false;
	}
}

} /* namespace wanhive */
