# Wall-climbing Robot for Inspection

## Changelog

### 20250502 - Version 1
Arduino UNO R4 WiFi
- Added status tracker to detect changes and send to UI.
- Created `StatusTracker.h` for status tracker class definition.
- Created `Components.h` to contain components definitions, initialisations and functions.
- Status will be updated and sent to UI only when changes occur.

ESP32-Cam
- Added complete code.
- Modified `stream_handler` and `capture_handler` in `app_httpd.cpp` by adding a line to set response header:

    ```httpd_resp_set_hdr(req, "Content-Encoding", "identity");```

UI
- Added title.
- Completed camera feed.
- Added a button for capturing and saving image.
- Added a note for robot movement.
- Robot status will be updated only when changes occur.
- Resolved issue: Front LED status and EDF Power value not updated on load/refresh.

### 20250429 - Beta 1
Arduino UNO R4 WiFi
- Basic control for LEDs, EDFs, and servo motors.

UI
- Basic control without camera feed.
