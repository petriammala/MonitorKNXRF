# MonitorKNXRF

## General

This program is developed to be used on Raspberry Pi 3 B+ together with a CC1101 radiomodule.

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

You need to make sure SPI is enabled on your Raspberry:
```
> ls /dev/spi*
```
If there is no SPI device you need to enable it by running `raspi-config` as this option to enable SPI does not seem to exist in `openhabian-config`.
```
> sudo apt install raspi-config

> sudo raspi-config
```
Navigate to *Interface Options->SPI* and enable the SPI interface.

## Dependencies
This program depends on:
- systemd-dev
- liburl
- WiringPi (deprecated, TODO: update to pigpio)

### Systemd
You need to get the develop-package systemd-dev
```
> sudo apt-get install libsystemd-dev
```

### Curl
There are three different versions of libcurl*-dev:
- libcurl4-gnutls-dev
- libcurl4-nss-dev
- libcurl4-openssl-dev

Check the version of curl installed (if installed):
```
> curl --version
curl 7.74.0 (arm-unknown-linux-gnueabihf) libcurl/7.74.0 OpenSSL/1.1.1n zlib/1.2.11 brotli/1.0.9 libidn2/2.3.0 libpsl/0.21.0 (+libidn2/2.3.0) libssh2/1.9.0 nghttp2/1.43.0 librtmp/2.3
```
In this case the already installed curl version use openssl, it is recommended to use the same to avoid any conflicts:
```
> sudo apt-get install libcurl4-openssl-dev
```

### WiringPi
WiringPi is now deprecated. The source code is added as a submodule to this repository and it is included in the Makefile.

## Make and install

###
Clone this repository:
```
> git clone git@github.com:petriammala/MonitorKNXRF.git
> cd MonitorKNXRF
```

Update the dependencies:
```
> git submodule update --init --recursive
```

### Compile monknxrf
Compile the program by running make:
```
> make
...tbd g++ -c -Wall -I/home/openhabian/jsmn -Iinclude/ monitorknxrf.cpp include/*.cpp
...tbd g++  -o monknxrf monitorknxrf.o cc1101.o openhabRESTInterface.o sensorKNXRF.o -lcurl -lwiringPi -lsystemd
```

Run the program with debug level 2:
```
> ./monknxrf localhost:8086 2
```
Change set temperature on any of your thermostats, and wait a minium 15 seconds.
Press Ctrl+C to stop the program.
Then check the log:
```
> grep -in monitorknxrf /var/log/syslog
...
... monknxrf: MonitorKNXRF is requesting data from Openhab.
... monknxrf: MonitorKNXRF got data from sensor 007402363C12 reading 2232 and 1950.
... monknxrf: MonitorKNXRF: marcstate: 0x01

```
In this case the actual temperature is 22.32 C and the set temperature is 19.5 C.

Sometimes the thermostats send 4094. This is nothing to worry about, it will be filtered before being sent to Openhab
```
...monknxrf: MonitorKNXRF got data from sensor 007402363C0C reading 4094 and 2000.
... monknxrf: MonitorKNXRF got data from sensor 0074045CF417 reading 2152 and 4094.
```

### Make application a deamon
To make the program running as a system service you need to copy the the systemd files as this:
```
> sudo cp monknxrf /usr/bin/.
> sudo cp monitorknxrf.service /usr/lib/systemd/system/.
> sudo systemctl enable monitorknxrf.service
```

When the Raspberry is restarted the service should start automatically. It is possible to manually start/stop and check the status of the program:
```
> sudo systemctl start monitorknxrf.service
> sudo systemctl status monitorknxrf.service
● monitorknxrf.service - Service to collect KNX RF data and send to Influx database
   Loaded: loaded (/usr/lib/systemd/system/monitorknxrf.service; enabled)
   Active: active (running) since Thu 2020-01-02 12:33:25 CET; 2s ago
 Main PID: 16235 (monknxrf)
   CGroup: /system.slice/monitorknxrf.service
           └─16235 /usr/bin/monknxrf

... systemd[1]: Started Service to collect KNX RF data and send to openhab2 via the REST API.
...  monknxrf[16235]: MonitorKNXRF started
```

## Cleanup (optional)
To remove any all build artifacts including the `monknxrf` application:
```
> make clean
```

## Uninstall 
If you longer need the program you stop the service and then remove the files from the system directory:
```
> sudo systemctl stop monitorknxrf.service
> sudo systemctl disable monitorknxrf.service
> sudo rm /usr/bin/monknxrf
> sudo rm /usr/lib/systemd/system/monitorknxrf.service
```
Then you uninstall the WiringPi library:
```
> make uninstall
```
