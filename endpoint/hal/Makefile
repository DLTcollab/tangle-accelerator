DEVICE_OBJS = device.o
export DEVICE_OBJ

all: $(DEVICE_OBJS)

device.o: device.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^

clean:
	rm -f $(DEVICE_OBJS)
