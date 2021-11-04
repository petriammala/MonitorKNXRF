#ifndef _INFLUXDBINTERFACE_H
#define _INFLUXDBINTERFACE_H

#include <string>
#include "sensorKNXRF.h"

#define NO_VALUE 4094

class InfluxData {
  char data[256];
public:
  char *sensor = NULL;
  char t1[32] = {0};
  char t2[32] = {0};
  int temperature = 0, targetTemperature = 0, rssi = 0;
  bool batteryOk = 0;

  std::string asLineProtocol(std::string measurement) {
    if (this->temperature != NO_VALUE) sprintf(t1, "temperature=%d,", this->temperature);
    if (this->targetTemperature != NO_VALUE) sprintf(t2, "targetTemperature=%d,", this->targetTemperature);
    sprintf(data, "%s,sensor=%s %s%srssi=%d,batteryOk=%d",
            measurement.c_str(), this->sensor, t1, t2, this->rssi, this->batteryOk);
    return std::string(data);
  };
};

void sendToInfluxDbs(SensorKNXRF *currentSensor, char *influxHost);

#endif
