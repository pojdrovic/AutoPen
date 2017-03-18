import general_use
import dependencies
import subprocess
import os

def install_GNU(d):
	pack_man = general_use.package_tool(d)

	gnu = dependencies.install_general(pack_man, 'gnuradio')
	if gnu != 0:
		print ('INSTALLATION FAILED: Failed to install gnuradio')
		print ('WITH ERROR CODE:', gnu)
	else:
		print ('INSTALLATION SUCCESSFUL: Successfully installed gnuradio')	
