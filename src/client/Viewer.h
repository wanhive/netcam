/*
 * Viewer.h
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

#ifndef CLIENT_VIEWER_H_
#define CLIENT_VIEWER_H_
#include <wanhive/wanhive.h>
#include <opencv2/opencv.hpp>

namespace wanhive {

class Viewer: public ClientHub {
public:
	Viewer(unsigned long long uid, const char *path = nullptr) noexcept;
	virtual ~Viewer();
	//This peer will be used as the source of the video stream
	void setPeer(unsigned long long id) noexcept;
	unsigned long long getPeer() noexcept;
private:
	void configure(void *arg) override final;
	void cleanup() noexcept override final;
	void route(Message *message) noexcept override final;
	void maintain() noexcept override final;
	void processClockNotification(unsigned long long uid,
			unsigned long long ticks) noexcept override final;

	//Send a heartbeat message, request <frames> frames from the source <id>
	void sendHeartbeat(unsigned long long id, unsigned int frames) noexcept;
	//Reset the source identifier of the image
	bool resetSource(unsigned long long id, unsigned int frameRate,
			unsigned int session) noexcept;
	//Start a new jpeg frame
	void resetFrame(unsigned int size, unsigned int width, unsigned int height,
			unsigned int sequenceNumber) noexcept;
	//Populate the Jpeg frame
	void populateFrame(const unsigned char *data, unsigned int bytes) noexcept;
	//Display the image in a desktop window
	void processImage() noexcept;
	//Handle the response to a pairing request sent out by the heartbeat function
	int handlePairingResponse(Message *message) noexcept;

	//Reset the viewer and the video file
	void resetSink();
	//Process keyboard inputs
	void processKeyPress(int keyCode);
	//Hide the window
	void hideWindow() noexcept;
public:
	static constexpr unsigned long MAX_IMAGE_SIZE = 65536 * 4;
private:
	struct {
		unsigned long long id; //Desired source identifier
		unsigned int session; //Desired session identifier
	} source;

	struct {
		//Currently designated source of this image
		unsigned long long source;
		//Frame rate reported by the current source
		unsigned int frameRate;
		//Number of frames received since the last reset
		unsigned int frames;
		//Sequence number of the current frame
		unsigned int sequence;
		//Expected size of the image
		unsigned int size;
		//Bytes transferred so far by the source
		unsigned int bytes;
		//Frame width
		unsigned int width;
		//Frame height
		unsigned int height;
		//Image data
		unsigned char data[MAX_IMAGE_SIZE];
	} image;

	struct {
		int pan;
		int tilt;
	} control;

	struct {
		const char *fourcc { nullptr };
		char name[256];
		char fileName[PATH_MAX];
		cv::VideoWriter writer;
		bool reset { false };
		bool writeVideo { false };
	} sink;

	struct {
		double timestamp;
		double latitude;
		double longitude;
		bool show;
	} location;
};

} /* namespace wanhive */

#endif /* CLIENT_VIEWER_H_ */
