# PSP2-batteryFixer
A small utility that fixes most of PSVita's battery-related problems.
<br>
It deletes all the temp flags from the battery chip and hard-resets all devices.
</br>
<br>
This tool was tested on firmwares 3.52 - 3.74.
</br>

![ref0](https://github.com/SKGleba/PSP2-batteryFixer/raw/master/screen/screen1.jpg)

## Usage
1) Install the VPK with VitaShell
2) Start the batteryFixer app
3) Follow the instructions displayed on your Vita screen.
	- If you don't see any text, and your vita doesn't power off after 5 seconds, press START.
	
## How this works
This tool resets the battery controller (Abby) and reboots the system controller (Ernie) before Abby can get its calibration data.
This in turn causes Ernie to do a full power cycle of all devices on Ernie startup due to bad Abby data, it also clears all the Abby calibration data and flags.
<br>
The ```PS+SELECT+LT+PWR+START for 10-15 secs``` key combination reboots Ernie and makes it do a full power cycle of all devices on startup, including Abby, but it does NOT clear its calibration data.
</br>

## Credits
	- Xerpi for plugin loader and baremetal loader/sample
	- Dots-tb and the team behind LOLIcon
