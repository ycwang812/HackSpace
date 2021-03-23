#!/usr/bin/env python

import sys
import time
import pigpio
import fractions
import os


#pins
clk_pin=18 # clock signal from caliper
dat_pin=17 # dat signal from caliper
metric_pin = 27 # metric or imperial
speak_pin = 3 # pushbutton to make pi speak. Hold down to shut down pi

#clock and data variables
last = None # last microsecond stamp for clock state change
count = 0 # count pulses - 24 per frame, 2 frames per cycle
cb = [] # callback queue
frames=[0]*48 # caliper dat bit buffer
dat_level = 0 # current value of dat line on calipers

#output variables
metric = True
mm = 0

def sayFractions(ftn):
    #makes fractions sound right
    fraction_words = ["0","1","half","third","quarter","fifth"]
    if "." in str(ftn):
        whol,frac = str(ftn).split(".")
    else:
        whol=str(ftn)
        frac="0"
        
    numer=fractions.Fraction("."+frac).numerator
    denom=fractions.Fraction("."+frac).denominator
    if numer==0:
        output = str(whol)+" inches"
        return output
    else:
        output = str(whol)+" and "+str(numer)+" "
    if  fractions.Fraction("."+frac).denominator < 6:
        output += fraction_words[denom]
    else:
        output += str(denom)+"ths"
    output += " of an inch"
    return output

def translate(value, leftMin, leftMax, rightMin, rightMax):
    # Figure out how 'wide' each range is
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin

    # Convert the left range into a 0-1 range (float)
    valueScaled = float(value - leftMin) / float(leftSpan)

    # Convert the 0-1 range into a value in the right range.
    return rightMin + (valueScaled * rightSpan)

def clk_cbf(GPIO, level, tick):
    # follows the clock and syncs the frames
    global last
    global count
    global frames
    global dat_level
    global mm
    
    if last is not None:
      diff = pigpio.tickDiff(last, tick)
      # if calipers report a new frame or the frame has been reset
      if diff > 300000 or count >= 48:
          temp="".join(str(x) for x in frames[4:23])[::-1]
          temp2="".join(str(x) for x in frames[27:47])[::-1]
          
          if frames[47]==1:
              #negative number
              temp2="".join(str(int(not x)) for x in frames[27:49])[::-1]
              mm=0-round(translate(int(temp2,2),0,15511,0,153.9),2)
          else:
              #positive number
              mm=round(translate(int(temp2,2),0,15511,0,153.9),2)
          count = 0
          frames=[0]*48

      else:
          frames[count]=dat_level
          count +=1
    last = tick

def dat_cbf(GPIO, level, tick):
    #get the dat pin readings and stick them in dat_level for clk_cbf to read
    global dat_level
    dat_level = int(level)

def speak_cbf(GPIO, level, tick):
    print("SPEAK PRESSED")
    if metric==True:
        print(str(mm)+"mm")
        os.system("echo '"+str(mm)+"millimeters'|festival --tts")
    else:
        print(str(sayFractions(round(mm/25.4,2))))

def metric_cbf(GPIO, level, tick):
    global metric
    print("METRIC PRESSED")
    if metric == False:
        metric = True
    else:
        metric = False
    
    
#setup piGPIO for near-realtime gpio monitoring
pi = pigpio.pi()

if not pi.connected:
    exit()

# set the pin modes for the buttons and signal pins
pi.set_mode(clk_pin, pigpio.INPUT)
pi.set_mode(dat_pin, pigpio.INPUT)
pi.set_mode(metric_pin, pigpio.INPUT)
pi.set_mode(speak_pin, pigpio.INPUT)
pi.set_pull_up_down(metric_pin, pigpio.PUD_UP)
pi.set_pull_up_down(speak_pin, pigpio.PUD_UP)
pi.set_glitch_filter(metric_pin, 200)
pi.set_glitch_filter(speak_pin, 200)

# add callbacks for the buttons and signal pins
cb.append(pi.callback(dat_pin, pigpio.EITHER_EDGE, dat_cbf))
cb.append(pi.callback(clk_pin, pigpio.FALLING_EDGE, clk_cbf))
cb.append(pi.callback(speak_pin, pigpio.RISING_EDGE, speak_cbf))
cb.append(pi.callback(metric_pin, pigpio.RISING_EDGE, metric_cbf))

try:
    while True:
        time.sleep(60)

        
except KeyboardInterrupt:
    print("\nTidying up")
    for c in cb:
      c.cancel()

pi.stop()

