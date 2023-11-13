#pragma once

#include "simpleble/SimpleBLE.h"

class LedStrip
{
private:
	const char* CONNECTED_STATUS = "Connected!";
	const char* FAILED_TO_CONNECT_STATUS = "Failed to Connect, try again!";
	const char* BLT_NOT_ENABLED = "Bluetooth is not enabled!";
	const char* ADAPTER_NOT_FOUND = "Adapter not found!";
	SimpleBLE::Peripheral peripheral;

public:
	const char* ScanAndConnect();
	void ChangeLEDColor(const float* color);
};