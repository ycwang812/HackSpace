# Gemma IO Cap Touch Color Change
# Connect NeoPixels to D1
# Connect Conductive fabric/metal to A2
# Colors will switch (red, green, blue) when conductive material is touched.

import touchio
import board
import neopixel
import time


touch1 = touchio.TouchIn(board.A2)
pixpin = board.D1
numpix = 5
strip = neopixel.NeoPixel(pixpin, numpix, brightness=0.1, auto_write=False)
color = 1

while True:
    
    if touch1.value:
        print(color)
        color = color + 1
        time.sleep(.25)

    if color == 1:
        print('red')
        strip.fill((255, 0, 0))
        strip.write()
        #time.sleep(1)

    if color == 2:
        print('green')
        strip.fill((0, 255, 0))
        strip.write()
        #time.sleep(1)

    if color == 3:
        print('blue')
        strip.fill((0, 0, 255))
        strip.write()
        #time.sleep(1)
    

    
    if color > 3:
        color = 1