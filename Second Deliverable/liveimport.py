import csv
import matplotlib.pyplot as plt
import numpy as np
import math
from operator import itemgetter
import matplotlib.animation as animation

#class CANdata:
#	def __init__(self):
#		self.list = []
#	def addrow(self,row):
#		self.list.append(row)
#		return


def parsedata(data, filename):
	with open(filename, newline='') as csvfile:
		canreader = csv.reader(csvfile, delimiter=',', quotechar='|')
		for ii in range (1,38):
			next(canreader)
		for row in canreader:
			data.append(row)
	return

def finduniques(list):
	templist = []
	for x in list:
		templist.append(x[9])
	dataset = set(templist)
	return dataset

def printArbId(list):
	for x in list:
		print ('{:>12}  {:>12}'.format(x[0][9], len(x)))

def plotArbIdf(starttime,endtime,list,inc):
	plotlist = np.asarray(list)
	plt.ylabel('number of calls')
	plt.xlabel('time (s)')
	plt.ion()
	# for i in range(int(starttime),int(endtime)):
	tracker = inc
	freq = 0
	for x in list:
		if float(x[1])-starttime <= tracker:
			freq += 1
		else:
			plt.scatter(tracker,freq)
			plt.pause(0.05)
			freq = 0
			tracker += inc
	# freqlist = [0] * (int(endtime-starttime)+1)
	# for x in list:
	# 	timefloor = math.floor(float(x[1])-starttime)
	# 	freqlist[timefloor] += 1
	# plt.plot(freqlist, 'ro')
	# plt.show()

def plotArbIdc(starttime,endtime,list,inc):
	plotlist = np.asarray(list)
	plt.ylabel('number of calls')
	plt.xlabel('time (s)')
	plt.ion()
	# for i in range(int(starttime),int(endtime)):
	tracker = inc
	freq = 0
	xvars = []
	yvars = []
	for x in list:
		if float(x[1])-starttime <= tracker:
			freq += 1
		else:
			xvars.append(tracker)
			yvars.append(freq)
			plt.plot(xvars,yvars, 'r-')
			plt.scatter(tracker,freq)
			plt.pause(0.05)
			tracker += inc

def plotArbIdchanges(starttime,endtime,list,inc):
	plotlist = np.asarray(list)
	plt.ylabel('number of calls')
	plt.xlabel('time (s)')
	plt.ion()
	# for i in range(int(starttime),int(endtime)):
	tracker = inc
	freq = 0
	prevfreq = 0
	for x in list:
		if float(x[1])-starttime <= tracker:
			freq += 1
		else:
			plt.scatter(tracker,freq-prevfreq)
			plt.pause(0.05)
			prevfreq = freq
			freq = 0
			tracker += inc

def main():
#	data = CANdata();+
	data = []
	parsedata(data,'can1.csv')
	# print ('Input filename (enter n if that is the last of the data): ')
	# userinput = input()
	# while (userinput != 'n'):
	# 	parsedata(data,userinput)
	# 	print ('Input filename (enter n if that is the last of the data): ')
	# 	userinput = input()
	# # print(sorted(data, key=itemgetter(9)))
	uniquedata = []
	dataset = finduniques(data)
	for x in dataset:
		templist = [row for row in data if row[9] == x]
		uniquedata.append(templist)
	##########uniquedata is where the data by ARB ID is stored
	# for x in uniquedata[3]:
	# 	print (x)

	while (True):
		print ('\n(1) list all ARB IDs')
		print ('(2) print ARB ID calls over time (frequency)')
		print ('(3) print ARB ID calls over time (cumulative)')
		print ('(4) print ARB ID calls over time (changes)')
		print ('(5) write data to file')
		command = input('Input command: ')
		inc = 1
		if (command == '1'):
			printArbId(uniquedata);
		elif (command == '2'):
			arbID = input('ARB ID to search for: ')
			for row in uniquedata:
				if row[0][9] == arbID:
					plotArbIdf(float(data[0][1]),float(data[len(data)-1][1]), row, inc)
					break;
		elif (command == '3'):
			arbID = input('ARB ID to search for: ')
			for row in uniquedata:
				if row[0][9] == arbID:
					plotArbIdc(float(data[0][1]),float(data[len(data)-1][1]), row, inc)
					break;
		elif (command == '4'):
			arbID = input('ARB ID to search for: ')
			for row in uniquedata:
				if row[0][9] == arbID:
					plotArbIdchanges(float(data[0][1]),float(data[len(data)-1][1]), row, inc)
					break;
		elif (command == '5'):
			writeto = input ('Enter file to write to: ')
			print ('Writing to ' + writeto + '.csv...')
			with open(writeto + '.csv', 'w') as csvfile:
				csvwrite = csv.writer(csvfile, quoting=csv.QUOTE_ALL)
				for iid in uniquedata:
					for row in iid:
						csvwrite.writerow(row)
			print ('Write complete.')





if __name__ == "__main__":
	main()
