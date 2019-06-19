This project is to control a needlevalve mechanism with a homeswitch through the serial-USB port of an Arduino Nano, while also measuring the temperature with an smt172 in combination with a Raspberry Pi.
The commands that can be sent are the following:

cal:x

This calibrates the needlevalve mechanism. It seeks the homeswitch and settles there, then it moves x steps CW or CCW (-x = CCW, x = CW) and defines that position to be 0. This makes manual finetuning possible to make sure the needlevalve can be opened with the motor without loosing steps.

move:x

This opens (-x) or closes (x) the needle valve. It does not respond until calibrated.

pos:x

This takes care of the absolute positioning. -x is a position that can be reached going CCW and x a position that can be reached in the CW direction and has the calibrated 0 as the starting point. It does not respond until calibrated.

es

This returns the state of the homeswitch (NO): 0(closed) or 1(open)

The install script is meant to be run on a raspberry pi like this:
wget -O - https://raw.githubusercontent.com/nrbrt/smt172-nano-needlevalve/master/install.sh | sh

This will program the nano, that needs to be connected at that moment, without any user interaction and is meant for novice users
and easy installation.

The smt172 sensor needs to be connected to pin 8 and needs some electronics to connect it to the Arduino as shown in the connection diagram, the drv8825 to pin 12(dir) and pin 11(step), the homeswitch to pin 10.
This sketch uses the great smt172 library and smt172 connection diagram picture by Edwin Croissant.