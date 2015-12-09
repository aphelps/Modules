1284p Board Setup
===========


Bootloading
-----------

* Setup Arduino as ISP
* Select "maniacbug" Might 1284
* Select "Arduino as ISP" under Tools->Programmer
* Select Tools->Bootloader

Verification
------------
* Connect FTDI cable
* Upload "Blink" on pin 12-14
* Upload HMTLPythonConfig
* Set configuration
  - HMTLConfig.py -v -f ~/Dropbox/Arduino/HMTL/python/configs/HMTLBoard_1284_v8.1.json [-w]
* Upload HMTL_Bringup
  - Check pixels, RS485 connection, etc