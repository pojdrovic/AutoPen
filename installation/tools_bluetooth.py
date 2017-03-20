'''

TOOLS TO DO:
	command line tools: 
		hciconfig
		hcitool
		l2ping
		hcidump
		blueranger
	gui:
		btscanner
		bluemaho
		redfang
		spooftooph

# NEED TO WRITE SCRIPTS TO IMPLEMENT ATTACKS OR GUIDE USERS

'''

repo_bluelog = 'https://github.com/MS3FGX/Bluelog.git'

#dependencies links
repo_bluez = 'https://github.com/khvzak/bluez-tools.git'
link_package = 'https://pkgconfig.freedesktop.org/releases/pkg-config-0.21.tar.gz'


def github_tools(distro, toolname, repo):
	pack_man = general_use.package_tool(d)
	general_use.update(d)

	clone = dependencies.clone_git_repo(repo)
	if clone != 0:
		print ('CLONING FAILED: Failed to clone repository at', repo)
		print ('WITH ERROR CODE: ', clone)
	elif clone == 0:
		print ('CLONING SUCCESSFUL: Successfully cloned repository at', repo)
		if toolname == 'bluelog': #has an optional web mode so when running want to add that functionality.  (just run make to run)
			print ('INSTALLATION SUCCESSFUL: Successfully installed bluelog')
		elif toolname == 'bluemaho':
			wxpython = dependencies.install_general(pack_man, 'python-wxgtk2.6')
			bluez = dependencies.install_bluez()
			config = dependencies.download_general('pkg-config', link_package)

			depend = ['libopenobex1', 'libopenobex-dev', 'libxml2', 'libxml2-dev', 'libusb-dev', 'libreadline5-dev', 'lightblue-0.3.3']
			returncodes = [dependencies.install_general(pack_man, i) for i in libraries]
			depend.append('wxpython')
			depend.append('bluez')
			depend.append('config')
			returncodes.append(wxpython)
			returncodes.append(bluez)
			returncodes.append(config)

			essential = 0

			for i,j in enumerate(returncodes):
				if j != 0:
					if i < 7:
						print ('INSTALLATION FAILED: Failed to install dependency', depend[i], '. May remove the ability to run a specific attack using Bluemaho. Please refer to the github repo')
						print ('WITH ERROR CODE:', j)
					else:
						print ('INSTALLATION FAILED: Failed to install dependency', depend[i])
						essential = -1

			if essential != 0:
				print ('INSTALLATION FAILED: Failed to install Bluemaho dependencies: either wxPython, bluez or pkg-config')
			else:
				print ('INSTALLATION SUCCESSFUL: Successfully installed all dependencies for Bluemaho')
				print ('Builiing Bluemaho...')
				build = subprocess.run([./build.sh])
				if build.returncode != 0:
					print ('BUILD FAILED: Failed to build and complete installation of Bluemaho')
					print ('WITH ERROR CODE:'cd)








