// stm32_comm.cpp
#include <HardwareSerial.h>
#include "device_status.h"
#include "config.h"

static HardwareSerial stm32Serial(2);

void stm32Init() {
  stm32Serial.begin(STM32_BAUD, SERIAL_8N1, STM32_RX, STM32_TX);
}

void stm32SendCommand(const String& cmd) {
  stm32Serial.println(cmd);
}

void stm32ProcessRecv() {
  if (stm32Serial.available()) {
    String line = stm32Serial.readStringUntil('\n');
    line.trim(); // 去除首尾空格和换行符
    
    // 调试输出：打印接收到的原始数据
    Serial.print("Received from STM32: ");
    Serial.println(line);
    
    // 在这里解析数据，更新 deviceStatus
    // STM32发送的数据格式："TEMP:%d,HUMI:%d,LIGHT:%d,CO2:%d"
    if (line.length() > 0) {
      int index = 0;
      while (index < line.length()) {
        int commaIndex = line.indexOf(',', index);
        if (commaIndex == -1) {
          commaIndex = line.length();
        }
        String param = line.substring(index, commaIndex);
        int colonIndex = param.indexOf(':');
        if (colonIndex != -1) {
          String key = param.substring(0, colonIndex);
          String value = param.substring(colonIndex + 1);
          
          // 更新设备状态（注意STM32使用大写键名）
          if (key == "TEMP") {
            deviceStatus.temp = value.toInt();
          } else if (key == "HUMI") {
            deviceStatus.humi = value.toInt();
          } else if (key == "LIGHT") {
            deviceStatus.light = value.toInt();
          } else if (key == "CO2") {
            deviceStatus.co2 = value.toInt();
          }
        }
        index = commaIndex + 1;
      }
    }
  } else {
    // 可选：添加调试信息，显示没有数据可用
    // Serial.println("No data available from STM32");
  }
}