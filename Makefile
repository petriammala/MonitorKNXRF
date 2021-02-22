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
	$(CC)  -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

$(OBJS) : monitorknxrf.cpp
	$(CC) $(CFLAGS) $< include/*.cpp

clean:
	rm -f *.o
install:
	systemctl stop monitorknxrf
	cp monknxrf /usr/bin
	cp monitorknxrf.service /etc/systemd/system
	systemctl daemon-reload
	systemctl enable monitorknxrf
	systemctl start monitorknxrf
