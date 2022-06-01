/*
 * Streamer.h
 *
 * Copyright (C) 2020 Wanhive Systems Private Limited (info@wanhive.com)
 * This file is part of Wanhive Netcam.
 *
 * Wanhive Netcam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Wanhive Netcam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wanhive Netcam. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef CLIENT_STREAMER_H_
#define CLIENT_STREAMER_H_

#include "../device/Camera.h"
#include "../device/GPS.h"
#include "../device/Gimbal.h"
#include <wanhive/wanhive.h>

namespace wanhive {

class Streamer final: public ClientHub {
public:
	Streamer(unsigned long long uid, const char *path = nullptr) noexcept;
	virtual ~Streamer();
private:
	void configure(void *arg) override;
	void cleanup() noexcept override;
	void route(Message *message) noexcept override;
	void maintain() noexcept override;
	void processAlarm(unsigned long long uid, unsigned long long ticks) noexcept
			override;
	void sendImage() noexcept;
	//Handle an incoming pairing request
	int handlePairingRequest(Message *message) noexcept;
	//Handle an incoming position (PAN/TILT) request
	int handlePositionRequest(Message *message) noexcept;
	bool updateGeoLocation() noexcept;
	void resetGPS() noexcept;
	bool updatePanTilt(unsigned int pan, unsigned int tilt) noexcept;
	void initDevices();
	void clear() noexcept;
private:
	/*
	 * Devices
	 */
	struct {
		Camera *camera;
		GPS *gps;
		Gimbal *gimbal;
	} devices;

	struct {
		unsigned long long id; //current peer's identifier
		unsigned long long frames; //number of frames requested
	} peer;

	GeoLocation location;
	struct {
		const char *cameraName;
		unsigned jpegQuality;
		bool gps;
		bool servo;
	} ctx;

	FlowControl flow; //Flow control
};

} /* namespace wanhive */

#endif /* CLIENT_STREAMER_H_ */
