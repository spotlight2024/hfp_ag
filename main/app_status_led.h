#ifndef __APP_STATUS_LED_H__
#define __APP_STATUS_LED_H__

#include <stdbool.h>

enum DeviceState {
    kDeviceStateUnknown,
    kDeviceStateStarting,
    kDeviceStateWiFiConnected,
    kDeviceStateBTConnected,
    //kDeviceStateWifiConfiguring,
    //kDeviceStateIdle,
    //kDeviceStateConnecting,
    //kDeviceStateListening,
    kDeviceStateListening_VAD_SPEECH,
    kDeviceStateListening_VAD_SILENCE,
    kDeviceStateSpeaking
    //kDeviceStateUpgrading,
    //kDeviceStateActivating,
    //kDeviceStateFatalError
};

void SetDeviceLedState(enum DeviceState state);
enum DeviceState GetDeviceLedState(void);
void Init_Status_Led(void);
bool IsDeviceListen(void);
bool IsDeviceSpeaking(void);

#endif