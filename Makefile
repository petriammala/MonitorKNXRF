# What to call the final executable
TARGET = monknxrf

OBJS = monitorknxrf.o cc1101.o influxDbInterface.o sensorKNXRF.o
DEP = WiringPi/wiringPi WiringPi/devLib WiringPi/gpio

# What compiler to use
CC = g++
CFLAGS = -c -Wall -Iinclude
LDFLAGS =

# We need -lcurl for the curl stuff
LIBS = -lcurl -lwiringPi -lsystemd

.PHONY:
all: $(TARGET)

.PHONY: wiringpi
wiringpi: $(DEP)
	for f in $^ ; do make -C $$f && sudo make -C $$f install ; done

# Link the target with all objects and libraries
.PHONY: $(TARGET)
$(TARGET): objs
	$(CC) $(CFLAGS) monitorknxrf.cpp
	$(CC)  -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

.PHONY: objs
objs: wiringpi install
	$(CC) $(CFLAGS) include/*.cpp

.PHONY: clean
clean: $(DEP)
	for f in $^ ; do make -C $$f clean ; done
	rm -f *.o $(TARGET)
	rm -f monknxrf

.PHONY: install
install: $(DEP)
	for f in $^ ; do sudo make -C $$f install ; done
#	systemctl stop monitorknxrf
#	cp monknxrf /usr/bin
#	cp monitorknxrf.service /etc/systemd/system
#	systemctl daemon-reload
#	systemctl enable monitorknxrf
#	systemctl start monitorknxrf

.PHONY: uninstall
uninstall: $(DEP)
	for f in $^ ; do sudo make -C $$f uninstall ; done
