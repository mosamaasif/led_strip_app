#pragma once

#include "simpleble/SimpleBLE.h"
#include <thread>
#include <vector>

enum BLESTATUS {
	UNDEFINED,
	SCANNING,
	CONNECTED,
	FAILED_TO_CONNECT,
	BLE_PERIPHERAL_NOT_FOUND,
	BLE_PERIPHERAL_NOT_CONNECTED,
	BLT_NOT_ENABLED,
};

class BLEManager
{
public:
	float* color;
	float brightness;
	BLESTATUS connectionStatus;
private:
	SimpleBLE::Peripheral* m_Peripheral;
	bool m_IsScanning;
	bool m_IsDeviceOn;
	std::thread m_ScanningThread;

	const SimpleBLE::BluetoothUUID WRITE_SERVICE = "0000ffd5-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::BluetoothUUID WRITE_CHARACTERISTIC = "0000ffd9-0000-1000-8000-00805f9b34fb";
	const std::string TURN_ON_COMMAND = "\xCC\x23\x33";
	const std::string TURN_OFF_COMMAND = "\xCC\x24\x33";

public:
	BLEManager();
	void ScanAndConnect();
	void SetDeviceOn();
	void UpdateLedColor();
	void UpdateBrightness();
	const char* ConnectionStatusStr();
	void JoinScanningThread();
	bool IsConnected();
	inline bool IsScanning() const { return m_IsScanning; }
	inline bool IsDeviceOn() const { return m_IsDeviceOn; }
	~BLEManager();

private:
	void ScanAndConnectInternal();
};