#!/usr/bin/env python

#~ """
#~ This demo demonstrates how to draw a dynamic mpl (matplotlib) 
#~ plot in a wxPython application.
#~ 
#~ It allows "live" plotting as well as manual zooming to specific
#~ regions.
#~ 
#~ Both X and Y axes allow "auto" or "manual" settings. For Y, auto
#~ mode sets the scaling of the graph to see all the data points.
#~ For X, auto mode makes the graph "follow" the data. Set it X min
#~ to manual 0 to always see the whole data from the beginning.
#~ 
#~ Note: press Enter in the 'manual' text box to make a new value 
#~ affect the plot.
#~ 
#~ Eli Bendersky (eliben@gmail.com)
#~ License: this code is in the public domain
#~ Last modified: 31.07.2008
#~ """
import os
import pprint
import random
import sys
import wx

# The recommended way to use wx with mpl is with the WXAgg
# backend. 
#
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
    FigureCanvasWxAgg as FigCanvas, \
    NavigationToolbar2WxAgg as NavigationToolbar
import numpy as np
import pylab


class DataGen(object):
    """ A silly class that generates pseudo-random data for
        display in the plot.
    """
    def __init__(self, init=50):

        self.deltaSlip = 100.0
        self.rpm = 0
        self.stable = True
        self.load = 0
        self.freq = 0
        self.slip = 100
        self.speed = 0
        self.direction = 0
        
    def next(self, newLoad, oldLoad):
        self.load = newLoad
        # Which direciton should frequency head?
        if (newLoad - oldLoad) > 0:
            self.direction = 1
            #~ print ("Direction 1")
        elif (newLoad - oldLoad) < 0:
            self.direction = -1
            #~ print ("Direction -1")
        else:
            self.direction = 0
            #~ print ("Direction 0")
        
        if newLoad == 0:
            # Reset the system
            self.load = newLoad
            self.freq = 0
            self.slip = 100
            self.speed = 0
            
        elif self.direction == 1:
			# Frequency Increase
			self.freq += 2
			desiredFreq = (newLoad * 0.6) + 20
			self.rpm = ((120.0 * desiredFreq) / 3.0)
			self.speed = ((120.0 * self.freq) / 3.0)
			self.slip = (abs(self.rpm - self.speed) / self.rpm ) * 100
			if self.slip < 15:
				self.stable = True
			else:
				self.stable = False
			
        elif self.direction == -1:
			# Frequency Decrease
			self.freq -= 2
			desiredFreq = (newLoad * 0.6) + 20
			self.rpm = ((120.0 * desiredFreq) / 3.0)
			self.speed = ((120.0 * self.freq) / 3.0)
			self.slip = (abs(self.speed - self.rpm) / self.speed ) * 100
			if self.slip < 15:
				self.stable = True
			else:
				self.stable = False
							
        else:
			# Check for stability
            if self.stable == False:
                desiredFreq = (newLoad * 0.6) + 20
                self.rpm = ((120.0 * desiredFreq) / 3.0)
                self.speed = ((120.0 * self.freq) / 3.0)
                if (self.rpm - self.speed) > 0:
					self.freq += 2
					self.slip = (abs(self.rpm - self.speed) / self.rpm ) * 100
					if self.slip < 15:
						self.stable = True
					else:
						self.stable = False
                else:
					self.freq -= 2
					self.slip = (abs(self.speed - self.rpm) / self.speed ) * 100
					if self.slip < 15:
						self.stable = True
					else:
						self.stable = False                     
                    
class BoundControlBox(wx.Panel):
    """ A static box with a couple of radio buttons and a text
        box. Allows to switch between an automatic mode and a 
        manual mode with an associated value.
    """
    def __init__(self, parent, ID, label, initval):
        wx.Panel.__init__(self, parent, ID)
        
        self.value = initval
        
        box = wx.StaticBox(self, -1, label)
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        
        self.radio_auto = wx.RadioButton(self, -1, 
            label="Auto", style=wx.RB_GROUP)
        self.radio_manual = wx.RadioButton(self, -1,
            label="Manual")
        self.manual_text = wx.TextCtrl(self, -1, 
            size=(35,-1),
            value=str(initval),
            style=wx.TE_PROCESS_ENTER)
        
        self.Bind(wx.EVT_UPDATE_UI, self.on_update_manual_text, self.manual_text)
        self.Bind(wx.EVT_TEXT_ENTER, self.on_text_enter, self.manual_text)
        
        manual_box = wx.BoxSizer(wx.HORIZONTAL)
        manual_box.Add(self.radio_manual, flag=wx.ALIGN_CENTER_VERTICAL)
        manual_box.Add(self.manual_text, flag=wx.ALIGN_CENTER_VERTICAL)
        
        sizer.Add(self.radio_auto, 0, wx.ALL, 10)
        sizer.Add(manual_box, 0, wx.ALL, 10)
        
        self.SetSizer(sizer)
        sizer.Fit(self)
    
    def on_update_manual_text(self, event):
        self.manual_text.Enable(self.radio_manual.GetValue())
    
    def on_text_enter(self, event):
        self.value = self.manual_text.GetValue()
    
    def is_auto(self):
        return self.radio_auto.GetValue()
        
    def manual_value(self):
        return self.value


class GraphFrame(wx.Frame):
    """ The main frame of the application
    """
    title = 'Demo: dynamic matplotlib graph'
    
    def __init__(self):
        wx.Frame.__init__(self, None, -1, self.title)
        
        self.datagen = DataGen()
        self.data = [0]
        self.dataFreq = [0]
        self.dataSlip = [100]
        self.dataSpeed = [0]
        self.paused = False
        
        self.dataLoad = [0]
        self.frequency = 0
        self.slip = 1.0
        
        self.create_menu()
        self.create_status_bar()
        self.create_main_panel()
        
        self.redraw_timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.on_redraw_timer, self.redraw_timer)        
        self.redraw_timer.Start(100)

    def create_menu(self):
        self.menubar = wx.MenuBar()
        
        menu_file = wx.Menu()
        m_expt = menu_file.Append(-1, "&Save plot\tCtrl-S", "Save plot to file")
        self.Bind(wx.EVT_MENU, self.on_save_plot, m_expt)
        menu_file.AppendSeparator()
        m_exit = menu_file.Append(-1, "E&xit\tCtrl-X", "Exit")
        self.Bind(wx.EVT_MENU, self.on_exit, m_exit)
                
        self.menubar.Append(menu_file, "&File")
        self.SetMenuBar(self.menubar)

    def create_main_panel(self):
        self.panel = wx.Panel(self)

        self.init_plot()
        self.canvas = FigCanvas(self.panel, -1, self.fig)
        self.canvasSlip = FigCanvas(self.panel, -1, self.figSlip)
        self.canvasFreq = FigCanvas(self.panel, -1, self.figFreq)

        self.xmin_control = BoundControlBox(self.panel, -1, "X min", 0)
        self.xmax_control = BoundControlBox(self.panel, -1, "X max", 50)
        self.ymin_control = BoundControlBox(self.panel, -1, "Y min", 0)
        self.ymax_control = BoundControlBox(self.panel, -1, "Y max", 100)
        
        self.pause_button = wx.Button(self.panel, -1, "Pause")
        self.Bind(wx.EVT_BUTTON, self.on_pause_button, self.pause_button)
        self.Bind(wx.EVT_UPDATE_UI, self.on_update_pause_button, self.pause_button)

        self.reset_button = wx.Button(self.panel, -1, "Reset")
        self.Bind(wx.EVT_BUTTON, self.on_reset_button, self.reset_button)
        
        self.speedLabel = wx.StaticText(self.panel, -1, "Speed:")
        self.speedText = wx.TextCtrl(self.panel, -1, "0", size=(100, -1))
        self.freqLabel = wx.StaticText(self.panel, -1, "Frequency:")
        self.freqText = wx.TextCtrl(self.panel, -1, "0", size=(100, -1))
        self.slipLabel = wx.StaticText(self.panel, -1, "Slip:")
        self.slipText = wx.TextCtrl(self.panel, -1, "100", size=(100, -1))
        
        self.sliderLabel = wx.StaticText(self.panel, -1, "Load", style=wx.ALIGN_CENTRE)
        self.slider = wx.Slider(self.panel, -1, 0, 0, 100, size=(200, -1), name="Load")
        
        self.Bind(wx.EVT_SLIDER, self.on_slider, self.slider)
        
        self.cb_grid = wx.CheckBox(self.panel, -1, 
            "Show Grid",
            style=wx.ALIGN_RIGHT)
        self.Bind(wx.EVT_CHECKBOX, self.on_cb_grid, self.cb_grid)
        self.cb_grid.SetValue(True)
        
        self.cb_xlab = wx.CheckBox(self.panel, -1, 
            "Show X labels",
            style=wx.ALIGN_RIGHT)
        self.Bind(wx.EVT_CHECKBOX, self.on_cb_xlab, self.cb_xlab)        
        self.cb_xlab.SetValue(True)
        
        self.hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox1.Add(self.pause_button, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
        self.hbox1.AddSpacer(20)
        self.hbox1.Add(self.cb_grid, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
        self.hbox1.AddSpacer(10)
        self.hbox1.Add(self.cb_xlab, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
        
        self.hbox2 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox2.Add(self.xmin_control, border=5, flag=wx.ALL | wx.EXPAND)
        self.hbox2.Add(self.xmax_control, border=5, flag=wx.ALL | wx.EXPAND)
        self.hbox2.AddSpacer(24)
        self.hbox2.Add(self.ymin_control, border=5, flag=wx.ALL | wx.EXPAND)
        self.hbox2.Add(self.ymax_control, border=5, flag=wx.ALL | wx.EXPAND)
        self.hbox2.Add(self.reset_button, border=5, flag=wx.ALL | wx.EXPAND)
        
        self.hbox3 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox3.Add(self.speedLabel, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox3.Add(self.speedText, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox3.Add(self.freqLabel, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox3.Add(self.freqText, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox3.Add(self.slipLabel, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox3.Add(self.slipText, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox3.Add(self.sliderLabel, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox3.Add(self.slider, border=5, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        
        self.hbox4 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox4.Add(self.canvasFreq, 1, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox4.Add(self.canvasSlip, 1, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        self.hbox4.Add(self.canvas, 1, flag=wx.LEFT | wx.TOP | wx.EXPAND)
        
        self.vbox = wx.BoxSizer(wx.VERTICAL)
        #~ self.vbox.Add(self.canvas, 1, flag=wx.LEFT | wx.TOP | wx.GROW)
        self.vbox.Add(self.hbox4, 0, flag=wx.ALIGN_LEFT | wx.TOP)

        self.vbox.Add(self.hbox3, flag=wx.ALIGN_LEFT | wx.EXPAND, border=5)        
        self.vbox.Add(self.hbox1, 0, flag=wx.ALIGN_LEFT | wx.TOP)
        self.vbox.Add(self.hbox2, 0, flag=wx.ALIGN_LEFT | wx.TOP)
        
        self.panel.SetSizer(self.vbox)
        self.vbox.Fit(self)
    
    def create_status_bar(self):
        self.statusbar = self.CreateStatusBar()

    def init_plot(self):
        self.dpi = 100
        self.fig = Figure((3.0, 3.0), dpi=self.dpi)

        self.axes = self.fig.add_subplot(111)
        self.axes.set_axis_bgcolor('black')
        self.axes.set_title('Load', size=12)
        
        pylab.setp(self.axes.get_xticklabels(), fontsize=8)
        pylab.setp(self.axes.get_yticklabels(), fontsize=8)

        # plot the data as a line series, and save the reference 
        # to the plotted line series
        #
        self.plot_data = self.axes.plot(
            self.data, 
            linewidth=1,
            color=(1, 1, 0),
            )[0]
            
        # Plot 2
        self.figFreq = Figure((3.0, 3.0), dpi=self.dpi)
        self.axesFreq = self.figFreq.add_subplot(111)
        self.axesFreq.set_axis_bgcolor('black')
        self.axesFreq.set_title('Frequency', size=12)
        
        pylab.setp(self.axesFreq.get_xticklabels(), fontsize=8)
        pylab.setp(self.axesFreq.get_yticklabels(), fontsize=8)

        # plot the data as a line series, and save the reference 
        # to the plotted line series
        #
        self.plot_dataFreq = self.axesFreq.plot(
            self.dataFreq, 
            linewidth=1,
            color=(1, 1, 0),
            )[0]
        
        # Plot 3
        self.figSlip = Figure((3.0, 3.0), dpi=self.dpi)
        self.axesSlip = self.figSlip.add_subplot(111)
        self.axesSlip.set_axis_bgcolor('black')
        self.axesSlip.set_title('Slip', size=12)
        
        pylab.setp(self.axesSlip.get_xticklabels(), fontsize=8)
        pylab.setp(self.axesSlip.get_yticklabels(), fontsize=8)

        # plot the data as a line series, and save the reference 
        # to the plotted line series
        #
        self.plot_dataSlip = self.axesSlip.plot(
            self.dataSlip, 
            linewidth=1,
            color=(1, 1, 0),
            )[0]
        
            

    def draw_plot(self):
        """ Redraws the plot
        """
        # when xmin is on auto, it "follows" xmax to produce a 
        # sliding window effect. therefore, xmin is assigned after
        # xmax.
        #
        if self.xmax_control.is_auto():
            xmax = len(self.dataLoad) if len(self.dataLoad) > 50 else 100
            xmaxFreq = len(self.dataFreq) if len(self.dataFreq) > 50 else 100
            xmaxSlip = len(self.dataSlip) if len(self.dataSlip) > 50 else 100
        else:
            xmax = int(self.xmax_control.manual_value())
            xmaxFreq = int(self.xmax_control.manual_value())
            xmaxSlip = int(self.xmax_control.manual_value())
            
        if self.xmin_control.is_auto():            
            xmin = xmax - 100
            xminFreq = xmaxFreq - 100
            xminSlip = xmaxSlip - 100
        else:
            xmin = int(self.xmin_control.manual_value())
            xminFreq = int(self.xmin_control.manual_value())
            xminSlip = int(self.xmin_control.manual_value())

        # for ymin and ymax, find the minimal and maximal values
        # in the data set and add a mininal margin.
        # 
        # note that it's easy to change this scheme to the 
        # minimal/maximal value in the current display, and not
        # the whole data set.
        # 
        if self.ymin_control.is_auto():
            ymin = round(min(self.dataLoad), 0) - 1
            yminFreq = round(min(self.dataFreq), 0) - 1
            yminSlip = round(min(self.dataSlip), 0) - 1
        else:
            ymin = int(self.ymin_control.manual_value())
            yminFreq = int(self.ymin_control.manual_value())
            yminSlip = int(self.ymin_control.manual_value())
        
        if self.ymax_control.is_auto():
            ymax = 100
            ymaxFreq = 100
            ymaxSlip = 100
        else:
            ymax = int(self.ymax_control.manual_value())
            ymaxFreq = int(self.ymax_control.manual_value())
            ymaxSlip = int(self.ymax_control.manual_value())

        self.axes.set_xbound(lower=xmin, upper=xmax)
        self.axes.set_ybound(lower=ymin, upper=ymax)
        
        self.axesFreq.set_xbound(lower=xminFreq, upper=xmaxFreq)
        self.axesFreq.set_ybound(lower=yminFreq, upper=ymaxFreq)
        
        self.axesSlip.set_xbound(lower=xminSlip, upper=xmaxSlip)
        self.axesSlip.set_ybound(lower=yminSlip, upper=ymaxSlip)
        
        # anecdote: axes.grid assumes b=True if any other flag is
        # given even if b is set to False.
        # so just passing the flag into the first statement won't
        # work.
        #
        if self.cb_grid.IsChecked():
            self.axes.grid(True, color='gray')
            self.axesFreq.grid(True, color='gray')
            self.axesSlip.grid(True, color='gray')
        else:
            self.axes.grid(False)
            self.axesFreq.grid(False)
            self.axesSlip.grid(False)

        # Using setp here is convenient, because get_xticklabels
        # returns a list over which one needs to explicitly 
        # iterate, and setp already handles this.
        #  
        pylab.setp(self.axes.get_xticklabels(), 
            visible=self.cb_xlab.IsChecked())
        
        self.plot_data.set_xdata(np.arange(len(self.dataLoad)))
        self.plot_data.set_ydata(np.array(self.dataLoad))
        
        self.canvas.draw()

        pylab.setp(self.axesFreq.get_xticklabels(), 
            visible=self.cb_xlab.IsChecked())
        self.plot_dataFreq.set_xdata(np.arange(len(self.dataFreq)))
        self.plot_dataFreq.set_ydata(np.array(self.dataFreq))
        
        self.canvasFreq.draw()
        
        pylab.setp(self.axesSlip.get_xticklabels(), 
            visible=self.cb_xlab.IsChecked())
        self.plot_dataSlip.set_xdata(np.arange(len(self.dataSlip)))
        self.plot_dataSlip.set_ydata(np.array(self.dataSlip))        
        
        self.canvasSlip.draw()
    
    def on_pause_button(self, event):
        self.paused = not self.paused

    def on_reset_button(self, event):
        self.data.append(0)
        self.dataFreq.append(0)
        self.dataSlip.append(100)        
        self.dataSpeed.append(0)
        self.freqText.SetValue(str(0))
        self.slipText.SetValue(str(100))
        self.speedText.SetValue(str(0))
        self.slider.SetValue(0)
        self.dataLoad.append(0)
        self.datagen.rpm = 0
        self.datagen.stable = True
        self.datagen.load = 0
        self.datagen.freq = 0
        self.datagen.slip = 100
        self.datagen.speed = 0
                        

    def on_slider(self, event):
        #~ self.dataLoad.append(self.slider.GetValue())
        self.datagen.next(self.slider.GetValue(), self.dataLoad[-1])
    
    def on_update_pause_button(self, event):
        label = "Resume" if self.paused else "Pause"
        self.pause_button.SetLabel(label)
    
    def on_cb_grid(self, event):
        self.draw_plot()
    
    def on_cb_xlab(self, event):
        self.draw_plot()
    
    def on_save_plot(self, event):
        file_choices = "PNG (*.png)|*.png"
        
        dlg = wx.FileDialog(
            self, 
            message="Save plot as...",
            defaultDir=os.getcwd(),
            defaultFile="plot.png",
            wildcard=file_choices,
            style=wx.SAVE)
        
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self.canvas.print_figure(path, dpi=self.dpi)
            self.flash_status_message("Saved to %s" % path)
    
    def on_redraw_timer(self, event):
        # if paused do not add data, but still redraw the plot
        # (to respond to scale modifications, grid change, etc.)
        #
        if not self.paused:
            #~ print ("Updating with " + str(self.slider.GetValue()) + " " + str(self.dataLoad[-1]))
            self.datagen.next(self.slider.GetValue(), self.dataLoad[-1])
            #~ print(str(self.datagen.load))
            self.dataLoad.append(self.datagen.load)
            self.dataFreq.append(self.datagen.freq)
            self.dataSlip.append(self.datagen.slip)
            self.dataSpeed.append(self.datagen.speed)
            
            self.freqText.SetValue(str(self.dataFreq[-1]))
            self.slipText.SetValue(str(self.dataSlip[-1]))
            self.speedText.SetValue(str(self.dataSpeed[-1]))
        
        self.draw_plot()
    
    def on_exit(self, event):
        self.Destroy()
    
    def flash_status_message(self, msg, flash_len_ms=1500):
        self.statusbar.SetStatusText(msg)
        self.timeroff = wx.Timer(self)
        self.Bind(
            wx.EVT_TIMER, 
            self.on_flash_status_off, 
            self.timeroff)
        self.timeroff.Start(flash_len_ms, oneShot=True)
    
    def on_flash_status_off(self, event):
        self.statusbar.SetStatusText('')


if __name__ == '__main__':
    app = wx.PySimpleApp()
    app.frame = GraphFrame()
    app.frame.Show()
    app.MainLoop()

