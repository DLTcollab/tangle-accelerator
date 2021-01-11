# OBD2 emulator

OBD2 emulator is a query server for endpoint OBDMonitor to get the vehicle response by the PID query on the CAN bus.
The OBD2 emulator has been implemented according to the SAE standard. For more information. You can refer to [OBD-II PIDs](https://en.wikipedia.org/wiki/OBD-II_PIDs).

## How to build OBD2 emulator
```
bazel build //endpoint/OBDComp/emulator:obd-emulator
```

## How to use OBD2 emulator
1. Confirm the virtual CAN driver has support. If virtual CAN is not available, you should re-configure the Linux kernel and build the corresponding image.
```
root@swi-mdm9x28-wp:~# zcat /proc/config.gz | grep VCAN
CONFIG_CAN_VCAN=y
```
2. Bring up CAN interface 
```
ip link add dev vcan0 type vcan
ifconfig vcan0 up 
```
3. Run the emulator
```
./obd-emulator vcan0
```
