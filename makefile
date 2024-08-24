CC = gcc
CFLAGS = -Wall -O2
LIBS = -lasound
TARGET = record_audio

all: $(TARGET)

$(TARGET): record_audio.c
	$(CC) $(CFLAGS) record_audio.c -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET) *.o
