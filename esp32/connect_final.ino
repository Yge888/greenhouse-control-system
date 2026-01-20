#include "config.h"
#include "wifi_manager.h"
#include "stm32_comm.h"
#include "web_server.h"

void setup() {
  Serial.begin(115200);

  wifiInit();
  stm32Init();
  webServerInit();
}

void loop() {
  webServerLoop();
  stm32ProcessRecv();
}
