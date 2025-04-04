#include <stdio.h>
#include <string.h>

#include "nvs.h"
//#include "nvs_flash.h"
//#include "esp_system.h"
#include "esp_log.h"
#include "app_hf_msg_set.h"
#include "app_nv.h"
#include "esp_bt_defs.h"



static const char *TAG = "app_nv";

uint8_t bt_mac[24] = {0};
uint8_t ssid[32] = {0};
uint8_t pwd[64] = {0};
uint8_t ws_addr[64] = {0};

const char* default_mac = "00:11:22:33:44:55";
const char* default_ssid = "ssid";
const char* default_pwd = "pwd";
const char* default_ws_addr = "ws://192.168.1.188/5000";

extern esp_bd_addr_t hf_peer_addr; 

int _AppNVRead(nvs_handle_t nv_handle, const char *key, uint8_t *value, const char *defaultvalue)
{
    size_t len;
    esp_err_t err;
    err = nvs_get_str(nv_handle, key, NULL, &len);
    switch (err) {
        case ESP_OK:
            nvs_get_str(nv_handle, key, (char *)value, &len);
            ESP_LOGI(TAG, "read %s from nv and value is %s",key, value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            nvs_set_str(nv_handle, key, defaultvalue);
            nvs_commit(nv_handle);
            memcpy(value,defaultvalue,strlen(defaultvalue));            
            ESP_LOGI(TAG, "%s use default value: %s",key, value);
            break;
        default :
            ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
            return -2;
    }
    return ESP_OK;
}

int AppNVInit(void)
{
    nvs_handle_t gf_handle;
    esp_err_t err;
    //size_t buflen;
    err = nvs_open(NV_GF_GROUP, NVS_READWRITE, &gf_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return -1;
    } 

    _AppNVRead(gf_handle,BT_HS,bt_mac,default_mac);
    _AppNVRead(gf_handle,SSID,ssid,default_ssid);
    _AppNVRead(gf_handle,PWD,pwd,default_pwd);
    _AppNVRead(gf_handle,WS_ADDR,ws_addr,default_ws_addr);

    uint8_t* pmac = hf_peer_addr;

    sscanf((char *)bt_mac, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
            &pmac[0], &pmac[1], &pmac[2], &pmac[3], &pmac[4], &pmac[5]);
    
    nvs_close(gf_handle);
    return ESP_OK;

}
//void AppNVRead();
void AppNVWrite(const char *key, const char *value)
{
    nvs_handle_t gf_handle;
    esp_err_t err;
    err = nvs_open(NV_GF_GROUP, NVS_READWRITE, &gf_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    } 

    nvs_set_str(gf_handle, key, value);
    nvs_commit(gf_handle);

    nvs_close(gf_handle);
}