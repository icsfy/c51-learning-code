#include "esp8266.h"
#include "stdio.h"
#include "string.h"

#define USE_ONENET 1
#define USE_BIGIOT 0

#if USE_ONENET      /* OneNet api.heclouds.com */
#define DEVICE_ID   "25842835"
#define API_KEY     "DkYap3u7DgYmu8x=beNR=lQd8GM="
#define TCP_ADDR    "183.230.40.33"
#define TCP_PORT    "80"
#define TX_BUF_SIZE 64
uchar pdata buff_tx[TX_BUF_SIZE];
#elif USE_BIGIOT      /* 贝壳物联 www.bigiot.net */
#define DEVICE_ID   "4623"
#define API_KEY     "3f534be1d"
#define DATA_ID     "4160"
#define TCP_ADDR    "121.42.180.30"
#define TCP_PORT    "8181"
#endif

static void ESP8266_CheckOK(char *param)
{
  char i;
  for (i = 0; i < strlen(param); i++) {
    while(!RI);
    RI = 0;
    if (SBUF != param[i]) {
      i = -1;
    }
  }
}

/* ESP8266需预先通过串口配置好参数
 * 配置好串口波特率为9600，需要连接的wifi和密码等
 * 这里不进行这些配置，芯片上电后应能够输出:
 *   WIFI CONNECTED
 *   WIFI GOT IP
 */
void ESP8266_Init(void)
{
  // 等待WIFI连接
  LED1 = 0;
  ESP8266_CheckOK("WIFI CONNECTED");
  LED1 = 1;
  
  // 等待获得IP
  LED2 = 0;
  ESP8266_CheckOK("WIFI GOT IP");
  LED2 = 1;
  
  // 设置模块传输模式为透传模式
  printf("AT+CIPMODE=1\n");
  LED3 = 0;
  ESP8266_CheckOK("OK");
  LED3 = 1;
}

/* 连接到TCP服务器 */
static void ESP8266_ConnectToTCPServer(void)
{
  printf("AT+CIPSTART=\"TCP\",\""TCP_ADDR"\","TCP_PORT"\n");
  LED4 = 0;
  ESP8266_CheckOK("OK");
  LED4 = 1;
}

/* 进入透传模式 */
static void ESP8266_TransparentMode(void)
{
  printf("AT+CIPSEND\n");
  LED5 = 0;
  ESP8266_CheckOK(">");
  LED5 = 1;
}

/* 上传数据 */
void ESP8266_PostJsonData(uint speed)
{
  static bit flag_connected = 0;
  /* 第一次发送数据时建立连接 */
  if (!flag_connected) {
    ESP8266_ConnectToTCPServer();
    ESP8266_TransparentMode();
#if USE_BIGIOT
    LED6 = 0;
    printf("{\"M\":\"checkin\",\"ID\":\""DEVICE_ID"\",\"K\":\""API_KEY"\"}\n");
    ESP8266_CheckOK("checkinok");
    LED6 = 1;
#endif
    flag_connected = 1;
  }
#if USE_ONENET
  sprintf(buff_tx, "{\"datastreams\":[{\"id\":\"Speed\","
          "\"datapoints\":[{\"value\":%d}]}]}", speed);
  printf("POST /devices/"DEVICE_ID"/datapoints HTTP/1.1\n"
         "api-key:"API_KEY"\n"
         "Host:api.heclouds.com\n"
//         "Content-Type: application/json\n"
         "Connection:close\n"
         "Content-Length:%d\n\n%s\n", strlen(buff_tx), buff_tx);
#elif USE_BIGIOT
  printf("{\"M\":\"update\",\"ID\":\""DEVICE_ID"\","
          "\"V\":{\""DATA_ID"\":\"%d\"}}\n", speed);
#endif
}
