#include <stdint.h>
#include <string>
#include <signal.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <syslog.h>
#include <systemd/sd-daemon.h> // needed for sd_notify, if compiler error try to run >> sudo apt-get install libsystemd-dev
#include "cc1101.h"
#include "sensorKNXRF.h"

#define GDO0idx 0
#define GDO2idx 1

volatile sig_atomic_t stopprogram;

void handle_signal(int sig)
{
	if (sig != 17) {
		syslog(LOG_INFO, "MonitorKNXRF received: %d", sig);
		stopprogram = 1;
	} 
}
// static volatile uint8_t globalCounter[2];

// Setting debug level to !0 equal verbose
uint8_t cc1101_freq_select = 3;
uint8_t cc1101_mode_select = 2;
uint8_t cc1101_channel_select = 0;
uint8_t cc1101_debug = 0;

// Global variables
CC1101 cc1101;
SensorKNXRF *sensorBuffer = NULL;

/*
 * Interrupts:
 *********************************************************************************
 */

/* void cc1101InterruptGDO0(void) {
	piLock(GDO0idx);
	++globalCounter[GDO0idx];
	syslog(LOG_INFO, "\r\nglobalCounter[GDO0idx]: %02d\r\n",globalCounter[GDO0idx]);
	piUnlock(GDO0idx); 
} */
void cc1101InterruptGDO2(void) {
	uint8_t dataBuffer[CC1101_BUFFER_LEN] ={0xFF}, dataLen;
	piLock(GDO2idx);
	cc1101.rx_payload_burst(dataBuffer, dataLen);
	saveSensorData(dataBuffer, dataLen, sensorBuffer);
	piUnlock(GDO2idx);
} 

/*
 *********************************************************************************
 * main
 *********************************************************************************
 */

int main (int argc, char* argv[])
{
	char s[256];
	char *influxHost = NULL;
	uint8_t addrCC1101 = 0;
	int internalWD = 0;
	int exitCode = EXIT_SUCCESS;
	int d = 15000;

        if (argc > 1) {
                influxHost = argv[1];
	}
        if (argc > 2) {
                cc1101_debug = atoi(argv[2]);
	} else {
		cc1101_debug = 0;
	}
	syslog(LOG_INFO, "MonitorKNXRF started");
	
	for (int n = 1; n < 32; n++) {
		signal(n, handle_signal);
	}
	
	
	
	stopprogram = 0;
	
	cc1101.begin(addrCC1101);			//setup cc1101 RF IC
	
	wiringPiSetup();			//setup wiringPi library
/* 	wiringPiISR (GDO0, INT_EDGE_RISING, &cc1101InterruptGDO0) ; */
	wiringPiISR (GDO2, INT_EDGE_RISING, &cc1101InterruptGDO2) ;



	cc1101.show_register_settings();

	try {
		while (!stopprogram) {
			delay(d);
			sd_notify(0,"WATCHDOG=1");
			internalWD++;
			while (sensorBuffer) {
				sprintf(s, "MonitorKNXRF got data from sensor %04X%08X reading %d and %d.", 
						sensorBuffer->serialNoHighWord, sensorBuffer->serialNoLowWord, transformTemperature(sensorBuffer->sensorData[1]), transformTemperature(sensorBuffer->sensorData[2]));
				syslog(LOG_INFO, s);
				piLock(GDO2idx);
				sendSensorData(sensorBuffer, influxHost);
				piUnlock(GDO2idx);
				delay(1);
				internalWD = 0;
			}
			if (internalWD > (60000 / d * 15)) { // terminate after waiting total 15 minutes
				stopprogram = 1;
				syslog(LOG_ERR, "MonitorKNXRF stopping due to no data received from CC1101");
				exitCode = EXIT_FAILURE;
			}
		}
	} catch (const std::exception& e) {
		syslog(LOG_ERR, "A standard exception was caught, with message: '%s'", e.what());
		exitCode = EXIT_FAILURE;
    }
	
	cc1101.end();
	syslog(LOG_INFO, "MonitorKNXRF stopped");
	return exitCode;
}
