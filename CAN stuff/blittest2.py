import time
import matplotlib.pylab as plt
import random

fig = plt.figure()
ax1 = fig.add_subplot(1, 1, 1)

def plotArb(ax):
    ax.cla()
    ax.set_title('Sensor Input vs. Time -' + 'Blit [{0:3s}]'.format("On"))
    ax.set_xlabel('Time (s)')
    ax.set_ylabel('Number of calls')

    plt.ion()  # Set interactive mode ON, so matplotlib will not be blocking the window
    plt.show(False)  # Set to false so that the code doesn't stop here

    ax.hold(True)

    xinput, yinput = [1,2,3,4,5,6,7,8,9,10], [5,6,1,2,10,4,4,4,2,3]
    x, y = [], []
    x.append(0)
    y.append(0)

    line1, = ax.plot(x, y, '.-', alpha=0.8, color="gray", markerfacecolor="red")

    fig.show()
    fig.canvas.draw()

    background = fig.canvas.copy_from_bbox(ax.bbox) # cache the background

    niter = 10
    i = 0
    inputstream = []
    while i < niter:
        x.append(xinput[i])
        y.append(yinput[i])

        # this removes the tail of the data so you can run for long hours. You can cache this
        # and store it in a pickle variable in parallel.

        if len(x) > 5:
           del y[0]
           del x[0]

        xmin, xmax, ymin, ymax = [min(x)*1.1, max(x) * 1.1, min(y)*1.1, max(y) * 1.1]

        # feed the new data to the plot and set the axis limits again
        line1.set_xdata(x)
        line1.set_ydata(y)

        plt.axis([xmin, xmax, ymin, ymax])
            
        fig.canvas.restore_region(background)    # restore background
        ax.draw_artist(line1)                   # redraw just the points
        fig.canvas.blit(ax.bbox)                # fill in the axes rectangle

        i += 1

plotArb(ax1)