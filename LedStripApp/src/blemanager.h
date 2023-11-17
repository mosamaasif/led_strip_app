#pragma once

#include "simpleble/SimpleBLE.h"

enum BLESTATUS {
	UNDEFINED,
	SCANNING,
	CONNECTED,
	FAILED_TO_CONNECT,
	BLE_PERIPHERAL_NOT_FOUND,
	BLT_NOT_ENABLED,
};

class BLEManager
{
public:
	float* color;
	BLESTATUS connectionStatus;
private:
	SimpleBLE::Peripheral m_peripheral;

public:
	BLEManager();
	void ScanAndConnect();
	void UpdateLedColor();
	const char* ConnectionStatusStr();
	~BLEManager();
};