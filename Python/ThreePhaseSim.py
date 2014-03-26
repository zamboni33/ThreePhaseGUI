#!/usr/bin/env python

import os, sys, math, datetime, subprocess, glob, time

import gobject


import numpy as np
from matplotlib.lines import Line2D
import matplotlib.pyplot as plt
import matplotlib.animation as animation

from matplotlib.figure import Figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtkagg import NavigationToolbar2GTKAgg as NavigationToolbar
import threading

try:
  import pygtk
  pygtk.require ("2.0")
except:
  pass
try:
  import gtk
  import gtk.glade
except:
  sys.exit(1)


class ThreePhaseSim(threading.Thread):
    def __init__(self):
        
        threading.Thread.__init__(self)
        self.gladefile = "/home/xander/Escritorio/Python/ThreePhaseSim.glade"
        #self.gladefile = os.path.abspath(os.path.join(__file__, os.pardir)) + "/home/amorgan/Desktop/ThreePhaseSim.glade"
        self.wMainTree = gtk.glade.XML(self.gladefile, "main_window")
        self.mainWindow = self.wMainTree.get_widget("main_window")
        self.mainWindow.connect("delete-event", self.exitWindow)
        
        # initialize text values
        self.load = self.wMainTree.get_widget("load_value")
        self.load.connect("value-changed", self.scale_moved)
        self.frequency = self.wMainTree.get_widget("frequency_value")
        #~ self.frequency.connect("activate", self.freq_moved)
        self.slip = self.wMainTree.get_widget("slip_value")
        
        self.frequency.set_text(str(20))
        self.slip.set_text(str(10))

        self.graph = self.wMainTree.get_widget("load_graph")
        vbox = self.wMainTree.get_widget("vbox2")
        #~ self.mainWindow.add(vbox)

        fig = Figure(figsize=(5,4), dpi=100)
        ax = fig.add_subplot(111)
        t = np.arange(0.0,3.0,0.01)
        s = np.sin(2*math.pi*t)

        ax.plot(t,s)


        canvas = FigureCanvas(fig)  # a gtk.DrawingArea
        vbox.pack_start(canvas)
        toolbar = NavigationToolbar(canvas, self.mainWindow)
        vbox.pack_start(toolbar, False, False) 
        
        # show window
        self.mainWindow.show_all()
        
        
    def run(self):
        while True:
            x = 0
            time.sleep(1)
            #~ plt.show()   
            gtk.main() 		
    
    def scale_moved(self, Widget):
        #print(self.load.get_value())
        # While statement in here that spins on slip value until 
		# equilibrium re-established
        print(self.load.get_value()) 
        self.frequency.set_text(str(20))
        self.slip.set_text(str(10))
        ani = animation.FuncAnimation(fig, oscopeLoad.update, emitterLoad, interval=10,blit=True)
        return

    def exitWindow(self, Widget, data):
        plt.close('all')
        gtk.main_quit()
        return
        

class Scope(threading.Thread):
    def __init__(self, ax1, ax2, ax3, maxt=2, dt=0.02):
        threading.Thread.__init__(self)
        self.ax = ax1
        self.dt = dt
        self.maxt = maxt
        self.tdata = [0]
        self.ydata = [0]     
        self.line = Line2D(self.tdata, self.ydata)
        self.ax.add_line(self.line)
        self.ax.set_ylim(0.0, 100.0)
        self.ax.set_xlim(0, self.maxt)
        
        self.ax2 = ax2
        self.dt2 = dt
        self.maxt2 = maxt
        self.tdata2 = [0]
        self.ydata2 = [0]        
        self.line2 = Line2D(self.tdata2, self.ydata2)
        self.ax2.add_line(self.line2)
        self.ax2.set_ylim(0.0, 100.0)
        self.ax2.set_xlim(0, self.maxt)
        
        self.ax3 = ax3
        self.dt3 = dt
        self.maxt3 = maxt
        self.tdata3 = [0]
        self.ydata3 = [0]        
        self.line3 = Line2D(self.tdata3, self.ydata3)
        self.ax3.add_line(self.line3)
        self.ax3.set_ylim(0.0, 100.0)
        self.ax3.set_xlim(0, self.maxt)
        #~ self.start()

    def update(self, y):
        #~ print("Update: " + str(y))
        lastt = self.tdata[-1]
        if lastt > self.tdata[0] + self.maxt: # reset the arrays
            self.tdata = [self.tdata[-1]]
            self.ydata = [self.ydata[-1]]
            self.ax.set_xlim(self.tdata[0], self.tdata[0] + self.maxt)
            self.ax.figure.canvas.draw()

        t = self.tdata[-1] + self.dt
        self.tdata.append(t)
        self.ydata.append(y)
        self.line.set_data(self.tdata, self.ydata)
        return self.line,
        
    def update2(self, y):
        #~ print("Update: " + str(y))
        lastt = self.tdata2[-1]
        if lastt > self.tdata2[0] + self.maxt2: # reset the arrays
            self.tdata2 = [self.tdata2[-1]]
            self.ydata2 = [self.ydata2[-1]]
            self.ax2.set_xlim(self.tdata2[0], self.tdata2[0] + self.maxt2)
            self.ax2.figure.canvas.draw()

        t = self.tdata2[-1] + self.dt2
        self.tdata2.append(t)
        self.ydata2.append(y)
        self.line2.set_data(self.tdata2, self.ydata2)
        return self.line2,
        
    def update3(self, y):
        #~ print("Update: " + str(y))
        lastt = self.tdata3[-1]
        if lastt > self.tdata3[0] + self.maxt3: # reset the arrays
            self.tdata3 = [self.tdata3[-1]]
            self.ydata3 = [self.ydata3[-1]]
            self.ax3.set_xlim(self.tdata3[0], self.tdata3[0] + self.maxt3)
            self.ax3.figure.canvas.draw()

        t = self.tdata3[-1] + self.dt3
        self.tdata3.append(t)
        self.ydata3.append(y)
        self.line3.set_data(self.tdata3, self.ydata3)
        return self.line3,    
        
	def run(self):
		print("oscopeLoad Thread Open")
		while True:
			print("Scope Thread")
        
def emitterLoad():
    'return a random value with probability p, else 0'
    #while True:
    yield threePhase.load.get_value()

def emitterFreq():
    'return a random value with probability p, else 0'
    #while True:
    #~ print(threePhase.frequency.get_text())
    yield float(threePhase.frequency.get_text())
    
def emitterSlip():
    'return a random value with probability p, else 0'
    #while True:
    yield float(threePhase.slip.get_text())
                
if __name__ == "__main__":
    gtk.gdk.threads_init()
    
    threePhase = ThreePhaseSim()
    threePhase.start()
    
    fig, ax = plt.subplots()
    ax2 = fig.add_subplot(111)
    ax3 = fig.add_subplot(111)
    oscopeLoad = Scope(ax, ax2, ax3)
    print("Starting second thread")
    oscopeLoad.start()
    
    print("Objects Made")
    
    ani = animation.FuncAnimation(fig, oscopeLoad.update, emitterLoad, interval=40,blit=True)
    ani2 = animation.FuncAnimation(fig, oscopeLoad.update2, emitterFreq, interval=40,blit=True)
    ani3 = animation.FuncAnimation(fig, oscopeLoad.update3, emitterSlip, interval=40,blit=True)
  		
    plt.show()    		

    gtk.main()


