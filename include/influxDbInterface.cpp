#include <syslog.h>
#include <string>
#include <curl/curl.h>
#include "influxDbInterface.h"
#include "sensorKNXRF.h"

const std::string DEFAULT_INFLUX_HOST = "localhost:8086";
const std::string INXFLUX_MEASUREMENTS = "knxrf_measurements";

void sendToInfluxDb(SensorKNXRF *currentSensor, char *influxHost) {
  char url[256];
  char sensorId[16];

  CURL *curl = curl_easy_init();
  if (curl) {
    InfluxData influxData;
    sprintf(sensorId, "%04X%08X", currentSensor->serialNoHighWord, currentSensor->serialNoLowWord);
    influxData.sensor = sensorId;
    influxData.temperature = transformTemperature(currentSensor->sensorData[1]);
    influxData.targetTemperature = transformTemperature(currentSensor->sensorData[2]);
    influxData.rssi = currentSensor->rssi;
    influxData.batteryOk = currentSensor->batteryOK;
    sprintf(url, "http://%s/write?db=knxrf", influxHost != NULL ? influxHost : DEFAULT_INFLUX_HOST.c_str());
    std::string data = influxData.asLineProtocol(INXFLUX_MEASUREMENTS);
    syslog(LOG_INFO, "Sending data '%s' to %s", data.c_str(), url);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* size of the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());

    /* pass in a pointer to the data - libcurl will not copy */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
}
