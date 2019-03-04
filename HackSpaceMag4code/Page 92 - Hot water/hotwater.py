#!/usr/bin/python

#PART ONE – IMPORT LIBRARIES

import datetime
import os
import pygsheets
from blinkt import set_pixel, set_brightness, show, clear, set_clear_on_exit

#PART TWO – DEFINING KEY VARIABLES

# Setting operating parameters

temp_min = 35.0 # the temp below which all LEDs will be 100% blue
temp_max = 40.0 # the temp above which all LEDs will be 100% red
temp_thresh = ((temp_max - temp_min)/2) + temp_min # half way between these temps will be the threshold for calculating percentage remaining

# define sensor addresses
sensor1 = '28-031683f981ff'
sensor2 = '28-0416845ca7ff'
sensor3 = '28-0416a4e09fff'
sensor4 = '28-0516a50d47ff'
sensor5 = '28-0316a48e3fff'
sensor6 = '28-0416a4e19cff'
sensor7 = '28-0416a4e5b2ff'
sensor8 = '28-0416a4dea9ff'

# list of sensors to go into the FOR loop
sen = [sensor1,sensor2,sensor3,sensor4,sensor5,sensor6,sensor7,sensor8]

# conversion factor to convert temp to LED colours
factor = (temp_max - temp_min) / 256

# Set LED parameters
set_brightness(0.05)
set_clear_on_exit(False)

#PART THREE – OPENING THE SENSOR DRIVERS

# opening sensor drivers
os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')

#PART FOUR – A SUBROUTINE TO UPLOAD THE DATA TO GOOGLE

def update_spreadsheet():  # Subroutine for google spreadsheet update
# Open Hot Water google spreadsheet
# If client_secret.json is stored elsewhere, the folder must be changes in the line below
    gs = pygsheets.authorize(service_file='/home/pi/python/client_secret.json')
# If the spreadsheet is not called "Hot Water", change the name in the line below
    sh = gs.open('Hot Water')
    wks = sh.sheet1
    print "Contacting Google..."

# Add new row at the top and delete the last row
    wks.insert_rows(16,1)
    wks.delete_rows(304,1)

# Insert data
    wks.update_cell("A2",str(hw_left) + "%")
    wks.update_cell("A3",the_time)
    wks.update_cell("A17",the_time)
    wks.update_cell("B17",temp_out[0])
    wks.update_cell("C17",temp_out[1])
    wks.update_cell("D17",temp_out[2])
    wks.update_cell("E17",temp_out[3])
    wks.update_cell("F17",temp_out[4])
    wks.update_cell("G17",temp_out[5])
    wks.update_cell("H17",temp_out[6])
    wks.update_cell("I17",temp_out[7])
    wks.update_cell("J17",hw_left)
    print "Data added"

#PART FIVE – A LOOP THAT RUNS 8 TIMES – ONCE PER SENSOR

# These next 2 varables are used within the FOR loop to capture data
hw_left = 0.0 # starts a count of the number of sensor with hot water
temp_out = [] # creates an new empty list to add the temperature data into


for x in range (0,8):  # run the same code 8 times, with new sensor address each time

# open data file and read all data
    sensor_file = '/sys/bus/w1/devices/' + sen[x] + '/w1_slave'
    f = open(sensor_file, 'r')
    sensor_output = f.readlines()
    f.close()

# parse data file and extract the temperature from the last 5 digits
    if sensor_output[0].strip()[-3:] == 'YES' : # a valid data file will end in YES on the first line
	equals = sensor_output[1].find('=') # Looks for the equals sign, and returns just the values afterwards
	temp_output = sensor_output[1].strip() [equals +1:]
    else:
	temp_output =  0.0 # if the file has no data, zeros are added to avoid crashing the later code

# format temp reading to 1 decimal place
    temp = float(temp_output) / 1000.0
    temp = round(temp,1)
    temp_out.append(temp)

# if the sensor is above threshold temp, then increment the hw_left counter by 1
    if temp > temp_thresh:
        hw_left = hw_left+1

# set pixel colour based on temp
    colour = (temp - temp_min) / factor
    colour = int(round(colour,0))

    red = colour
    if red > 255:
        red = 255

    if red < 0:
        red = 0
    blue = 255 - red
    pix = 7 - x # invert the order in which LEDs are lit as the Blinkt is upside down
    set_pixel(pix, red, 0 , blue)
    show()

# screen dump output of each loop for troubleshooting
    print "Sensor:", x, "Address:", sen[x], "Temp:", str(temp),"RGB:", red , "0", blue

#PART SIX – FORMATTING THE DATA INTO A TIDY STATE

# convert the count of sensors above the threshold into a percentage
hw_left = hw_left / 8
hw_left = hw_left * 100
hw_left = int(round(hw_left,0))

# Format the time to be spreadsheet friendly - remove microseconds
the_time = datetime.datetime.now().replace (microsecond=0).isoformat(' ')

#PART SEVEN – CALLING THE DATA UPLOAD SUBROUTINE

# transfer data to google
try:
    update_spreadsheet()
except Exception:
    print "Cannot connect to google"
    pass