
#!/usr/bin/python
#use pygame mixer to play boot sounds
#talkey takes a long time to initialize, so do this now
import pygame
pygame.mixer.init(frequency=16000)
pygame.init()
pygame.mixer.music.load("startup.wav")
pygame.mixer.music.play()
print "Please wait while I start up. This might take up to a minute."

msg={
'camfail':"I'm having trouble connecting to the camera. Check the cable is connected, or try holding down the camera button for 5 seconds.",
'init':"Almost ready.",
'ready':"Hello. I'm ready to work now.",
'notext':"Sorry, I couldn't find the text."
}

import talkey
#setup the TTS engine and notifications - happens before everything else
tts=talkey.Talkey(engine_preference=["pico"])

# import the necessary packages
import cv2
import time
import RPi.GPIO as GPIO
import pytesseract
import numpy
from PIL import Image

def say(txt):
    tts.say(txt)
    print txt

say(msg['init'])



# activate the pullup on the GPIO pins
GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def getImage():
    # Try to open the camera, resolve if fails
    # returns an image from the camera
    cam = cv2.VideoCapture(1)
    cam.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH,2304)
    cam.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT,1536)
    while not cam.isOpened():
        say(msg['camfail'])
        cam = cv2.VideoCapture(0)
        cv2.waitKey(10000)
    ret_val, image = cam.read()
    return image

def look():
    #Variables for thresholding values for blobbing operations
    min_area = 100     # minimum size of contours to include
    max_area = 600    # maximum size of contours to include
    min_thresh = 130  # threshold value when using binary threshing
    dilations = 9     # number of times to apply dilation operations
    
    #This captures an image from the camera, and reads out any text
    pygame.mixer.init(48000)
    pygame.init()
    pygame.mixer.music.load("shutter.wav")

    # get the image and play a sound to let the user know what's happening
    image = getImage()
    pygame.mixer.music.play()    

    # convert image to grey and blob it
    gray = cv2.cvtColor(image,cv2.COLOR_BGR2GRAY) # grayscale
    #_,thresh = cv2.threshold(gray,min_thresh, 255,cv2.THRESH_BINARY_INV) # threshold
    thresh = cv2.adaptiveThreshold(gray, 255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY_INV,41,20)# threshold
    kernel = cv2.getStructuringElement(cv2.MORPH_CROSS,(3,3))
    dilated = cv2.dilate(thresh,kernel,iterations = dilations) # dilate

    #get the contours (blocks of text)
    contours, hierarchy = cv2.findContours(dilated,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_NONE) # get contours

    #check something was found
    if len(contours)==0:
        say(msg["notext"])
        
    # split each contour into a separate image
    paralist=[] # image list containing each contour area
    deskewed=[] # image list containing contour areas corrected for rotation
    for contour in contours:
        # get rectangle bounding contour
        [x,y,w,h] = cv2.boundingRect(contour)
        # discard areas that are too big or small
        if h < min_area or w < min_area:
            continue
        if h > max_area and w > max_area:
            continue
        paralist.append(image[y:y+h, x:x+w])

    # straighten images to remove skew/rotation
    paralist.reverse()
    for para in paralist:
        # convert the image to binary
        gray = cv2.cvtColor(para, cv2.COLOR_BGR2GRAY)
        gray = cv2.bitwise_not(gray)
        #gray = cv2.equalizeHist(gray)
        clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
        cli = clahe.apply(gray)
        gray=cli
        #_,gray = cv2.threshold(gray, 210, 255, cv2.THRESH_BINARY)
        #DEBUG
        img = cv2.cvtColor(gray,cv2.COLOR_GRAY2RGB)
        pil_im = Image.fromarray(img)
        pil_im.show()

        thresh = cv2.threshold(gray, 0,255,cv2.THRESH_BINARY | cv2.THRESH_OTSU)[1]

        # clip to min area rectangle, and find rotation angle
        coords = numpy.column_stack(numpy.where(thresh>0))
        angle = cv2.minAreaRect(coords)[-1]
        
        # make the angle positive, accounting for the range of the minareaRect function
        if angle < -45:
            angle = -(90+angle)
        else:
            angle = -angle
        print angle

        # do the rotation
        (h,w) = para.shape[:2]
        center = (w//2,h//2)
        M = cv2.getRotationMatrix2D(center, angle, 1.0)
        rotated = cv2.warpAffine(para, M, (w,h), flags=cv2.INTER_CUBIC, borderMode=cv2.BORDER_REPLICATE)
        deskewed.append(rotated)

    # say it
    for para in deskewed:
        parab = cv2.resize( para, None, fx=4,fy=4,interpolation = cv2.INTER_CUBIC)
        img = cv2.cvtColor(parab,cv2.COLOR_BGR2RGB)
        pil_im = Image.fromarray(img)
        say(pytesseract.image_to_string(pil_im))


def main():
    cycle_init=time.time() # this is for the button-press debounce
    ret_val = getImage() #check the camera is working as soon as we start
    say(msg['ready'])
    
    while True:
        btn = GPIO.input(21)
        #if the button is pressed and not same press as last time
        if btn == False and time.time()-cycle_init > 1:
            look()
            cycle_init=time.time()


if __name__ == "__main__":
    main()
    
