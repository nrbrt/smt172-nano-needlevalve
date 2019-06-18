This is to control a needlevalve mechanism with a homeswitch through the serial-USB port of an Arduino Nano, while also measuring the temperature with an smt172 in combination with a Raspberry Pi.
The commands that can be sent are the following:

cal:x

This calibrates the needlevalve mechanism. It seeks the homeswitch and settles there, then it moves x steps CW or CCW (-x = CCW, x = CW) and defines that position to be 0. This makes manual finetuning possible to make sure the needlevalve can be opened with the motor without loosing steps.

move:x

This opens (-x) or closes (x) the needle valve. It does not respond until calibrated.

pos:x

This takes care of the absolute positioning. -x is a position that can be reached going CCW and x a position that can be reached in the CW direction. It does not respond until calibrated.

es

This returns the state of the homeswitch (NO): 0(closed) or 1(open)
