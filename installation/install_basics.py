'''

This script begins by first checking the distribution that the user has and will install the following commands/libraries:

git
python3

Assumes user is running as root and has apt-get installed

'''

#send STDERR to STDOUT: http://stackoverflow.com/questions/29580663/save-error-message-of-subprocess-command
#right now it logs everything, once completed add functionality for errors to print in RED and for multiple logs to be made, complete, STDERR, etc.

import platform

distro = ''

def check_distribution():
	system = platform.system() #could use this to check whether it's a linux system or not
	distribution = (platform.linux_distribution())[0]

	if 'Kali' == distribution or 'kali' == distribution:
		distro = 'kali'
	elif 'Debian' == distribution or 'debian' == distribution:
		distro = 'debian'
	elif 'Ubuntu' == distribution or 'ubuntu' == distribution:
		distro = 'ubuntu'
	elif 'Red Hat' == distribution or 'red hat' == distribution:
		distro = 'red hat'

def package_tool(distro):
	pt = ''

	if d == 'kali' or d == 'debian' or d == 'ubuntu':
		pt = 'apt-get'
	elif d == 'red hat':
		pt = 'yum'

	return pt

def install_python(d):
	'''
	This function installs or updates Python 3 depending on whether it is already on the system or not
	'''

	print ('Installing Python 3...')

	pack_man = package_tool(d)

	p = subprocess.run("sudo", pack_man, "install", "python3")
	if p.returncode != 0:
		print ('INSTALLATION FAILED: Could not install Python 3')
		print ("WITH ERROR CODE:", p.returncode)
	elif p.returncode == 0:
		print ('INSTALLATION SUCCESSFUL: Python 3 successfully installed')

def install_git(d):
	'''
		This function installs or updates git depending on whether it is already on the system or not
	'''
	pack_man = package_tool(d)

	print('Installing git...')
	g = subprocess.run("sudo", pack_man, "install", "git")

	if g.returncode != 0:
		print ("INSTALLATION FAILED: Could not install git")
		print ("WITH ERROR CODE:", g.returncode)
	elif g.returncode == 0:
		print ("INSTALLATION SUCCESSFUL: Git successfully installed")


def update(d):
	'''
		This function updates software packages based on repository state
		This needs to be included at the beginning of every major installation function
	'''	
	pack_man = package_tool(d)

	update = subprocess.run(["sudo", pack_man, "update"])


check_distribution()
install_python(distro)
install_git(distro)
update(distro)
