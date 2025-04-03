#include "esp_log.h"
#include "app_sr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "app_status_led.h"


static const char *TAG = "app_sr";

#define FEED_EVENT 1

static esp_afe_sr_iface_t *afe_handle = NULL;
static srmodel_list_t *models = NULL;
static afe_config_t *afe_config = NULL;
static esp_afe_sr_data_t *afe_data = NULL;
EventGroupHandle_t app_sr_eventgroup = NULL;


void app_sr_feed_Task(void *arg)
{
    // esp_afe_sr_iface_t *afe_handle = &ESP_AFE_SR_HANDLE;
    //afe_task_into_t *afe_task_info = (afe_task_into_t *)arg;
    //esp_afe_sr_iface_t *afe_handle = afe_task_info->afe_handle;
    //esp_afe_sr_data_t *afe_data = afe_task_info->afe_data;
    if ((!afe_handle) || (!afe_data))
    {
        return;
    }


    while (1) {
        xEventGroupWaitBits(app_sr_eventgroup, FEED_EVENT, pdFALSE, pdTRUE, portMAX_DELAY);
        afe_fetch_result_t *res = afe_handle->fetch(afe_data);

        if (!res || res->ret_value == ESP_FAIL) {
            ESP_LOGW(TAG,"APP SR fetch error!");
            continue;
        }

        if ( res->vad_state == VAD_SPEECH )
        {
            SetDeviceLedState(kDeviceStateListening_VAD_SPEECH);
        } 
        else
        {
            SetDeviceLedState(kDeviceStateListening_VAD_SILENCE);
        }

        //vad_state_t vad_state = res->vad_state;
        //wakenet_state_t wakeup_state = res->wakeup_state;

        // if vad cache is exists, please attach the cache to the front of processed_audio to avoid data loss
        //if (res->vad_cache_size > 0) {
        //    int16_t *vad_cache = result->vad_cache;
        //}

        //if (res->wakeup_state == WAKENET_DETECTED) {
        //    detect_cnt++;
        //}
        vTaskDelay( 30 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}


esp_err_t app_sr_start(void)
{
    esp_err_t ret = ESP_OK;
    models = esp_srmodel_init("model");     

    afe_config = afe_config_init("M", models, AFE_TYPE_SR, AFE_MODE_HIGH_PERF);
    afe_config->vad_mode = VAD_MODE_3;
    afe_config->vad_min_noise_ms = 2000;
    afe_config_print(afe_config); // print all configurations

    // 获取句柄
    afe_handle = esp_afe_handle_from_config(afe_config);
    // 创建实例
    afe_data = afe_handle->create_from_config(afe_config);

    app_sr_eventgroup = xEventGroupCreate();

     //task_flag = 1;
    //xTaskCreatePinnedToCore(&detect_Task, "detect", 8 * 1024, (void*)afe_data, 5, NULL, 1);
    xTaskCreatePinnedToCore(&app_sr_feed_Task, "feed", 8 * 1024, (void*)afe_data, 9, NULL, 0);
    return ret;
}

size_t get_appsr_feedbuff_size(void)
{

    if ((!afe_handle) || (!afe_data))
    {
        return 0;
    }

    int feed_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int feed_nch = afe_handle->get_feed_channel_num(afe_data);
    int buff_size = feed_chunksize * feed_nch * sizeof(int16_t);

    return buff_size;
}

esp_err_t app_sr_feed(int16_t *feed_buff)
{
    esp_err_t ret = ESP_OK;
    if ((!afe_handle) || (!afe_data))
    {
        return -1;
    }

    //int feed_chunksize = afe_handle->get_feed_chunksize(afe_data);
    //int feed_nch = afe_handle->get_feed_channel_num(afe_data);

    afe_handle->feed(afe_data, feed_buff);

    return ret;
}

void app_sr_StartDetection(void)
{
    xEventGroupSetBits(app_sr_eventgroup,FEED_EVENT);
} 

void app_sr_StopDetection(void)
{
    xEventGroupClearBits(app_sr_eventgroup,FEED_EVENT);
} 

bool Is_app_sr_DetectionRunning() {
    return xEventGroupGetBits(app_sr_eventgroup) & FEED_EVENT;
}

