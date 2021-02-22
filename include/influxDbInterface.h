#ifndef _INFLUXDBINTERFACE_H
#define _INFLUXDBINTERFACE_H

#include <string>
#include "sensorKNXRF.h"

class InfluxData {
	char data[256];
        public:
                char* sensor = NULL;
                int temperature = 0, targetTemperature = 0, rssi = 0;
                bool batteryOk = 0;
                std::string asLineProtocol(std::string measurement) {
                        sprintf(data, "%s,sensor=%s temperature=%d,targetTemperature=%d,rssi=%d,batteryOk=%d",
                                measurement.c_str(), this->sensor, this->temperature, this->targetTemperature, this->rssi, this->batteryOk);
			return std::string(data);
                };
};

void sendToInfluxDb(SensorKNXRF *currentSensor, char *influxHost);

#endif
