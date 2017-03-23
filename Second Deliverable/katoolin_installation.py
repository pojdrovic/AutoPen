#Need to change this to be backwards compatable with Python 2

'''

This is the backbone of the Bluetooth tools. It does the following: 

1. Installs github if the machine does not have git (or maybe the software will come with github)
2. Clones katoolin so that can run on machine (maybe this will allow users to install tools from Kali Linux that they want)
3. Install Kali Repositories (this step has to happen to be able to install rest of the tools)
4. Installs Bluetooth essentials allowing the user to use

Note: A Linux machine with root access required
Note: This script runs during startup
Note: All information will be logged to /var/log/katoolin_installation.py 

'''

import subprocess


	print ("Setting /usr/bin/katoolin to executable...")
	changemode_EC = subprocess.run(["chmod", "754", "/usr/bin/katoolin"]) #executable script for both you and your group but not for the world. 
	if changemode_EC.returncode != 0:
		print ("CONVERSION FAILED: Could not make /usr/bin/katoolin executable")
		print ("ERROR CODE:", changemode_EC.returncode)
	elif changemode_EC.returncode == 0:
		print ("CONVERSION SUCCESSFUL: /usr/bin/katoolin set to executable")
		enter_katoolin_EC = subprocess.run(["katoolin"])
		print ("Starting up Katoolin...")
		if enter_katoolin_EC.returncode != 0:
			print ("KATOOLIN STARTUP FAILED: attempt to reinstall with the tool or go to README for instructions on how to install")
			print ("ERROR CODE:", enter_katoolin_EC.returncode)
		elif enter_katoolin_EC.returncode == 0:
			print ("STARTUP SUCCESSFUL: Entering Katoolin...")
		

'''

Next step? Guide them through using the tool with instructions from README
Trying to figure out a way to make this interactive (know Petit wanted some tutorial like thing so figuring that out ~~ yay)

'''




