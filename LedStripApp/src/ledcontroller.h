#pragma once

#include "simpleble/SimpleBLE.h"
#include <thread>
#include <atomic>
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
	float* color;
	float brightness;
private:
	SimpleBLE::Peripheral* m_Peripheral;
	std::atomic_bool m_IsScanning;
	bool m_IsDeviceOn;
	BLESTATUS m_ConnectionStatus;
	std::thread m_ScanningThread;

	const std::string LED_DEVICE_NAME = "QHM-1151";
	const SimpleBLE::BluetoothUUID WRITE_SERVICE = "0000ffd5-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::BluetoothUUID WRITE_CHARACTERISTIC = "0000ffd9-0000-1000-8000-00805f9b34fb";
	const std::string TURN_ON_COMMAND = "\xCC\x23\x33";
	const std::string TURN_OFF_COMMAND = "\xCC\x24\x33";

public:
	LEDController();
	void ScanAndConnect();
	void ToggleDevice();
	void UpdateColor();
	void UpdateBrightness();
	const char* ConnectionStatusStr();
	void TryJoinScanningThread();
	bool IsConnected();
	inline bool IsScanning() const { return m_IsScanning; }
	inline bool IsDeviceOn() const { return m_IsDeviceOn; }
	~LEDController();

private:
	void ScanAndConnectInternal();
	void UpdateColorInternal(float intensity);
	std::vector<char> GenerateColorCommand(float intensity);
};