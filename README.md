# LED Strip App

A Windows desktop app for controlling cheap BLE LED strips over Bluetooth, built with C++ and ImGui.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/mosamaasif/led_strip_app/blob/main/LICENSE)

## About

I bought a cheap LED strip from AliExpress that came with the "Happy Lighting" app. Since the strip was attached to my workstation table and I didn't want to rely on the stock app, I wrote this small Windows program to control it directly over Bluetooth Low Energy. It's a straightforward utility -- nothing fancy, but it gets the job done.

## Features

- Scan for and connect to BLE LED strips by device name
- Turn the strip on and off
- Pick any color with a full RGB color picker
- Adjust brightness with a slider
- Automatically saves your current settings (color, brightness, on/off state) on exit and restores them on next launch

## Built With

- **C++20** -- core language
- **[Dear ImGui](https://github.com/ocornut/imgui)** -- immediate-mode GUI
- **Win32 + Direct3D 12** -- window management and rendering
- **[SimpleBLE](https://github.com/OpenBluetoothToolbox/SimpleBLE)** -- Bluetooth Low Energy communication
- **Visual Studio 2022** (v143 toolset)

## Getting Started

> **Note:** This is a Windows-only application.

### Prerequisites

- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload installed
- Windows 10 SDK
- A Bluetooth adapter on your PC
- To find your LED strip's device name, use the [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile) app on Android or iOS to scan nearby BLE devices

### Build and Run

1. Clone the repository:
   ```
   git clone https://github.com/mosamaasif/led_strip_app.git
   ```
2. Open `LedStripApp.sln` in Visual Studio.
3. Select your desired configuration (Debug/Release) and platform (x86/x64).
4. Build the solution. The output executable will be in `bin/{Platform}{Configuration}/` (e.g., `bin/x64Release/`).
5. Run the app, enter your LED strip's device name, and click **Connect**.

## Screenshot

![LED Strip App Screenshot](https://github.com/mosamaasif/led_strip_app/assets/13409110/9319fc2b-7e7d-414c-a379-060cc9535f1e)

## License

This project is licensed under the MIT License -- see the [LICENSE](LICENSE) file for details.
