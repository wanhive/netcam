WH_INTERFACE_HDRS = src/interface/I2C.h
WH_INTERFACE_SRCS = src/interface/I2C.cpp

WH_DEVICE_HDRS = src/device/Camera.h src/device/Gimbal.h src/device/GPS.h \
	src/device/PCA9685.h src/device/Servo.h
WH_DEVICE_SRCS = src/device/Camera.cpp src/device/Gimbal.cpp src/device/GPS.cpp \
	src/device/PCA9685.cpp src/device/Servo.cpp

WH_CLIENT_HDRS = src/client/ClientManager.h src/client/Streamer.h src/client/Viewer.h
WH_CLIENT_SRCS = src/client/ClientManager.cpp src/client/Streamer.cpp \
	src/client/Viewer.cpp

WH_NC_INCLUDE_FLAGS = -I/usr/include/opencv4
WH_NC_LINKER_FLAGS = 

WH_NC_CXXFLAGS = $(WH_NC_INCLUDE_FLAGS) -O2 -g -Wall -c
WH_NC_LDFLAGS = $(WH_NC_LINKER_FLAGS) \
	-lwanhive -lopencv_core -lopencv_videoio -lopencv_highgui \
	-lopencv_imgproc -lopencv_imgcodecs


WH_STREAMER_HDRS = $(WH_INTERFACE_HDRS) $(WH_DEVICE_HDRS) $(WH_CLIENT_HDRS)
WH_STREAMER_SRCS = $(WH_INTERFACE_SRCS) $(WH_DEVICE_SRCS) $(WH_CLIENT_SRCS) \
	src/wanhive-netcam.cpp

WH_STREAMER_CXXFLAGS = $(WH_NC_CXXFLAGS)
WH_STREAMER_LDFLAGS = $(WH_NC_LDFLAGS) -li2c -lgps

WH_VIEWER_HDRS = src/client/ClientManager.h src/client/Viewer.h
WH_VIEWER_SRCS = src/client/ClientManager.cpp src/client/Viewer.cpp \
	src/wanhive-netcam.cpp

WH_VIEWER_CXXFLAGS = -DWH_WITHOUT_STREAMER $(WH_NC_CXXFLAGS)
WH_VIEWER_LDFLAGS = $(WH_NC_LDFLAGS)

WH_STREAMER_BIN = wanhive-nc
WH_VIEWER_BIN = wanhive-ncv


all: streamer

streamer: $(WH_STREAMER_HDRS) $(WH_STREAMER_SRCS)
	g++ $(WH_STREAMER_CXXFLAGS) $(WH_STREAMER_SRCS)
	g++ -o $(WH_STREAMER_BIN) *.o $(WH_STREAMER_LDFLAGS)
	rm -rf *.o
	
viewer: $(WH_VIEWER_HDRS) $(WH_VIEWER_SRCS)
	g++ $(WH_VIEWER_CXXFLAGS) $(WH_VIEWER_SRCS)
	g++ -o $(WH_VIEWER_BIN) *.o $(WH_VIEWER_LDFLAGS)
	rm -rf *.o

clean:
	rm -rf *.o $(WH_STREAMER_BIN) $(WH_VIEWER_BIN)


.PHONY: all clean

