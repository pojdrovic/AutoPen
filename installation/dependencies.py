'''
	This script checks and/or installs necessary dependencies to run tools
'''


import general_use

def install_general(pack_man, i): # maven, pip 
	general_use.update(d)
	c = subprocess.run(['sudo', pack_man, 'install', i])
	return c.returncode

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
