# MonitorKNXRF

## General

This program is developed to be used on Raspberry Pi 3 together with a CC1101 radiomodule.

The program will listen to interrupts generated from the CC1101. When an interrupt is recieved the data will be stored to a variable length buffer of class SensorKNXRF.
Every 15th second the program will sen data (temperature, target temperature, battery state and rssi) to Influx database.

## Pin layout

| Raspberry pin # | Name | CC1101 |
| --: | --- | --- |
| 17 | 3.3V              | VCC |
| 18 | GPIO24            | GDO0 |
| 19 | GPIO10 (SPI_MOSI) | SI |
| 20 | Ground            | GND |
| 21 | GPIO09 (SPI_MISO) | SO |
| 22 | GPIO25            | GDO2 |
| 23 | GPIO11 (SPI_CLK)  | SCLK1 |
| 24 | GPI008 (SP_CE0_N) | CSn |

Note: GDO0 is not used in the program. which GDO for interrupts to use is specifed in the configuration of the chip, see cc1101.cpp (look for IOCFG[0..2] and the data sheet for the CC1101.

## Example

Compile the program by running make:
```
$ make
g++ -c -Wall -I/home/openhabian/jsmn -Iinclude/ monitorknxrf.cpp include/*.cpp
g++  -o monknxrf monitorknxrf.o cc1101.o openhabRESTInterface.o sensorKNXRF.o -L/home/openhabian/jsmn -lcurl -ljsmn -lwiringPi -lsystemd
```

Before running the program, create Influx database

```
$ influx
Connected to http://localhost:8086 version 1.6.4
InfluxDB shell version: 1.6.4
> create database knxrf
```

Run the program with debug level 2:
```
./monknxrf localhost:8086 2
```
Change set temperature on any of your thermostats, and wait a minimum 15 seconds.
Press Ctrl+C to stop the program.
Then check the log:
```
grep -in monitorknxrf /var/log/syslog
...
... monknxrf: MonitorKNXRF got data from sensor 007402363C12 reading 2232 and 1950.
... monknxrf: MonitorKNXRF: marcstate: 0x01

```
In this case the actual temperature is 22.32 C and the set temperature is 19.5 C.

Sometimes the thermostats send 4094. This is nothing to worry about, it will be filtered before being sent to Influx database
```
... monknxrf: MonitorKNXRF got data from sensor 007402363C0C reading 4094 and 2000.
... monknxrf: MonitorKNXRF got data from sensor 0074045CF417 reading 2152 and 4094.
```

To make the program running as a system service you need to copy and modify the example service `monitorknxrf.service.example` to `monitorknxrf.service`:
```
cp monitorknxrf.service.example monitorknxrf.service
<modify service for your needs>
sudo make install
```

When the Raspberry is restarted the program should start automatically. It is possible to manually start/stop and check the status of the program:
```
$ sudo systemctl start monitorknxrf.service
$ sudo systemctl status monitorknxrf.service
● monitorknxrf.service - Service to collect KNX RF data and send to Influx database
   Loaded: loaded (/etc/systemd/system/monitorknxrf.service; enabled; vendor preset: enabled)
   Active: active (running) since Mon 2021-02-22 21:24:09 EET; 4min 28s ago
 Main PID: 13592 (monknxrf)
    Tasks: 2 (limit: 881)
   CGroup: /system.slice/monitorknxrf.service
           └─13592 /usr/bin/monknxrf localhost:8086

... systemd[1]: Started Service to collect KNX RF data and send to Influx database
... monknxrf[13592]: MonitorKNXRF started
```
