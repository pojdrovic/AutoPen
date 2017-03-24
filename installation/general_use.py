'''

	This script has functions that can be used across all scripts

'''

import platform
import subprocess

def check_distribution():

	distro = ''
	system = platform.system() #could use this to check whether it's a linux system or not
	distribution = (platform.linux_distribution())[0]

	if 'Kali' == distribution or 'kali' == distribution: #maybe make this to do general debian based tools
		distro = 'kali'
	elif 'Debian' == distribution or 'debian' == distribution:
		distro = 'debian'
	elif 'Ubuntu' == distribution or 'ubuntu' == distribution:
		distro = 'ubuntu'
	elif 'Red Hat' == distribution or 'red hat' == distribution: #make this so that it checks fedora 
		distro = 'red hat'

	return distro

def package_tool(d):
	pt = ''

	if d == 'kali' or d == 'debian' or d == 'ubuntu':
		pt = 'apt-get'
	elif d == 'red hat':
		pt = 'yum'

	return pt

def update(d):
	'''
		This function updates software packages based on repository state
		This needs to be included at the beginning of every major installation function
	'''	

	pack_man = package_tool(d)
	update = subprocess.run(["sudo", pack_man, "update"])