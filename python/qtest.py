import qwtwc as q   # add plotting library wrapper
import numpy as np  # for number manupulation

# start the system
q.start()
#q.test() # run simple test

#create  some data
x = np.linspace(0., 10., 500)
y1 = np.sin(x) + np.cos(x * 1.8)
y2 = np.sin(x) + np.cos(x * 0.8)

x5 = np.linspace(0., 10., 12)
y5 = np.sin(x5*0.7) + np.cos(x5 * 1.5)

# draw/select a plot, with 'ID' = 1
q.fig(1)
q.line(np.stack((x, y1)) )  # first line
q.line(np.stack((x, y2)), 'test #2', '-r', 3, 1)   # add more lines
q.line(np.stack((x5, y5)), "test #3", " tm", 1, 8)   # more lines
q.xlab('seconds?')                  # add X axis label
q.ylab('something')                 # add y axis label
q.name('a plot 123')                # add plot title

# create some more data:
M = np.max(x) / (2.0 * 3.14159)
x1 = np.sin(x / M)
x2 = np.cos(x / M)
x11 = x1*0.8 + 0.7
x12 = x2*0.8 - 0.5

# draw a new plot, with 'default' ID
q.fig(0)
q.line(np.stack((x1, x2, x)),  "test #21", "-k", 3)    # add first circle
q.line(np.stack((x11, x12, x)),  "test #22", "-Y", 3)  # add another circle
q.xlab('meters')
q.ylab('yards')
q.name('a plot 234')

# another bunch of plots: we can plot a list of lists quite 
p = []
for i in range(0, 500):
    p.append([i / 100.0, np.sin(i / 100.), np.cos(i / 100.), np.cos(i / 85.) + np.sin(i / 42.0), np.cos(i / 75.) + np.sin(i / 62.0) + np.cos(i / 31) * 0.28])
q.plot(p, 'a bunch of plots')    

# create a map:
north = np.array([55.688713, 55.698713, 55.678713, 55.60, 55.61])  # north coords
east = np.array([37.901073, 37.911073, 37.905073, 37.9, 37.85 ])    # east coords
e = np.max(x)
mt = np.array([e * 0.2, e * 0.4, e * 0.6, e * 0.88, e * 0.98])        # attach 'time' info
q.map(0)
q.line(np.stack((east, north, mt)),  "a red line on a map", "-r", 3)    # add the line
q.name('map demo')

q.mw()  # add a list of all the plots to the UI

input('press any key')