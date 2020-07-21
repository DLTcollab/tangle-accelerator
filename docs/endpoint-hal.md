# Hardware Abstract Layer(HAL) for Endpoint

Hardware Abstract Layer(HAL) defines a standard interface for hardware vendors to implement new device, which enables Tangle-accelerator to be aware of lower-level driver implementations. HAL provides operators such as UART, GPIO, secure storage and other low-level operators. The endpoint is a daemon, offering monitor for the device and being able to send message to Tangle-Accelerator. HAL layer lets the user only need to implement their device operators from abstract interface which is defined inside `device.h` but not need to know how endpoint works.

## How to implement new device

Create a directory for the new device under `platform`.
```bash
$ mkdir -p platform/mydevice
```
* Create `impl.c` and `build.mk` under the new device directory.
* Include `device.h` into the new device header or the new device source file.

For `build.mk` you should define the `platform-build-command`. Here are the example from the WP7702 module:

```makefile
platform-build-command = \
	cd endpoint && leaf shell -c "mkapp -v -t wp77xx $(LEGATO_FLAGS) endpoint.adef"
```

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

impl.c

```c
#include "endpoint/hal/device.h"

static status_t simulator_init(void) {
  status_t err = register_device(&simulator_device_type);
  if (err != SC_OK) LE_ERROR("register simulator device error:%d", err);
  return err;
}

static void simulator_release(void) {
  status_t ret = unregister_device(&simulator_device_type);
  LE_INFO("unregister simulator return: %d", ret);
}

static const struct device_operations simulator_ops = {
    .init = simulator_init,
    .fini = simulator_release,
    .get_key = simulator_get_key,
    .get_device_id = simulator_get_device_id,
};

static const struct uart_operations simulator_uart = {
    .init = uart_init,
    .write = uart_write,
    .read = uart_read,
    .clean = uart_clean,
};

static const struct secure_store_operations simulator_sec_ops = {
    .init = sec_init,
    .write = sec_write,
    .read = sec_read,
    .delete = sec_delete,
};

struct device_type simulator_device_type = {
    .name = "simulator",
    .op = &simulator_ops,
    .uart = &simulator_uart,
    .sec_ops = &simulator_sec_ops,
};

// must be declared at the end of impl.c
DECLARE_DEVICE(simulator);
```
