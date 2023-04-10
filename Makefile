CC=/home/ruben/Documents/buildroot-2022.02.9/output/host/bin/arm-buildroot-linux-gnueabihf-gcc
CFLAGS=-c -Wall
LDFLAGS=-lpthread

SOURCES=accelerometer.c color_sensor.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=sensors

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)

