# GPS_Tracker_Navigator_Arduino
hardware description coming soon...
this is based on Arduino Uno rev3 board + proto board on which are connected: GPS shield mtk3329, lcd 1.8" with micro sd card reader both from Adafruit and three push buttons.

libraries used: 
- tinygps (edited)
- sdfat
- st7735 (edited for smaller footprint from adafruit)

The software has three modes: tracking, navigation and exploration:
- Tracking mode saves trackpoints with geolocation. speed. elevation and date/time every second to a csv file on microsd card. 

- Exploration mode saves waypoints with geolocation only.

- Navigation mode uses one of previously saved waypoints files with exploration mode for navigation
info displayed on screen changes a little in each mode, but basically it shows date/time, current speed, bearing, # trackpoints or waypoints, distance to next waypoint, coordinates, number of satellites/precision

this project is operational and I have used it many times during my mountain bike sessions. Can also be used for running or in cars (for tracking) with very few changes. 
feel free to fork, ask questions or ask for more code

more details to come soon...
