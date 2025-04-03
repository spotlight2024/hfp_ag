#include "app_status_led.h"
#include <ws2812_control.h>
#include "esp_log.h"
#include "bt_app_ws.h"
//#include "freertos/ringbuf.h"


#define START_COLOR             COLOR_GRAY
#define WIFI_COLOR              COLOR_YELLOW
#define BT_COLOR                COLOR_GREEN
//#define LISTEN_COLOR            COLOR_NAVY
#define VAD_SILENCE_COLOR       COLOR_PURPLE
#define VAD_SPEECH_COLOR        COLOR_NAVY
#define SPEAKING_COLOR          COLOR_MAROON

ws2812_strip_t* WS2812 = NULL;
//extern RingbufHandle_t m_ws_rb;

enum DeviceState device_state_ = kDeviceStateUnknown;
//volatile bool IsOnlyVADnotOpus = true;   // VAD 30ms,  Opus 60ms
enum DeviceState prev_device_state_ = kDeviceStateUnknown;

//ws2812_set(WS2812,COLOR_BLUE,LED_EFFECT_BLINK_SLOW);
//    led_set_on(WS2812,COLOR_GREEN);
//    led_set_off(WS2812);
enum DeviceState GetDeviceLedState(void)
{
    return device_state_;
}

bool IsDeviceListen(void)
{
    return (device_state_ == kDeviceStateListening_VAD_SILENCE || device_state_ == kDeviceStateListening_VAD_SPEECH );
}

void SetDeviceLedState(enum DeviceState state)
{
    led_set_off(WS2812);
    prev_device_state_ = device_state_;
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
    case kDeviceStateListening_VAD_SILENCE:
        if (prev_device_state_ == kDeviceStateListening_VAD_SPEECH)
        {
            ws_Send_Stop();
        }

        led_set_on(WS2812,VAD_SILENCE_COLOR);
        break;
    case kDeviceStateListening_VAD_SPEECH:
        if (prev_device_state_ == kDeviceStateListening_VAD_SILENCE)
        {
            ws_Send_Start();
        }
        led_set_on(WS2812,VAD_SPEECH_COLOR);
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

