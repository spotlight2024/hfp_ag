#include "app_status_led.h"
#include <ws2812_control.h>
#include "esp_log.h"

#define START_COLOR             COLOR_GRAY
#define WIFI_COLOR              COLOR_YELLOW
#define BT_COLOR                COLOR_GREEN
#define LISTEN_COLOR            COLOR_NAVY
#define SPEAKING_COLOR          COLOR_MAROON

ws2812_strip_t* WS2812 = NULL;

enum DeviceState device_state_ = kDeviceStateUnknown;

//ws2812_set(WS2812,COLOR_BLUE,LED_EFFECT_BLINK_SLOW);
//    led_set_on(WS2812,COLOR_GREEN);
//    led_set_off(WS2812);
enum DeviceState GetDeviceLedState(void)
{
    return device_state_;
}

void SetDeviceLedState(enum DeviceState state)
{
    led_set_off(WS2812);
    device_state_ = state;
    switch (state)
    {
    case kDeviceStateStarting:
        led_set_on(WS2812,START_COLOR);
        break;
    case kDeviceStateWiFiConnected:
        led_set_on(WS2812,WIFI_COLOR);
        break;
    case kDeviceStateBTConnected:
        led_set_on(WS2812,BT_COLOR);
        break;
    case kDeviceStateListening:
        led_set_on(WS2812,LISTEN_COLOR);
        break;
    case kDeviceStateSpeaking:
        led_set_on(WS2812,SPEAKING_COLOR);
        break;    
    default:
        break;
    }
}

void Init_Status_Led(void)
{
    WS2812 = ws2812_create();
    SetDeviceLedState(kDeviceStateStarting);    
}

