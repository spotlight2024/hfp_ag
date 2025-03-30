#ifndef __APP_SR_H__
#define __APP_SR_H__

#include "esp_err.h"
#include "esp_mn_speech_commands.h"
#include "esp_process_sdkconfig.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_models.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_iface.h"
#include "esp_mn_iface.h"
#include "model_path.h"

esp_err_t app_sr_start(void);
esp_err_t app_sr_feed(int16_t *feed_buff);

size_t get_appsr_feedbuff_size(void);

void app_sr_StartDetection(void);
void app_sr_StopDetection(void);
bool Is_app_sr_DetectionRunning(void);

//esp_err_t app_sr_stop(void);
//esp_err_t app_sr_get_result(sr_result_t *result, TickType_t xTicksToWait);
//esp_err_t app_sr_start_once(void);

#endif
