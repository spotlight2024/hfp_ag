#include "app_opus_codec.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "opus_codec";

#ifdef USE_ESP_AUDIO_CODEC
esp_audio_enc_handle_t encoder = NULL;
uint8_t opus_enc_buf[2500];

#define APP_OPUS_ENC_CONFIG_DEFAULT()   (esp_opus_enc_config_t)  {  \
    .sample_rate        = ESP_AUDIO_SAMPLE_RATE_16K,         \
    .channel            = ESP_AUDIO_MONO,                    \
    .bits_per_sample    = ESP_AUDIO_BIT16,                   \
    .bitrate            = 256000,                            \
    .frame_duration     = ESP_OPUS_ENC_FRAME_DURATION_60_MS, \
    .application_mode   = ESP_OPUS_ENC_APPLICATION_VOIP,     \
    .complexity         = 3,                                 \
    .enable_fec         = true,                             \
    .enable_dtx         = true,                             \
    .enable_vbr         = true,                             \
}

esp_opus_enc_config_t  opus_cfg;
esp_audio_enc_config_t enc_cfg;

void app_opus_init(void)
{
    int ret = 0;

    esp_audio_enc_register_default();

    opus_cfg = APP_OPUS_ENC_CONFIG_DEFAULT();

    enc_cfg.type = ESP_AUDIO_TYPE_OPUS;
    enc_cfg.cfg = &opus_cfg;
    enc_cfg.cfg_sz = sizeof(opus_cfg);

    ret = esp_audio_enc_open(&enc_cfg, &encoder);
    if (ret != ESP_AUDIO_ERR_OK) {
        ESP_LOGE(TAG, "Fail to open encoder ret: %d", ret);
        return;
    } 
}

void app_opus_destroy(void)
{
    esp_audio_enc_close(encoder);
    esp_audio_enc_unregister_default();
}


void app_opus_enc_process(char *p_buff, uint32_t in_sz, void(*ws_send)(const char* p, uint32_t sz))
{
    int ret = 0;
    int read_size = 0;
    int out_size = 0;
    //uint8_t *read_buf = NULL;
    //uint8_t *write_buf = NULL;

    uint64_t s_time_start, s_time_finish;

    esp_audio_enc_get_frame_size(encoder, &read_size, &out_size);
    //ESP_LOGI(TAG, "encoder in size is: %d and out size is: %d", read_size,out_size);
    
    if ((read_size != in_sz) || (out_size >= 2500))
    {
        ESP_LOGE(TAG, "enc codec in size is different with enc codec setting!");
        return;
    }

    esp_audio_enc_in_frame_t in_frame = {
        .buffer = (uint8_t *) p_buff,
        .len = read_size,
    };
    esp_audio_enc_out_frame_t out_frame = {
        .buffer = opus_enc_buf,
        .len = out_size,
    };

    s_time_start = esp_timer_get_time();
    ret = esp_audio_enc_process(encoder, &in_frame, &out_frame);
    s_time_finish = esp_timer_get_time();
    ESP_LOGI(TAG, "encoder len: %ld and cost time: %lld", out_frame.encoded_bytes, s_time_finish - s_time_start);

    ws_send((char *)opus_enc_buf, out_frame.encoded_bytes);
}
#else

OpusEncoder* encoder = NULL;
uint8_t opus_enc_buf[1500];

void app_opus_init(void)
{
    if (encoder) { return; }
    encoder = opus_encoder_create(16000, 1, OPUS_APPLICATION_VOIP, NULL);
    assert(encoder != NULL);
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(3));
}

void app_opus_destroy(void)
{
    if (encoder)
    {
        opus_encoder_destroy(encoder);
        encoder = NULL;
    }
}

void app_opus_enc_process(char *p_buff, uint32_t in_sz, void(*ws_send)(const char* p, uint32_t sz))
{
    uint64_t s_time_start, s_time_finish;

    s_time_start = esp_timer_get_time();
    int len = opus_encode(encoder, (const opus_int16*)p_buff, in_sz/2, opus_enc_buf, sizeof(opus_enc_buf));
    s_time_finish = esp_timer_get_time();
    ESP_LOGI(TAG, "encoder len: %d and cost time: %lld", len, s_time_finish - s_time_start);
 
    ws_send((char *)opus_enc_buf, len);

}



#endif