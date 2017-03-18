'''

#Reference Links for me, might remove later we'll see how life goes 
	can-utils : https://discuss.cantact.io/t/using-can-utils/24 (eric evenchick)
	canbus-utils : http://www.digitalbond.com/blog/2015/03/05/tool-release-digital-bond-canbus-utils/
	NPM: http://blog.teamtreehouse.com/install-node-js-npm-linux
	NPM2: http://www.hostingadvice.com/how-to/install-nodejs-ubuntu-14-04/#node-version-manager (May 2016)
	build-essential can fail, try this just incase: http://unix.stackexchange.com/questions/275438/kali-linux-2-0-cant-install-build-essentials

TO DO: 
	NEED TO CHANGE ALL THE TOOLS TO DOWNLOAD STUFF INTO THEIR FOLDER so that it doesn't just throw everything in one directory (aka cd right after clone/download)

Front End/Back End Connection:
	We only want the whole thing to run if the install all or install all CAN button is pressed
	Else, we need to function to take in distro and the name of the tool depending on the button pressed and then install that

Note:
	ELM327:
		Be forewarned that the ELM327 has limited buffer space, so you will lose packets when sniffing,
 		and transmission can be a bit imprecise, but if you're in a pinch this is the cheapest route.

 	Make sure the user knows that this will all be cloned in the current directory aka most likely /Users/'user'


Tools may be included:
	canibus for ELM327:
		(go-server)
	CANtact:
		will provide a section in here with the github link and the website link so that the user can build their own
		Also, will possibly provide steps on how to do so (will learn how to do this first)

'''

import general_use
import os

#this will be placed somewhere else, maybe a dictionary?
repo_socketCAN = 'https://github.com/linux-can/can-utils.git' #needed to run can-utils
repo_canbus_utils = 'https://github.com/digitalbond/canbus-utils'
repo_kayak = 'https://github.com/dschanoeh/Kayak.git'
repo_caringcaribou = 'https://github.com/CaringCaribou/caringcaribou.git' #want to check this to make sure it works, instructions a bit unclear
repo_c0f = 'https://github.com/zombieCraig/c0f.git'
repo_udsim = 'https://github.com/zombieCraig/UDSim.git'

#for download links
download_pyobd = 'http://www.obdtester.com/download/pyobd_0.9.3.tar.gz'
download_pyobd_debian = 'http://www.obdtester.com/download/pyobd_0.9.3_all.deb'
download_pyserial = 'https://sourceforge.net/projects/pyserial/files/pyserial/2.0/pyserial-2.0.zip/download'
download_o2oo = 'https://www.vanheusden.com/O2OO/O2OO-0.9.tgz'

def github_tools(d, toolname, repo):
	'''
		This function installs the tools that use github
	'''
	pack_man = general_use.package_tool(d)

	general_use.update(d)

	install = ''

	print ("Cloning repository...")	#might install 
	clone = subprocess.run(["git", "clone", repo])
	if clone.returncode != 0:
		print ('CLONING FAILED: Failed to clone repository at', repo)
		print ('WITH ERROR CODE: ', clone.returncode)
	elif clone.returncode == 0:
		print ('CLONING SUCCESSFUL: Successfully cloned repository at', repo)


		#CHANGE DIRECTORY HERE to the folder that is created


		if toolname == 'can-utils': #this will install SocketCAN first as it is needed to be able to run can-utils
			print ('Installing can-utils...')
			can_utils = install_general(pack_man, 'can-utils')
			if can_utils != 0:
				print ("INSTALLATION FAILED: Failed to install can-utils")
				print ('WITH ERROR CODE: ', can_utils)
			elif can_utils == 0:
				print ('INSTALLATION SUCCESSFUL: Successfully installed can-utils')
				print ('Ensuring CAN modules are enabled...')
				lkm = subprocess.run(['sudo', 'modprobe', 'can']) 	#not sure if going to keep this yet mainly cuz might not be necessary, also need to check if redhat has modprobe, it should but need to check (also just check generally if other linux has this already installed)
				if lkm.returncode != 0:
					print ('CHECK FAILED: Failed to add a LKM to the kernel')
					print ('WITH ERROR CODE: ', lkm.returncode)
				elif lkm.returncode == 0:
					print ('CHECK SUCCESSFUL: Successfully added a LKM to the kernel')

		elif toolname == 'canbus-utils':	#find out if socketCAN needs to be installed to be able to use it
			print ('Installing canbus-utils...')
			e = dependencies.check_NPM(pack_man) #needed for the following step
			if e == 0:
				current_dir = os.getcwd()
				path = current_dir + '/canbus-utils'
				os.chdir(path)
				install = subprocess.run(['npm', 'install'])
				if install.returncode != 0:
					print ('INSTALLATION FAILED: Failed to install canbus-utils. Problem running "npm install" ')
					print ('WITH ERROR CODE: ', install.returncode)
				else:
					print ('INSTALLATION SUCCESSFUL: Successfully installed canbus-utils')
			elif e != 0:
				print ('INSTALLATION FAILED: Could not install canbus-utils. Check node.js and npm status')

		elif toolname == 'kayak':
			print ('Installing kayak...')
			e = dependencies.install_general(pack_man, 'maven')
			if e == 0:
				current_dir = os.getcwd()
				path = current_dir + '/Kayak'
				os.chdir(path)
				kayak = subprocess.run(['mvn', 'clean', 'package'])
				if kayak.returncode != 0:
					print ('INSTALLATION FAILED: Failed to install kayak')
					print ('WITH ERROR CODE: ', kayak.returncode)
				else:
					print ('INSTALLATION SUCCESSFUL: Successfully installed kayak')

		elif toolname == 'caringcaribou'
			#have a button that pops up that says that the user will have to set up there device, and have some steps on doing that (front end)
			print ('Installing caringcaribou...')
			load = subprocess.run(['sudo', 'modprobe', 'can'])
			if load.returncode != 0:
				print ('LOAD FAILED: Failed to load CAN module')
				print ('WITH ERROR CODE: ', load.returncode)
			elif load.returncode == 0:
				print ('LOAD SUCCESSFUL: Successfully loaded CAN module')
				#user needs to input the CAN bus they are on (can we fingerprint how many canbuses are on?)
				#user needs to know the bitrate that the bus runs with
				bitrate = 500000 #HARDCODED FOR NOW
				print ('Setting up CAN device...')
				setup = subprocess.run(['sudo', 'ip', 'link', 'set', 'can0', 'up', 'type', 'can', 'bitrate', bitrate])
				if setup.returncode != 0:
					print ('SETUP FAILED: Failed to set-up can device.')
					print ('WITH ERROR CODE: ', setup.returncode)
				elif setup.returncode == 0:
					print ('SETUP SUCCESSFUL: Successfully set-up can device. This will now display as a normal network interface as can0')
					pythoncan = subprocess.run(['curl', '-o', 'can', 'https://bitbucket.org/hardbyte/python-can/get/77eea796362b.zip'])
					if pythoncan.returncode != 0:
						print ('DOWNLOAD FAILED: Failed to download pythoncan from https://bitbucket.org/hardbyte/python-can/get/77eea796362b.zip')
						print ('WITH ERROR CODE: ', pythoncan.returncode)
					elif pythoncan.returncode == 0:
						print ('DOWNLOAD SUCCESSFUL: Successfully downloaded pythoncan from https://bitbucket.org/hardbyte/python-can/get/77eea796362b.zip')
						e = install_general_command(pack_man, 'python-pip')
						if e != 0:
							print ('INSTALLATION FAILED: Failed to install pip. Cannot finish caringcaribou installation')
							print ('WITH ERROR CODE: ', e)
						else:
							print ('INSTALLATION COMPLETE: Successfully installed pip')
							install = subprocess.run(['sudo', 'python', 'setup.py', 'install'])
							if install.returncode != 0:
								print ('DOWNLOAD SUCCESSFUL: Failed to install python-can')
								print ('WITH ERROR CODE: ', install.returncode)
							else:
								print ('INSTALLATION SUCCESSFUL: Successfully installed python-can')


								# NEED TO HANDLE THE CONFIGURATION FILE, ARE WE GOING TO TRY TO DO THIS OR IS THE USER GOING TO DO THIS

		elif toolname == 'c0f'
			print ('Installing c0f')
			e = install_general(pack_man, 'rubygems')
			if e != 0:
				print ('INSTALLATION FAILED: Failed to install gem. Cannot finish c0f installation')
				print ('WITH ERROR CODE: ', e)
			else:
				print ('INSTALLATION SUCCESSFUL: Successfully installed gem')
				install = subprocess.run(['gem', 'install', 'c0f'])
				if install.returncode != 0:
					print ('INSTALLATION FAILED: Failed to install c0f')
					print ('WITH ERROR CODE: ', install.returncode)
				else:
					print ('INSTALLATION SUCCESSFUL: Successfully installed c0f')

		elif toolname == 'udsim': #know later that when user starts the program they have to run make in the file that they are in. Or maybe we can run make
			number = 0
			dev = dependencies.install_general(packman, 'libsdl2-dev')
			image = dependencies.install_general(packman, 'libsdl2-image')
			ttf = dependencies.install_general(packman, 'libsdl2-ttf')
			returncode_list = [dev, image, ttf]
			for i, j in enumerate(returncode_list):
    			if j != 0:
    				number = -1
        			print ('INSTALLATION FAILED: Could not install library libsdl2-', i)
        			print ('WITH ERROR CODE: ', j)
        		else:
        			print ('INSTALLATION SUCCESSFUL: Successfully installed library libsdl2-', i)
        	if number == 0:
        		print ('INSTALLATION COMPLETE: Successfully installed the libraries needed to compile UDSIM.')

        elif toolname == 'canibus':


        	#TO DO 

def downloaded_tools(d, toolname, link): #WxPython and some other library
	pack_man = general_use.package_tool(d)
	general_use.update(d)

	#NOTE: If pyOBD link doesn't work tell them the install.html is available

	down = download_general(toolname, link)
	if down != 0:
		print ('DOWNLOAD FAILED: Failed to download file for', toolname, 'using download link:', link)
		print ('WITH ERROR CODE: ', down)
	elif down == 0:
		print ('DOWNLOAD SUCCESSFUL: Successfully downloaded file for' toolname)

		if toolname == 'pyobd':
			if d == 'debian':
				deb = dependencies.download_general('pyobd_0.9.3_all.deb', download_pyobd_debian)
				if deb != 0:
					print ('Download Failed: Failed to download debian a')
			print ('Beginning pyserial installation...')
			pyserial = dependencies.download_general('pyserial', download_pyserial)
			if pyserial != 0:
				print ('INSTALLATION FAILED: Failed to install pyserial. Cannot complete pyobd installation')
				print ('WITH ERROR CODE:', pyserial)
			else:
				print ('INSTALLATION SUCCESSFUL: Successfully installed pyserial and pyobd')	
		elif toolname = 'o2oo':
			print ('Installing o2oo...')
			extract = subprocess.run(['tar', '-xzvf', 'O2OO-0.9.tar'])
			if extract.returncode != 0:
				print ('EXTRACTION FAILED: Failed to decompress the o2oo tar file')
				print ('WITH ERROR CODE:', extract.returncode)
			else:
				print ('EXTRACTION SUCCESSFUL: Successfully extracted o2oo tar file. o2oo has been successfully installed')























