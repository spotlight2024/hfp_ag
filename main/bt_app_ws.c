#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"
#include "osi/allocator.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include "esp_eth.h"
#include "esp_websocket_client.h"
#include <cJSON.h>

#include "esp_tls.h"
#include "esp_tls_errors.h"
#include "esp_crt_bundle.h"

#include "app_status_led.h"
#include "bt_wifi_info.h"


#define GOT_IP_EVENT 1

EventGroupHandle_t app_ws_eventgroup = NULL;

//extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
//extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

//extern const uint8_t local_server_cert_pem_start[] asm("_binary_local_server_cert_pem_start");
//extern const uint8_t local_server_cert_pem_end[]   asm("_binary_local_server_cert_pem_end");

//static const int server_supported_ciphersuites[] = {MBEDTLS_TLS_RSA_WITH_AES_256_GCM_SHA384, MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256, 0};
//static const int server_unsupported_ciphersuites[] = {MBEDTLS_TLS_ECDHE_RSA_WITH_ARIA_128_CBC_SHA256, 0};


// 目标服务器
//#define CONFIG_WEBSOCKET_URI "ws://demo.piesocket.com/v3/channel_123?api_key=VCXCEuvhGcBDP7XhiJJUDvR1e1D3eiVjgZ9VRiaV&notify_self"

#define NO_DATA_TIMEOUT_SEC 5

static const char *TAG = "BT_APP_WS";

//static TimerHandle_t shutdown_signal_timer;
//static SemaphoreHandle_t shutdown_sema;

static esp_websocket_client_handle_t client = NULL;

extern RingbufHandle_t s_m_rb;

//TaskHandle_t wifi_TaskHandle = NULL;

static const char *strH = "{\"type\": \"hello\", \"transport\": \"websocket\", \
\"audio_params\": {\"format\": \"opus\", \"sample_rate\":  16000, \
\"channels\": 1, \"frame_duration\": 60 }}";

static const char *strStartManual = "{\"session_id\": \"\", \
\"type\":\"listen\",\"state\":\"start\", \"mode\": \"manual\"}";

static const char *strStartAuto = "{\"session_id\": \"\", \
\"type\":\"listen\",\"state\":\"start\", \"mode\": \"auto\"}";

static const char *strStop = "{\"session_id\": \"\", \
\"type\":\"listen\",\"state\":\"stop\" }";



void _WS_Parse_Incoming_Json(cJSON *root)
{
	cJSON *cJsonType = cJSON_GetObjectItem(root, "type");
	if (!strcmp("hello",cJsonType->valuestring) )
	{
		// gggccc Can not send ws in here, need use message to trig ws text send
		esp_websocket_client_send_text(client,strStartManual, strlen(strStartManual),portMAX_DELAY);
		SetDeviceLedState(kDeviceStateListening);
	} else if (!strcmp("tts",cJsonType->valuestring))
	{
		cJSON *cJsonState = cJSON_GetObjectItem(cJsonType,"state");
		if (strcmp(cJsonState->valuestring, "start") == 0)
		{
			SetDeviceLedState(kDeviceStateSpeaking);
		}else if (strcmp(cJsonState->valuestring, "stop") == 0)
		{
			esp_websocket_client_send_text(client,strStartManual, strlen(strStartManual),portMAX_DELAY);
			SetDeviceLedState(kDeviceStateListening);
		}
	}

	char* strRecJson = cJSON_Print(root);
	ESP_LOGI(TAG, "%s", strRecJson);
	//cJSON_free(strRecJson);

}


void ws_Send_Hello()
{
/*	
	char* strHello = NULL;
	cJSON* cjsonHello = cJSON_CreateObject();
	cJSON_AddStringToObject(cjsonHello,"type","hello");
	cJSON_AddNumberToObject(cjsonHello,"version",1);
	cJSON_AddStringToObject(cjsonHello,"transport","websocket");

	cJSON* cjsonAudioParam = cJSON_CreateObject();
	cJSON_AddStringToObject(cjsonAudioParam,"format","opus");
	cJSON_AddNumberToObject(cjsonAudioParam,"sample_rate",16000);
	cJSON_AddNumberToObject(cjsonAudioParam,"channels",1);
	cJSON_AddNumberToObject(cjsonAudioParam,"frame_duration",60);

	cJSON_AddItemToObject(cjsonHello,"audio_params",cjsonAudioParam);

	strHello = cJSON_Print(cjsonHello);
*/	
	//ESP_LOGI("GGGCCC", "%s\n",strH;
	
//	esp_websocket_client_send_text(client,strH, strlen(strH),portMAX_DELAY);
//	cJSON_free(strHello);

	// gggccc Here need handle memory leakage later.

}

void WIFI_CallBack(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	static uint8_t connect_count = 0;
	// WIFI 启动成功
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_START");
		ESP_ERROR_CHECK(esp_wifi_connect());
	}
	// WIFI 连接失败
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_DISCONNECTED");
		SetDeviceLedState(kDeviceStateStarting);
		connect_count++;
		if (connect_count < 6)
		{
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			ESP_ERROR_CHECK(esp_wifi_connect());
		}
		else
		{
			ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_DISCONNECTED 10 times");
		}
	}
	// WIFI 连接成功(获取到了IP)
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_GOT_IP");
		ip_event_got_ip_t *info = (ip_event_got_ip_t *)event_data;
		ESP_LOGI("WIFI_EVENT", "got ip:" IPSTR "", IP2STR(&info->ip_info.ip));
		xEventGroupSetBits(app_ws_eventgroup, GOT_IP_EVENT);
	}
}

// wifi初始化
void wifi_sta_init(void)
{
	app_ws_eventgroup = xEventGroupCreate();
	xEventGroupClearBits(app_ws_eventgroup, GOT_IP_EVENT);

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// 注册事件(wifi启动成功)
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START, WIFI_CallBack, NULL, NULL));
	// 注册事件(wifi连接失败)
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, WIFI_CallBack, NULL, NULL));
	// 注册事件(wifi连接失败)
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, WIFI_CallBack, NULL, NULL));

	// 初始化STA设备
	esp_netif_create_default_wifi_sta();

	/*Initialize WiFi */
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	// WIFI_INIT_CONFIG_DEFAULT 是一个默认配置的宏

	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	//----------------配置阶段-------------------
	// 初始化WIFI设备( 为 WiFi 驱动初始化 WiFi 分配资源，如 WiFi 控制结构、RX/TX 缓冲区、WiFi NVS 结构等，这个 WiFi 也启动 WiFi 任务。必须先调用此API，然后才能调用所有其他WiFi API)
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	// STA详细配置
	wifi_config_t sta_config = {
			.sta = {
					.ssid = ESP_WIFI_STA_SSID,
					.password = ESP_WIFI_STA_PASSWD,
					.bssid_set = false,
			},
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

	//----------------启动阶段-------------------
	ESP_ERROR_CHECK(esp_wifi_start());

	//----------------配置省电模式-------------------
	// 不省电(数据传输会更快)
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

	xEventGroupWaitBits(app_ws_eventgroup, GOT_IP_EVENT, pdTRUE, pdFALSE, portMAX_DELAY);
	SetDeviceLedState(kDeviceStateWiFiConnected);
	/*
	wifi_TaskHandle = xTaskGetHandle("wifi");
	if (wifi_TaskHandle)
	{
		vTaskPrioritySet(wifi_TaskHandle,18);
		ESP_LOGI(TAG, "Set wifi task priority to 22, lower than bt");
	}
	*/
}

static void log_error_if_nonzero(const char *message, int error_code)
{
	if (error_code != 0)
	{
		ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
	}
}

static void shutdown_signaler(TimerHandle_t xTimer)
{
	ESP_LOGI(TAG, "No data received for %d seconds, signaling shutdown", NO_DATA_TIMEOUT_SEC);
	//xSemaphoreGive(shutdown_sema);
}

// WebSocket客户端事件处理
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
	switch (event_id)
	{
	// 连接成功
	case WEBSOCKET_EVENT_CONNECTED:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
		esp_websocket_client_send_text(client,strH, strlen(strH),portMAX_DELAY);
		break;
	// 连接断开
	case WEBSOCKET_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
		log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
		if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
		{
			log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
			log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
			log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
		}
		break;
	// 收到数据
	case WEBSOCKET_EVENT_DATA:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
		ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
		if (data->op_code == 0x08 && data->data_len == 2 )
		{
			ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
		}
		else if (data->op_code == 0x02)
		{
			if (s_m_rb && data->data_len != 0) {
        		BaseType_t done = xRingbufferSend(s_m_rb, (char *)data->data_ptr, data->data_len, 0);
        		if (!done) {
           			ESP_LOGE(TAG, "ws to module ringbuf send fail");
				}
			}       
			//ESP_LOGW(TAG, "Received=%.*s\n\n", data->data_len, (char *)data->data_ptr);
			//ESP_LOGW(TAG, "Received=%d data", data->data_len);
		}
		else if ( data->op_code == 0x01)
		{
			// If received data contains json structure it succeed to parse
			cJSON *root = cJSON_Parse(data->data_ptr);
			if (root)
			{
				// Here Need Parsed Incoming Json 
				_WS_Parse_Incoming_Json(root);
				cJSON_Delete(root);
			}
		
		}
		ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d", data->payload_len, data->data_len, data->payload_offset);

	  // 定时器复位
		//xTimerReset(shutdown_signal_timer, portMAX_DELAY);
		break;
	// 错误
	case WEBSOCKET_EVENT_ERROR:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
		log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
		if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
		{
			log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
			log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
			log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
		}
		break;
	}
}

// WebSocket客户端
void websocket_app1_start(void)
{
	esp_err_t esp_ret = ESP_FAIL;
	esp_websocket_client_config_t websocket_cfg = {};

  // 创建定时器
	//shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", NO_DATA_TIMEOUT_SEC * 1000 / portTICK_PERIOD_MS,
	//																		 pdFALSE, NULL, shutdown_signaler);
	// 创建信号量
	//shutdown_sema = xSemaphoreCreateBinary();

	// 配置目标服务器
	websocket_cfg.uri = CONFIG_WEBSOCKET_URI;
	websocket_cfg.task_prio = (configMAX_PRIORITIES - 4);
	//websocket_cfg.use_global_ca_store = true;
	//websocket_cfg.transport = WEBSOCKET_TRANSPORT_OVER_SSL;
	websocket_cfg.crt_bundle_attach = esp_crt_bundle_attach;
	websocket_cfg.task_stack = (10 * 1024);

	ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);
	//ESP_LOGI("GGGCCC", "%s", WS_HELLO);

	//esp_ret = esp_tls_set_global_ca_store(server_root_cert_pem_start, server_root_cert_pem_end - server_root_cert_pem_start);
    //if (esp_ret != ESP_OK) {
    //    ESP_LOGE(TAG, "Error in setting the global ca store: [%02X] (%s),could not complete the https_request using global_ca_store", esp_ret, esp_err_to_name(esp_ret));
    //    return;
    //}

	// 创建WebSocket客户端
	client = esp_websocket_client_init(&websocket_cfg);
	esp_websocket_client_append_header(client,"Authorization","Bearer test-token");
	esp_websocket_client_append_header(client,"Protocol-Version","1");
	esp_websocket_client_append_header(client,"Device-Id","C8:2E:18:3C:F8:3A");
	esp_websocket_client_append_header(client,"Client-Id","5B283103-2D57-471F-ACCB-601627684E1F" );
	// 注册事件
	esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
	// 启动WebSocket客户端
	esp_websocket_client_start(client);
	//xTimerStart(shutdown_signal_timer, portMAX_DELAY);



	/*
	char data[32];
	int i = 0;
	// 发送5次数据
	while (i < 5)
	{
		if (esp_websocket_client_is_connected(client))
		{
			int len = sprintf(data, "hello %04d", i++);
			ESP_LOGI(TAG, "Sending %s", data);
			// 发送文本数据
			esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	ESP_LOGI(TAG, "Sending fragmented message");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	memset(data, 'a', sizeof(data));
	esp_websocket_client_send_text_partial(client, data, sizeof(data), portMAX_DELAY);
	memset(data, 'b', sizeof(data));
	esp_websocket_client_send_cont_msg(client, data, sizeof(data), portMAX_DELAY);
	esp_websocket_client_send_fin(client, portMAX_DELAY);

	*/
    
	// 等待信号量
	//xSemaphoreTake(shutdown_sema, portMAX_DELAY);
	/*
	// 关闭WebSocket客户端
	esp_websocket_client_close(client, portMAX_DELAY);
	ESP_LOGI(TAG, "Websocket Stopped");
	// 销毁WebSocket客户端
	esp_websocket_client_destroy(client);
	client = NULL;
	*/
}

void websocket_app1_stop(void)
{

	// 关闭WebSocket客户端
	if (client != NULL)
	{
		esp_websocket_client_stop(client);
		esp_websocket_client_close(client, portMAX_DELAY);
		ESP_LOGI(TAG, "Websocket Stopped");
		// 销毁WebSocket客户端
		esp_websocket_client_destroy(client);
		client = NULL;

		//esp_tls_free_global_ca_store();
	}	
}

void websocket_app_send(const char *buf, uint32_t sz)
{
	if (!client){
		return;
	}
	esp_websocket_client_send_bin(client, buf, sz, portMAX_DELAY);

}

void esp_print_tasks(void)
{
    char *pbuffer = (char*)osi_malloc(2048);
    vTaskList(pbuffer);
    printf("**********************************************\n");
    printf("Task         State   Prio    Stack     Num\n");
    printf("**********************************************\n");
    printf(pbuffer);
    printf("**********************************************\n");
    osi_free(pbuffer);

}