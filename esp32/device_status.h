#ifndef DEVICE_STATUS_H
#define DEVICE_STATUS_H

struct DeviceStatus {
  bool fan;
  bool pump;
  int stepper;

  int temp;
  int humi;
  int light;
  int co2;
};

extern DeviceStatus deviceStatus;

#endif
