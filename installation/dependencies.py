'''
	This script checks and/or installs necessary dependencies to run tools
'''


import general_use

def commandline_install(pack_man, i): # maven, pip 
	general_use.update(d)
	c = subprocess.run(['sudo', pack_man, '-y', 'install', i])
	return c.returncode

def download_install(toolname, link):
	d = subprocess.run(['curl', '-o', toolname, link]) 
	return d.returncode

def clone_git_repo(repo):
	print ("Cloning repository...")	#might install 
	clone = subprocess.run(["git", "clone", repo])
	return clone.returncode






def install_pyserial(link):
	down_load = subprocess.run(['curl', '-o', 'pyserial', link]) #FYI MIGHT BE ABLE TO RUN PIP INSTALL PYSERIAL 
	if down_load.returncode != 0:
			print ('DOWNLOAD FAILED: Failed to download pyserial. This is needed to run pyobd')
			print ('WITH ERROR CODE:', down_load.returncode)
			return down_load.returncode
		else:
			print ('DOWNLOAD SUCCESSFUL: Successfully downloaded pyserial')
			print ('Building PySerial 2.0 package...')
			current_dir = os.getcwd()
			path = current_dir + '/pyserial-2.0'
			os.chdir(path)
			install = subprocess.run(['python', 'setup.py', 'build'])
			if install.returncode != 0:
				print ('BUILD FAILED: Failed to build pyserial. This is needed to run pyobd. Cannot finish PyOBD installation')
				print ('WITH ERROR CODE:', install.returncode)
				return install.returncode
			else:
				print ('BUILD SUCCESSFUL: Successfully completed pyserial build')
				print ('Installing pyserial...')
				i = subprocess.run(['sudo', 'python', 'setup.py', 'install'])
				if i.returncode != 0:
					return i.returncode
				else:
					return 0

def install_NPM(pack_man):

	print ('Installing npm...')
		npm = install_general(pack_man, 'npm')
		if npm != 0:
			print ('INSTALLATION FAILED: Could not install npm. This package is necessary to run canbus-utils')
			print ('WITH ERROR CODE: ', npm)
		elif npm == 0:
			print ('INSTALLATION SUCCESSFUL: Successfully installed npm')

def check_symlink(source_file, symlink):
	
	check = subprocess.run(['test', '-h', symlink])
	if check.returncode != 0:
		print ('Creating symbolic link between', source_file, 'and', symlink, '...')
		symlink = subprocess.run(['sudo', 'ln', '-s', source_file, symlink]) #this might not work
		if symlink.returncode != 0:
			print ('SYMLINK CREATION FAILED: Failed to create a symbolic link between', source_file, 'and', symlink)
		elif symlink.returncode == 0:
			print ('SYMLINK CREATION SUCCESSFUL: Successfully created a symbolic link between', source_file, 'and', symlink)
		return symlink.returncode
	else:
		print ('Symbolic link already created between', source_file, 'and', symlink)
		return 0

def check_NPM(pack_man): #cant decide if should do this / count this as basics, like install ruby and gcc and homebrew so that later can just run brew install node? 

	check_node_existence = subprocess.run(['command', '-v', 'node'])
	check_npm_existence = subprocess.run(['command', '-v', 'npm'])

	if check_node_existence.returncode == 0:
		if check_npm_existence.returncode == 0:
			print ('CONFIRMATION COMPLETE: Both node.js and npm are installed')
		elif check_npm_existence.returncode != 0:
			install_NPM(pack_man)
	elif check_node_existence.returncode != 0:
		print ('Installing nodejs...')
		node = install_general(pack_man, 'nodejs')
		if node != 0:
			print ('INSTALLATION FAILED: Could not install nodejs. This package is necessary to run npm which is used for canbus-utils')
			print ('WITH ERROR CODE: ', node)
		elif node == 0:
			print ('INSTALLATION SUCCESSFUL: Successfully installed nodejs')
			print ('Installing npm...')
			install_NPM(pack_man)	

	print ('Checking for symbolic link existence between nodejs (/usr/bin/nodejs) and npm (/usr/bin/node)')
	return check_symlink('/usr/bin/nodejs', '/usr/bin/node')

	print ('Confirming complete installation of nodejs and npm...') #dk if will keep this yet, we'll see
	node_check = subprocess.run(['command', '-v', 'node'])
	npm_check = subprocess.run(['command', '-v', 'npm'])
	if node_check.returncode == 0 and npm_check.returncode == 0:
		print ('CONFIRMATION COMPLETE: Successfully confirmed installation of both node.js and npm')
		return 0
	else:
		if node_check.returncode != 0:
			print ('CONFIRMATION FAILED: Could not confirm installation of node.js')
			print ('WITH ERROR CODE: ', node_check.returncode)
			return node_check.returncode
		elif npm_check.returncode != 0:
			print ('CONFIRMATION FAILED: Could not confirm installation of npm')
			print ('WITH ERROR CODE: ', npm_check.returncode)
			return npm_check.returncode

def install_bluez():
	print ('Installing Bluez...')
	print ('Cloning Bluez repository...')
	clone = clone_git_repo(repo_bluez)
	if clone.returncode != 0:
		print ('CLONING FAILED: Failed to clone repository at', repo)
		print ('WITH ERROR CODE: ', clone.returncode)
	elif clone.returncode == 0:
		print ('CLONING SUCCESSFUL: Successfully cloned repository at', repo)
		current_dir = os.getcwd()
		path = current_dir + '/bluez-tools-master'
		os.chdir(path)
		config = subprocess.run(['./configure'])
		if config != 0:
			print ('CONFIGURATION FAILED: Failed to run ./configure after cloning Bluez repo')
			print ('WITH ERROR CODE:', config)
			return config
		else:
			print ('CONFIGURATION SUCCESSFUL: Successfully ran ./configure after cloning Bluez repo')
			print ('Compiling...')
			make = subprocess.run(['make'])
			if make.returncode != 0:
				print ('COMPILATION FAILED: Failed to compile bluez package')
				print ('WITH ERROR CODE:', make.returncode)
				return make.returncode
			else:
				print ('COMPILATION SUCCESSFUL: Successfully compiled bluez package')
				print ('Installing Bluez...')
				make_install = subprocess.run(['make', 'install'])
				if make_install.returncode != 0:
					print ('INSTALLATION FAILED: Failed to make install.')
					print ('WITH ERROR CODE:', make_install.returncode)
					return make_install.returncode
				else:
					print ('INSTALLATION SUCCESSFUL: Successfully ran make install')
					return 0












