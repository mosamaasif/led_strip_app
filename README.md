# LED STRIP APP

NOTE: `WINDOW ONLY FOR NOW`

Purchased an LED Strip from Ali Express that had an app to control it called "Happy Lighting". Since I dind't want to use it and the led strip was attached to my workstation table, I decided to write this simple app to control the LED. `It's stil WIP as far as cleanup and UI improvements go`, but it works fine and can do the follwoing:

- On launch scans for the device and connects if found (scan by name, you can edit this part as per your need. There is a file called data.txt that is stored in Documents/LEDStripApp)
- If no peripheral found, or no blt is not on it shows relevant error and a connect button
- Once connected it shows the status and On/Off button, color picker and a brightness slider

Built with:
- Imgui for UI
- Uses Win32 and Dx3d for window and graphics respectively

There is an exe avail if you just want to use the app, however feel free to build the project yourself:
- Download Visual Studio, setup as per c++ development
- Clone repo, ppen .sln file and build the project. Output will be a folder called bin/....

![image](https://github.com/mosamaasif/led_strip_app/assets/13409110/3d1f3662-7214-48e8-8967-91dacd353703)
