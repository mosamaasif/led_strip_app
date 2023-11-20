# LED STRIP APP

NOTE: `WINDOW ONLY FOR NOW`

Purchased an LED Strip from Ali Express that had an app to control it called "Happy Lighting". Since I didn't want to use it and the led strip was attached to my workstation table, I decided to write this simple windows app/program to control the LED. `It's stil WIP as far as cleanup and UI improvements go`, but it works fine and can do the follwoing:

- Scan and connect to the device
- Turn it on/off
- Change color
- Change brightness
- Saves currently selected setting before closing app and are loaded and applied next time app is launched

Built with:
- Imgui for UI
- Uses Win32 and Dx3d for window and graphics respectively
- Simpleble for bluetooth

There is an exe avail if you just want to use the app, however feel free to build the project yourself:
- Download Visual Studio, setup as per c++ development
- Clone repo, open .sln file and build the project. Output will be a folder called bin/{PLATFORM}{CONFIGURATION}/

NOTE: `To get device name use nRF Connect app (android and iOS) and scan, find your device and use that name`

![Screenshot 2023-11-18 232800](https://github.com/mosamaasif/led_strip_app/assets/13409110/9319fc2b-7e7d-414c-a379-060cc9535f1e)
