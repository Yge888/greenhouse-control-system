// web_server.cpp
#include <WebServer.h>
#include <ArduinoJson.h>
#include "device_status.h"
#include "html_page.h"
#include "stm32_comm.h"

static WebServer server(80);

void webServerInit() {
  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/status", []() {
    StaticJsonDocument<256> doc;
    doc["fan"] = deviceStatus.fan;
    doc["pump"] = deviceStatus.pump;
    doc["stepper"] = deviceStatus.stepper;
    doc["temp"] = deviceStatus.temp;
    doc["humi"] = deviceStatus.humi;
    doc["light"] = deviceStatus.light;
    doc["co2"] = deviceStatus.co2;

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.on("/control", []() {
    String device = server.arg("device");
    String action = server.arg("action");
    
    StaticJsonDocument<256> doc;
    doc["success"] = true;
    
    if (device == "FAN") {
      deviceStatus.fan = !deviceStatus.fan;
      stm32SendCommand(deviceStatus.fan ? "FAN:ON" : "FAN:OFF");
    } else if (device == "PUMP") {
      deviceStatus.pump = !deviceStatus.pump;
      stm32SendCommand(deviceStatus.pump ? "PUMP:ON" : "PUMP:OFF");
    } else if (device == "STEPPER") {
      if (action == "FORWARD") {
        deviceStatus.stepper = 1;
        stm32SendCommand("MOTOR:FORWARD");
      } else if (action == "BACKWARD") {
        deviceStatus.stepper = 2;
        stm32SendCommand("MOTOR:BACKWARD");
      } else if (action == "STOP") {
        deviceStatus.stepper = 0;
        stm32SendCommand("MOTOR:STOP");
      }
    } else {
      doc["success"] = false;
    }
    
    // 返回更新后的状态
    doc["fan"] = deviceStatus.fan;
    doc["pump"] = deviceStatus.pump;
    doc["stepper"] = deviceStatus.stepper;
    doc["temp"] = deviceStatus.temp;
    doc["humi"] = deviceStatus.humi;
    doc["light"] = deviceStatus.light;
    doc["co2"] = deviceStatus.co2;
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.begin();
}

void webServerLoop() {
  server.handleClient();
}