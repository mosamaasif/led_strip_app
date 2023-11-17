#include "blemanager.h"
#include <thread>

BLEManager::BLEManager(): color(new float[4]), connectionStatus(BLESTATUS::UNDEFINED) {}

void BLEManager::ScanAndConnect()
{
    connectionStatus = BLESTATUS::SCANNING;
    std::vector<SimpleBLE::Adapter> adapters = SimpleBLE::Adapter::get_adapters();
    SimpleBLE::Adapter adapter = adapters[0];

    SimpleBLE::Peripheral peri;
    bool deviceFound = false;
    adapter.set_callback_on_scan_found([peri, deviceFound, adapter](SimpleBLE::Peripheral peripheral) mutable {
        if (peripheral.identifier().compare("QHM-1151") == 0)
        {
            deviceFound = true;
            peri = peripheral;
            adapter.scan_stop();
        }
    });

    if (!deviceFound)
    {
        connectionStatus = BLESTATUS::BLE_PERIPHERAL_NOT_FOUND;
        return;
    }

    m_peripheral = peri;
    m_peripheral.connect();

    connectionStatus = !m_peripheral.is_connected() ? BLESTATUS::FAILED_TO_CONNECT : BLESTATUS::CONNECTED;
}

void BLEManager::UpdateLedColor()
{
    
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
    delete[] color;
}