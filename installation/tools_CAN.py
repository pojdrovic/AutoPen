'''

We only want the whole thing to run if the install all or install all CAN button is pressed

Else, we need to function to take in distro and the name of the tool depending on the button pressed and then install that

Note: Make sure the user knows that this will all be cloned in the current directory aka most likely /Users/'user'

'''

#Reference Links for me, might remove later we'll see how life goes 
#can-utils : https://discuss.cantact.io/t/using-can-utils/24 (eric evenchick)
#canbus-utils : http://www.digitalbond.com/blog/2015/03/05/tool-release-digital-bond-canbus-utils/
#NPM: http://blog.teamtreehouse.com/install-node-js-npm-linux
#NPM2: http://www.hostingadvice.com/how-to/install-nodejs-ubuntu-14-04/#node-version-manager (May 2016)
#build-essential can fail, try this just incase: http://unix.stackexchange.com/questions/275438/kali-linux-2-0-cant-install-build-essentials


import general_use
import os

distro = general_use.check_distribution()

def github_tools(d, toolname):
	'''
		This function installs the tools that use github
	'''
	pack_man = general_use.package_tool(d)

	#will probably edit it so that no matter the tool name, it starts with the link

	general_use.update(distro)

	repo_socketCAN = "https://github.com/linux-can/can-utils.git"
	repo_canbus_utils = 'https://github.com/digitalbond/canbus-utils'

	if toolname == 'can-utils': #this will install SocketCAN first as it is needed to be able to run can-utils
		print ("Cloning SocketCAN repository...")
		socket = subprocess.run(["git", "clone", repo_socketCAN])
		if socket.returncode != 0:
			print ('CLONING FAILED: Failed to clone socketCAN repository at', repo_socketCAN)
			print ('WITH ERROR CODE: ', socket.returncode)
		elif socket.returncode == 0:
			print ('CLONING SUCCESSFUL: Successfully cloned SocketCAN repository')
			print ('Installing can-utils...')
			can_utils = subprocess.run(["sudo", pack_man, "can-utils"])
			if can_utils.returncode != 0:
				print ("INSTALLATION FAILED: Failed to install can-utils")
				print ('WITH ERROR CODE: ', can_utils.returncode)
			elif can_utils.returncode == 0:
				print ('INSTALLATION SUCCESSFUL: Successfully installed can-utils')
				print ('Ensuring CAN modules are enabled...')
				lkm = subprocess.run(['sudo', 'modprobe', 'can']) 	#not sure if going to keep this yet, also need to check if redhat has modprobe, it should but need to check (also just check generally if other linux has this already installed)
				if lkm.returncode != 0:
					print ('CHECK FAILED: Failed to add a LKM to the kernel')
					print ('WITH ERROR CODE: ', lkm.returncode)
				elif lkm.returncode == 0:
					print ('CHECK SUCCESSFUL: Successfully added a LKM to the kernel')
	elif toolname == 'canbus-utils'	#find out if socketCAN needs to be installed to be able to use it
		print ('Cloning repository for canbus-utils...')
		canbus = subprocess.run(['git', 'clone', repo_canbus_utils])
		if canbus.returncode != 0:
			print ('CLONING FAILED: Failed to clone canbus-utils repository at', repo_canbus_utils)
			print ('WITH ERROR CODE:', canbus.returncode)
		elif canbus.returncode == 0:
			print ('CLONING SUCCESSFUL: Successfully cloned canbus-utils repository')
			current_dir = os.getcwd()
			path = current_dir + '/canbus-utils'
			os.chdir(path)
			dependencies.check_NPM(d)
			install = subprocess.run(['npm', 'install'])


