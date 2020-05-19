# Hardware Abstract Layer(HAL) for Endpoint

Hardware Abstract Layer(HAL) defines a standard interface for hardware vendors to implement new device, which enables Tangle-accelerator to be aware of lower-level driver implementations. HAL provides operators such as UART, GPIO, secure storage and other low-level operators. The endpoint is a daemon, offering monitor for the device and being able to send message to Tangle-Accelerator. HAL layer lets the user only need to implement their device operators from abstract interface which is defined inside `device.h` but not need to know how endpoint works.

## How to implement new device

Create a directory for the new device under `devices`.
```
$mkdir -p devices/mydevice
```
* Create `impl.c` and `Makefile` under the new device directory.
* Include `device.h` into the new device header or the new device source file.

Here are some operations needed to be implemented for new device:
* device_operations
    * init : initialize device
    * fini : finalize device
    * get_key : get device key
    * get_device_id : get device id(IMEI or other identifier)
* uart_operations
    * init : initialize uart
    * write : write command to uart device
    * read : read from uart device
    * clean : flush buffer
* secure_store_operations
    * init : initialize secure storage
    * write : write item to secure storage
    * read : read item from secure storage
    * delete : delete item inside secure storage

Here are the functions needed to be registered/unregistered inside `impl.c`:
* register_device : register device on startup
* unregister_device : unregistered device
* DECLARE_DEVICE : this must be declared inside `impl.c`

Add the new device into `hal/Makefile`:

* Append the device object to DEVICE_OBJS
* Add the new device build target(mydevice.o)
```
DEVICE_OBJS = wp7702.o emulator.o device.o mydevice.o
export DEVICE_OBJS

all: $(DEVICE_OBJS)

mydevice.o: device.o
    $(MAKE) -C ../devices/mydevice
```

Implement a new device which is created under `devices` directory, and edit the Makefile. The example device is named as `mydevice`:
```
all: mydevice.o
mydevice.o: impl.c
        $(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@
```
`$(CC)`,`$(CFLAGS)` and `$(INCLUDES)` are specified by build system. `CC` sets the default compiler for the project. `CFLAGS` are the default flags that would be passed to default compiler during compiling time. `INCLUDES` flag includes headers inside sub-projects and third-party libraries. You can also modify these flags inside your device's Makefile.

impl.c
```c
#include "device.h"

static inline void register_emulator(void);
static inline void unregister_emulator(void);

static struct device_operations emulator_ops = {.init = &emulator_init,
                                                .fini = &emulator_release,
                                                .get_key = &emulator_get_key,
                                                .get_device_id = &emulator_get_device_id};

static struct uart_operations emulator_uart = {
    .init = &uart_init, .write = &uart_write, .read = &uart_read, .clean = &uart_clean};

static struct device_type emulator_device_type = {
    .name = "emulator", .op = &emulator_ops, .uart = &emulator_uart, .sec_ops = &emulator_sec_ops};

static inline void register_emulator(void) {
  int err = register_device(&emulator_device_type);
  if (err) LOG_ERROR("register emulator device error:%d", err);
}

static inline void unregister_emulator(void) {
  int err = unregister_device(&emulator_device_type);
  if (err) LOG_ERROR("unregister device emulator error:%d", err);
}

static int emulator_init(void) {
  register_emulator();
  return DEVICE_OK;
}

static void emulator_release(void) { unregister_emulator(); }

// must be declared at the end of impl.c
DECLARE_DEVICE(emulator);
```
