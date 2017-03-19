import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import csv

def parsedata(data, filename):
	with open(filename, newline='') as csvfile:
		canreader = csv.reader(csvfile, delimiter=',', quotechar='|')
		for ii in range (1,38):
			next(canreader)
		for row in canreader:
			data.append(row)
	return

def finduniques(arbdata):
	temparbdata = []
	for x in arbdata:
		temparbdata.append(x[9])
	dataset = set(temparbdata)
	return dataset

def filterArbIdData(arbdata):
	temparbdata = []
	for x in arbdata:
		stringId = x[12]+","+x[13]+","+x[14]+","+x[15]+","+x[16]+","+x[17]+","+x[18]+","+x[19]
		temparbdata.append(stringId)
	dataset = set(temparbdata)
	return dataset

#blit plotting
def plotArbIdf(starttime,endtime,arbdata,inc):
	plotarbdata = np.asarray(arbdata)
	xvars, yvars = findvars(starttime, endtime, arbdata, inc)
	x=xvars[0:10]
	y=yvars[0:10]
	fig, axes = plt.subplots(nrows=6)
	styles = ['r-', 'g-', 'y-', 'm-', 'k-', 'c-']

	def plot(ax, style):
		return ax.plot(x, y, style, animated=True)[0]
	lines = [plot(ax, style) for ax, style in zip(axes, styles)]

	# We'd normally specify a reasonable "interval" here...
	def animate(i):
		for j, line in enumerate(lines, start=6):
			line.set_ydata(yvars[int(0+i):int(10+i)])
		return lines
	ani = animation.FuncAnimation(fig, animate, range(1, 15), interval=50, blit=True)
	plt.show()

def findvars(starttime, endtime, arbdata, inc):
	x = []
	y = []
	tracker = 0
	freq = 0
	for sec in arbdata:
		if float(sec[1])-starttime <= tracker:
			freq+=1
		else:
			x.append(tracker)
			y.append(freq)
			freq=0
			tracker += inc
	return x,y

def main():

	data = []
	parsedata(data,'can1.csv')
	uniquedata = []
	dataset = finduniques(data)
	for x in dataset:
		temparbdata = [row for row in data if row[9] == x]
		uniquedata.append(temparbdata)
	arbID = '185'
	inc = 1
	for row in uniquedata:
		if row[0][9] == arbID:
				plotArbIdf(float(data[0][1]),float(data[len(data)-1][1]), row, inc)

if __name__ == "__main__":
	main()