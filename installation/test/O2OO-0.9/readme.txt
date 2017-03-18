O2OO:
----
O2OO is a toolkit for retrieving sensor measurements from a car.
One needs an ELM327 module for connecting the pc to the CAN-bus of a vehicle to do so.
I bought mine from: http://dx.com/p/mini-elm327-obd2-obdii-bluetooth-auto-car-diagnostic-scan-tool-12v-132202

O2OO-collector collects all measurements in an SQLite database
O2OO-dumper can dump that data on the console or generate graphs from it.


Building the program:
--------------------
Required libraries:
collector:
 - libsqlite3-dev
dumper & evaluator:
 - libgd2-xpm-dev
 - msttcorefonts or an other package with compatible(!) fonts
evaluator:
 - libhpdf-dev
 - libtinyxml2-dev
 - libcurl4-openssl-dev

Build procedure:
  make install
This will build the programs and install them under /usr/local/bin
The dump-program can create graphs. It needs a ttf-font for that. By default it will use /usr/share/fonts/truetype/freefont/FreeMono.ttf If you want to set a different location, either change the "FONT=" line in the Makefile-file or set a different location using -p (see on-line help (-h) for details).

If the compiler complains about the "-ggdb3" switch, then either replace it with "-g" or remove it.


Steps:
-----
1. turn off the vehicle
2. plug the ELM327 module in the CAN-bus (which can be found in a radius of 3 feet of the steering wheel)
3. start the vehicle AND the engine (as some cars don't list all sensors when the engine is not running)
4. start O2OO-collector


Connecting the ELM327 module to your laptop (step 2)
----------------------------------------------------
Either plug the USB connector in your laptop or setup a bluetooth connection (depends on the module you have).
Connecting via bluetooth is described here: http://en.opensuse.org/SDB:Obd-II_scan_tool
This should give you for example a /dev/ttyUSB0 or /dev/rfcomm1 device. Use that device for the -d parameter for O2OO-collector.
Please note: GPS devices usually give you also such (/dev/ttyUSBx or /dev/rfcommx) device: don't switch them as that won't work! Usually ELM327 modules are silent when you do "cat /dev/device" (eg cat /dev/ttyUSB0) while GPS devices will give all kinds of data (at least once per second).


Connecting a GPS to your laptop (step 2b)
-----------------------------------------
After you've connected your GPS, a /dev/xxx device should appear (like with the ELM327 module).
The GPS should be "speaking" NMEA sentences.
With
	O2OO-w4gf -g /dev/ttyUSB0
you can get a notification for when the GPS has a fix (which is required for correctly returning location).
In this example /dev/ttyUSB0 is the device to which the GPS is connected.


Usage:
-----
Example usage of the collector:
	O2OO-collector -d /dev/rfcomm0 -b mydata.db -v
This lets the collector talk to the ELM327 device connected to /dev/rfcomm0 (this is the virtual serial port of an ELM327 device which interfaces via bluetooth) and store the measured data in a database file called "mydata.db". During collecting it shows what it is doing. To make it "silent", omit the -v flag.
Use:
	-m test  for a fake-vehicle, in case you want to test the program without a real CAN-bus connection
	-g fake  for a fake GPS module

The collector has a '-f' (full speed) switch.
"The speed at which you obtain information from vehicles. Prior to the APR2002 release of the J1979 standard, sending J1850 requests more frequently than every 100 msec was forbidden. With the APR2002 update, scan tools were allowed to send the next request without delay if it was determined that all the responses to the previous request had been received." (ELM327DS.pdf)
If you have determined that your vehicle implements the APR2002 release of the J1979 standard, you can use the -f switch: in that case, O2OO-collector will not insert an 100ms delay between each message to the car.

It is advised to filter sensors you're not interested in with the -i switch to retrieve more measurements per timeframe.

Example usage of the dumper:
	O2OO-dumper -b test.db -p width=800,height=600 -s sensor_engine_load -c graphsensor
This reads data from a databasefile called "test.db" and draws the values from a sensor called "sensor_engine_load" into a file called "sensor-data.png".

Run O2OO-collector, O2OO-dumper and/or O2OO-evaluate with the '-h' parameter to get a list of options.

Run O2OO-evaluate with -I to enable Google Maps lookups of start- and end position.


Notes:
-----
Please note: occasionally the collector aborts (at start) when it received an 0x51 byte from the ELM327 module. In that case, (delete the created (empty) database and -) restart the collector.
If this problem persists, please contact me.

Another note: I noticed that occasionally the initialization seems to "hang". I noticed that sometimes something on my Linux system seems to send a "AT+CGAP" to the device when it was inserted. In that case it might be needed top terminate the O2OO-collector program and restart it. The symptoms are that the screen stays black and nothing seems to happen. You can verify this by starting the collector with "-l mylog.txt" and then viewing mylog.txt.
This seems to be caused by the "modemmanager"-package probing for a modem as soon as a serial device is detected in the system.

In non-fullspeed mode, the program produces a bit less than 30KB/s so make sure the device on which you store the measurements go write that "fast".


License:
-------
This program is licensed under GPL version 2 with the addition that any fixes/additions must be opensourced and send in an e-mail to folkert@vanheusden.com


To do:
-----
  - elaborate on everything in this file
  - elaborate on the ELM327 module


-- folkert@vanheusden.com

SVN revision: $Revision: 367 $
