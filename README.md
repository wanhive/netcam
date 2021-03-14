# Wanhive Netcam

Video streamer application for the Wanhive IoT Platform.

## Features

* Multistreaming support. The sensor and control data interleave seamlessly with the video stream:
    - Geolocation data
    - Gimbal (pan/tilt) control
* Firewall friendly.
* Mobile network (4G/5G) friendly.
* Supports SSL for data security.

## Components

* **Streamer**: Outputs a sequence of JPEG images (just like MJPEG). Supports pan, tilt, and geolocation.
* **Viewer**: Captures and displays the video stream and sensor data from a Streamer. Keyboard controls:
    * Toggle geolocation (L)
    * Pan (A/D)
    * Tilt (W/S)

## Dependencies

Common (for Streamer and Viewer)
- Wanhive Hub development library (C++)
- OpenCV 4 development library

Additional dependencies for Streamer
- GPSd including the development library
- I2C userland development library

## Configuration

For Streamer (10 frames per second):

```
[HUB]
#listen = YES
timerExpiration = 100
timerInterval = 100

[CAMERA]
cameraName = /dev/videoXXX
jpegQuality = 70
gps = ON
servo = ON
```

For Viewer (heartbeat  at 5 seconds interval)

```
[HUB]
#listen = YES
timerExpiration = 100
timerInterval = 5000
```

## TODO

- Environment sensor
- Inertial sensor
- Motion detection

## Resources

- [CHANGELOG](ChangeLog.md)
- [LICENSE](LICENSE)
