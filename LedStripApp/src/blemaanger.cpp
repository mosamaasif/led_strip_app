#include "blemanager.h"
#include "simpleble/Exceptions.h"
#include <iostream>
#include <sstream>

BLEManager::BLEManager() : color(new float[4](0.0f)), connectionStatus(BLESTATUS::UNDEFINED), m_IsScanning(false), m_IsDeviceOn(false), m_Peripheral(nullptr) {}

void BLEManager::ScanAndConnect()
{
    if (m_IsScanning && !IsConnected())
    {
        return;
    }
    m_ScanningThread = std::thread(&BLEManager::ScanAndConnectInternal, this);
}

void BLEManager::ScanAndConnectInternal()
{
    m_IsScanning = true;

    connectionStatus = BLESTATUS::SCANNING;
    std::vector<SimpleBLE::Adapter> adapters = SimpleBLE::Adapter::get_adapters();
    SimpleBLE::Adapter adapter = adapters[0];
    if (!adapter.bluetooth_enabled())
    {
        m_IsScanning = false;
        connectionStatus = BLESTATUS::BLT_NOT_ENABLED;
        return;
    }

    SimpleBLE::Peripheral peri;
    bool deviceFound = false;
    adapter.set_callback_on_scan_found([&peri, &deviceFound](SimpleBLE::Peripheral peripheral) mutable {
        if (peripheral.identifier().compare("QHM-1151") == 0)
        {
            deviceFound = true;
            peri = peripheral;
        }
     });

    adapter.scan_start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    adapter.scan_stop();

    if (!deviceFound)
    {
        connectionStatus = BLESTATUS::BLE_PERIPHERAL_NOT_FOUND;
    }
    else 
    {
        m_Peripheral = new SimpleBLE::Peripheral(peri);
        m_Peripheral->connect();

        connectionStatus = IsConnected() ? BLESTATUS::CONNECTED : BLESTATUS::FAILED_TO_CONNECT;
    }

    m_IsScanning = false;
}

void BLEManager::SetDeviceOn()
{
    if (!IsConnected())
    {
        connectionStatus = BLESTATUS::BLE_PERIPHERAL_NOT_CONNECTED;
        return;
    }
    try {
        m_Peripheral->write_request(WRITE_SERVICE, WRITE_CHARACTERISTIC, m_IsDeviceOn ? TURN_OFF_COMMAND : TURN_ON_COMMAND);
        m_IsDeviceOn = !m_IsDeviceOn;
    }
    catch (SimpleBLE::Exception::BaseException ex)
    {
        std::cout << ex.what() << std::endl;
    }
}

void BLEManager::UpdateLedColor()
{
    /*std::stringstream stream;
    stream << std::hex << "56" << color[0] << color[1] << color[2] << "00" << "F0" << "AA";
    std::cout << stream.str();
    m_Peripheral->write_request(WRITE_SERVICE, WRITE_CHARACTERISTIC, stream.str());*/
}

bool BLEManager::IsConnected()
{
    return m_Peripheral != nullptr && m_Peripheral->is_connected();
}

void BLEManager::JoinScanningThread()
{
    if (!m_ScanningThread.joinable())
    {
        return;
    }
    m_ScanningThread.join();
}

const char* BLEManager::ConnectionStatusStr()
{
    const char* str = "";
    switch (connectionStatus)
    {
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
        str = "Could not find a bluetooth adapter!";
        break;
    default:
        str = "";
        break;
    }
    
    return str;
}

BLEManager::~BLEManager()
{
    m_Peripheral->disconnect();
    delete m_Peripheral;
    delete[] color;
}