# What to call the final executable
TARGET = monknxrf

OBJS= monitorknxrf.o cc1101.o influxDbInterface.o sensorKNXRF.o

# What compiler to use
CC = g++

CFLAGS = -c -Wall -Iinclude/

# We need -lcurl for the curl stuff
LIBS = -lcurl -lwiringPi -lsystemd

# Link the target with all objects and libraries
$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

monitorknxrf.o: monitorknxrf.cpp include/cc1101.h include/sensorKNXRF.h
	$(CC) $(CFLAGS) monitorknxrf.cpp

cc1101.o: include/cc1101.cpp include/cc1101.h
	$(CC) $(CFLAGS) include/cc1101.cpp

influxDbInterface.o: include/influxDbInterface.cpp include/influxDbInterface.h include/sensorKNXRF.h
	$(CC) $(CFLAGS) include/influxDbInterface.cpp

sensorKNXRF.o: include/sensorKNXRF.cpp include/sensorKNXRF.h include/influxDbInterface.h include/cc1101.h include/Crc16.h
	$(CC) $(CFLAGS) include/sensorKNXRF.cpp

clean:
	rm -f *.o
	rm -f monknxrf

install:
	systemctl stop monitorknxrf
	cp monknxrf /usr/bin
	cp monitorknxrf.service /etc/systemd/system
	systemctl daemon-reload
	systemctl enable monitorknxrf
	systemctl start monitorknxrf
