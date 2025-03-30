#ifndef __APP_STATUS_LED_H__
#define __APP_STATUS_LED_H__

enum DeviceState {
    kDeviceStateUnknown,
    kDeviceStateStarting,
    kDeviceStateWiFiConnected,
    kDeviceStateBTConnected,
    //kDeviceStateWifiConfiguring,
    //kDeviceStateIdle,
    //kDeviceStateConnecting,
    kDeviceStateListening,
    kDeviceStateSpeaking
    //kDeviceStateUpgrading,
    //kDeviceStateActivating,
    //kDeviceStateFatalError
};

void SetDeviceLedState(enum DeviceState state);
enum DeviceState GetDeviceLedState(void);
void Init_Status_Led(void);

#endif