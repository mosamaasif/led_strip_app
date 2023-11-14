#pragma once

#include "simpleble/SimpleBLE.h"

class BLEManager
{
public:
	float* color;
	const char* connectionStatus;
private:
	const char* CONNECTED_STATUS = "Connected!";
	const char* FAILED_TO_CONNECT_STATUS = "Failed to Connect, try again!";
	const char* BLT_NOT_ENABLED = "Bluetooth is not enabled!";
	const char* ADAPTER_NOT_FOUND = "Adapter not found!";
	SimpleBLE::Peripheral m_peripheral;

public:
	BLEManager();
	void ScanAndConnect();
	void UpdateLedColor();
	~BLEManager();
};