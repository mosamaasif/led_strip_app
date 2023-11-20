#include "ledcontroller.h"
#include "simpleble/Exceptions.h"
#include <iostream>

LEDController::LEDController()
{
    color = new float[3](1.0f, 1.0f, 1.0f);
    brightness = 1.0f;
    m_ConnectionStatus = BLESTATUS::UNDEFINED;
    m_IsScanning = false;
    m_IsDeviceOn = false;
    m_Peripheral = nullptr;
}

void LEDController::scanAndConnect()
{
    if (m_IsScanning || isConnected())
    {
        return;
    }
    m_ScanningThread = std::thread(&LEDController::scanAndConnectInternal, this);
}

void LEDController::toggleDevice()
{
    if (!isConnected())
    {
        m_ConnectionStatus = BLESTATUS::BLE_PERIPHERAL_NOT_CONNECTED;
        return;
    }
    m_IsDeviceOn = !m_IsDeviceOn;
    setDeviceOn(m_IsDeviceOn);
}

void LEDController::updateColor()
{
    updateColorInternal(1.0f);
}

void LEDController::updateBrightness()
{
    updateColorInternal(brightness);
}

void LEDController::writeCommand(SimpleBLE::ByteArray& command)
{
    try 
    {
        m_Peripheral->write_request(WRITE_SERVICE, WRITE_CHARACTERISTIC, command);
    }
    catch (SimpleBLE::Exception::BaseException ex)
    {
#if DEBUG
        std::cerr << ex.what() << std::endl;
#endif // DEBUG
    }
}

bool LEDController::isConnected()
{
    return m_Peripheral != nullptr && m_Peripheral->is_connected();
}


void LEDController::tryJoinScanningThread()
{
    if (m_IsScanning || !m_ScanningThread.joinable())
    {
        return;
    }
    m_ScanningThread.join();
}

std::string LEDController::connectionStatusStr()
{
    std::string str = "";
    switch (m_ConnectionStatus)
    {
    case BLESTATUS::UNDEFINED:
        str = "No Status";
        break;
    case BLESTATUS::SCANNING:
        str = "Scanning for device...";
        break;
    case BLESTATUS::CONNECTED:
        str = "Connected!";
        break;
    case BLESTATUS::FAILED_TO_CONNECT:
        str = "Failed to connect!";
        break;
    case BLESTATUS::BLE_PERIPHERAL_NOT_FOUND:
        str = "Could not find the peripheral!";
        break;
    case BLESTATUS::BLE_PERIPHERAL_NOT_CONNECTED:
        str = "Peripheral is not connected!";
        break;
    case BLESTATUS::BLT_NOT_ENABLED:
        str = "Bluetooth is not enabled!";
        break;
    }
    
    return str;
}

void LEDController::loadSettings()
{
    setDeviceOn(m_IsDeviceOn);
    // will also apply color since that's how the led command works
    updateBrightness();
}

void LEDController::setDeviceOn(bool isOn)
{
    SimpleBLE::ByteArray command = isOn ? TURN_ON_COMMAND : TURN_OFF_COMMAND;
    writeCommand(command);
}

void LEDController::updateColorInternal(float intensity)
{
    colorCommand[1] = static_cast<char>(color[0] * intensity * 255.999);
    colorCommand[2] = static_cast<char>(color[1] * intensity * 255.999);
    colorCommand[3] = static_cast<char>(color[2] * intensity * 255.999);
    writeCommand(colorCommand);
}

void LEDController::scanAndConnectInternal()
{
    m_IsScanning = true;
    m_ConnectionStatus = BLESTATUS::SCANNING;

    std::vector<SimpleBLE::Adapter> adapters = SimpleBLE::Adapter::get_adapters();
    SimpleBLE::Adapter adapter = adapters[0];
    if (!adapter.bluetooth_enabled())
    {
        m_IsScanning = false;
        m_ConnectionStatus = BLESTATUS::BLT_NOT_ENABLED;
        return;
    }

    SimpleBLE::Peripheral peri;
    bool deviceFound = false;
    char* dName = name;
    adapter.set_callback_on_scan_found([&dName, &peri, &deviceFound](SimpleBLE::Peripheral peripheral) mutable {
        if (peripheral.identifier().compare(dName) != 0)
        {
            return;
        }

        deviceFound = true;
        peri = peripheral;
        });

    adapter.scan_start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    adapter.scan_stop();

    if (!deviceFound)
    {
        m_ConnectionStatus = BLESTATUS::BLE_PERIPHERAL_NOT_FOUND;
    }
    else
    {
        m_Peripheral = new SimpleBLE::Peripheral(peri);
        m_Peripheral->connect();

        m_ConnectionStatus = isConnected() ? BLESTATUS::CONNECTED : BLESTATUS::FAILED_TO_CONNECT;
        loadSettings();
    }
    m_IsScanning = false;
}

LEDController::~LEDController()
{
    if (m_ScanningThread.joinable())
    {
        m_ScanningThread.join();
    }
    if (m_Peripheral != nullptr) 
    {
        m_Peripheral->disconnect();
        delete m_Peripheral;
    }
    delete[] color;
}