## ISS Pointer Requirements Sheet
Ben Andrews | Jared Dupont  
CS 466 Course Project  
2018-3-1  

#### Overview
This is a purely entertainment project. The purpose it to point an indicator at the International Space Station (ISS).

#### Top Level Functional Description
When the device is powered on it will calibrate for a couple of seconds to find compass north as well as it’s GPS position then constantly point at the International Space Station until power is cut or it is switched into calibration mode.  It should update every couple of seconds as the ISS move quickly.
The device should be approximately 8-12 inches squared as to fit on a desktop

There will be a button or a switch to toggle modes. The primary mode will have the device point at the ISS. The secondary mode is a calibration mode where the device will point North.

#### Top Level Non-functional Requirements
The software must fit on one Arduino Uno.

#### Detailed System Requirements
* Point at the ISS with ≈5° of accuracy
* Updates once every 1 to 5 seconds
* Self calibrate to find compass heading when button or switch is activated
* Self calibrate to find GPS position
* Display on LCD actual lat and long and status changes
