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
#include <cmath>
#include <cstring>

#if GPSD_API_MAJOR_VERSION < 10
#error "Incompatible GPSD API version."
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
		return false;
	} else if ((nRead = gps_read(&data, message, sizeof(message))) == -1) {
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
	memset(&location, 0, sizeof(location));
	const auto &fix = data.fix;
	location.timestamp = fix.time.tv_sec + (fix.time.tv_nsec / 1000000000.0);
	if (std::isfinite(fix.latitude)) {
		location.latitude = fix.latitude;
	}
	if (std::isfinite(fix.longitude)) {
		location.longitude = fix.longitude;
	}
	if (std::isfinite(fix.altMSL)) {
		location.altitude = fix.altMSL;
	}
	if (std::isfinite(fix.speed)) {
		location.speed = fix.speed;
	}
	if (std::isfinite(fix.track)) {
		location.heading = fix.track;
	}
	if (std::isfinite(fix.climb)) {
		location.climb = fix.climb;
	}
	location.mode = fix.mode;
}

bool GPS::hasData() noexcept {
	return isConnected() && data.set && (data.fix.status != STATUS_NO_FIX)
			&& (data.fix.mode == MODE_2D || data.fix.mode == MODE_3D);
}

bool GPS::isConnected() noexcept {
	return connected;
}

bool GPS::connect() noexcept {
	disconnect();
	if (gps_open(GPSD_SHARED_MEMORY, nullptr, &data) == -1) {
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
