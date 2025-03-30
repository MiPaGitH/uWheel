implements the USB HID PID class for constant force effect
SSI is implemented for communication with the magnetic encoder used for steering wheel rotation detection
a simple interface over UART is implemented to control the RN4870/71 Ble module; this module is used to communicate with the pedals (acceleration and brake) via bluetooth le
the motor that applies the force feedback effect over the wheel is controlled via PWM
a simple scheduler is used to alow easy configuration of periodic task and the execution of the background task
