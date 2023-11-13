#include "blemanager.h"

BLEManager::BLEManager(): color(new float[4]), connectionStatus("") {}

void BLEManager::ScanAndConnect()
{
    //if (!SimpleBLE::Adapter::bluetooth_enabled()) {
    //    return BLT_NOT_ENABLED;
    //}

    //auto adapters = SimpleBLE::Adapter::get_adapters();
    //if (adapters.empty()) {
    //    return ADAPTER_NOT_FOUND;
    //}

    //auto adapter = adapters[0];

    //// Scan for peripherals for 5000 milliseconds
    //adapter.scan_for(5000);

    //// Get the list of peripherals found
    //std::vector<SimpleBLE::Peripheral> peripherals = adapter.scan_get_results();

    //for (int i = 0; i < peripherals.size(); i++)
    //{
    //    if (peripherals[i].identifier().compare("QHM-1151")) {
    //        peripheral = peripherals[i];
    //        break;
    //    }
    //}

    //if (peripheral.initialized())
    //{
    //    peripheral.connect();
    //    for (auto service : peripheral.services()) {
    //        for (auto characteristic : service.characteristics()) {

    //        }
    //    }
    //}

    //connectionStatus = CONNECTED_STATUS;
}

void BLEManager::UpdateLedColor()
{
    /*if (peripheral.initialized())
    {

    }*/
}

BLEManager::~BLEManager()
{
    delete[] color;
    delete[] connectionStatus;
}