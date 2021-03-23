#include <AceTime.h>
#include <Adafruit_DotStar.h>
#include <SPI.h>  


//dotstar related
#define NUMPIXELS 120 // Number of LEDs in strip
#define DATAPIN    14 // Node pin D5
#define CLOCKPIN   12 // Node pin D6
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
int pixelHour; // Time Hours converted to LED index
int pixelMinute; // Time Minutes converted to LED index

// clock related
using namespace ace_time;
using namespace ace_time::clock;

static const char SSID[] = "YOURWIFI";
static const char PASSWORD[] = "YOURPASS";

static BasicZoneProcessor londonProcessor;
static NtpClock ntpClock;

acetime_t nowSeconds;
auto localTz = TimeZone::forZoneInfo(&zonedb::kZoneEurope_London, &londonProcessor);


unsigned long last_tick = 0;
unsigned long frequency = 10000; // loop every 10 seconds

void setup() {
  Serial.begin(115200);
  // start the ntp comms
  ntpClock.setup(SSID, PASSWORD);
  if (!ntpClock.isSetup()) {
    Serial.println("No Wifi detected.");
    return;
  }

  // fire up the strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void loop() {
  // skip the loop?
  if (millis() < last_tick+frequency){ 
    return;
  }
  last_tick = millis();
  
  // get the time
  nowSeconds = ntpClock.getNow();
  auto tick = ZonedDateTime::forEpochSeconds(nowSeconds, localTz);

  // skip if an ntp error happens
  if (tick.hour() == 255){
    return; 
  }
  
  // convert to LED indexes. 
  // First half of strip (0-60) is hours
  // second half is minutes
  
  if (tick.hour()<=12){
    pixelHour = map(tick.hour(),1,12,0,57);
  } else {
    pixelHour = map(tick.hour()-12,1,12,0,57);
    }
  pixelMinute = map(tick.minute(),0,59,120,63);

  Serial.print("Act: ");
  Serial.print(tick.hour());
  Serial.print(":");
  Serial.println(tick.minute());
  
  Serial.print("Mod: ");
  Serial.print(pixelHour);
  Serial.print(":");
  Serial.println(pixelMinute);
  Serial.println("============");


  strip.clear();
  // set hour
  strip.setPixelColor(pixelHour,255,20,0);
  strip.setPixelColor(pixelHour+1,255,255,150);
  strip.setPixelColor(pixelHour+2,255,20,0);
  // set minutes
  strip.setPixelColor(pixelMinute,20,250,0);
  strip.setPixelColor(pixelMinute-1,255,220,150);
  strip.setPixelColor(pixelMinute-2,20,250,0);
  strip.show();
  
  }
