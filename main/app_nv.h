#ifndef __APP_NV_H__
#define __APP_NV_H__

//#include "nvs.h"
//#include "nvs_flash.h"
//#include "esp_system.h"
//#include "esp_log.h"
//#include "app_hf_msg_set.h"

int AppNVInit(void);
//int _AppNVRead(const char *key, char *value, sizeof_t &len, const char *defaultvalue);
void AppNVWrite(const char *key, const char *value);

#endif