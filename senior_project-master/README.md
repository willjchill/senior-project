
## Introduction

ble looker.py is the python script. It does this:
    1. scan for bluetooth (including BLE) devices
    2. If it finds one that matches the MAC address (of the adafruit chip), it connects
    3. once connected, goes into a loop

I also included a folder, senior_project that includes a slightly modified code for the adafruit. To make it work, you will need to add it to the examples folder.
The modified adafruit is code is necessary because the python script is made in a way that only receives data while the adafruit code only sends data.
'