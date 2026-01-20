#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// Web
#define WEB_PORT 80

// 串口
#define STM32_RX 16
#define STM32_TX 17
#define STM32_BAUD 115200

#endif
