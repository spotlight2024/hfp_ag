#ifndef __APP_OPUS_CODEC_H__
#define __APP_OPUS_CODEC_H__

#define USE_ESP_AUDIO_CODEC 1

#ifdef USE_ESP_AUDIO_CODEC
#include "esp_audio_enc_default.h"
#include "esp_audio_enc_reg.h"
#include "esp_audio_enc.h"

#include "esp_audio_dec_default.h"
#include "esp_audio_dec_reg.h"
#include "esp_audio_dec.h"
#else

#include "opus.h"

#endif

void app_opus_init(void);
void app_opus_enc_process(char *p_buff, uint32_t in_sz, void(*ws_send)(const char* p, uint32_t sz));
void app_opus_destroy(void);


#endif