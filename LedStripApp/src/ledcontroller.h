#pragma once

#include "simpleble/SimpleBLE.h"
#include <atomic>
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

class LEDController
{
public:
	char name[128];
	float* color;
	float brightness;
private:
	const SimpleBLE::BluetoothUUID WRITE_SERVICE = "0000ffd5-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::BluetoothUUID WRITE_CHARACTERISTIC = "0000ffd9-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::ByteArray TURN_ON_COMMAND = { (char)0xCC, (char)0x23, (char)0x33 };
	const SimpleBLE::ByteArray TURN_OFF_COMMAND = { (char)0xCC, (char)0x24, (char)0x33 };
	SimpleBLE::ByteArray colorCommand = { (char)0x56, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0xF0, (char)0xAA };

	SimpleBLE::Peripheral* m_Peripheral;
	std::atomic_bool m_IsScanning;
	bool m_IsDeviceOn;
	BLESTATUS m_ConnectionStatus;
	std::thread m_ScanningThread;

public:
	LEDController();
	void scanAndConnect();
	void toggleDevice();
	void updateColor();
	void updateBrightness();
	void writeCommand(SimpleBLE::ByteArray& command);
	void tryJoinScanningThread();
	std::string connectionStatusStr();
	bool isConnected();
	inline bool isScanning() const { return m_IsScanning; }
	inline bool isDeviceOn() const { return m_IsDeviceOn; }
	inline void setDeviceOnFlag(bool isOn) { m_IsDeviceOn = isOn; }
	~LEDController();

private:
	void loadSettings();
	void setDeviceOn(bool isOn);
	void scanAndConnectInternal();
	void updateColorInternal(float intensity);
};