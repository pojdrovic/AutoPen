import general_use
import dependencies

'''

Note:
	RomRaider: Subaru
		Warning: RomRaider is intended for use only by experienced tuners who understand the consequences. As with any tuning solution, the potential
		for engine damage is very high when altering your ECUs hard coded values. The use of appropriate equipment (ie, knock sensor, wideband oxygen 
		sensor) is extremely important. By downloading RomRaider, you agree to assume all risks and accept its license. Use at your own risk.

Tools may be included:
	Octane:
		not made available because runs on Windows	
	AVRDUDESS:
		(this one not sure if im going to install yet)
	OpenPilot:
		Hondas and Acuras (believe this one needs specific hardware / it needs to be installed on that but maybe we can make it possible / do for them)

'''

def install_RomRaider(d):
	pack_man = general_use.package_tool(d)
	rom = dependencies.download_general('romraider', 'http://assembla.com/spaces/romraider/documents/a5Ao9gHEir5P9Udmr6QqzO/download/RomRaider0.5.9RC3-linux.jar')
	if rom != 0:
		print ('INSTALLATION FAILED: Failed to install RomRaiders.')
		print ('WITH ERROR CODE:', rom)
	else:
		print ('INSTALLATION SUCCESSFUL: Successfully installed RomRaiders')
