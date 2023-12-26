// Thin = 11
// med = A0
// thick = 10


#include <Adafruit_NeoPixel.h>
#define LED_PIN 2
#define LED_COUNT 50
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

#define LOW_THRESHOLD 30
#define STEP_SIZE 2
#define DELAY_TIME 25

/* DIGITAL IO */
#define MSGEQ7_RESET 4
#define MSGEQ7_STROBE 6
#define MINI_DEBUG_LED 13

/* ANALOG IO */
#define MSGEQ7_ANAOUT A0
#define MSGEQ7_SETTLE_TIME 50

/* CONSTANTS */
#define DEBUG_BLINK_RATE 2
#define NUM_BANDS 7

/* Frequency Bands */
#define FREQ0_63_HZ 0
#define FREQ1_160_HZ 1
#define FREQ2_400_HZ 2
#define FREQ3_1000_HZ 3
#define FREQ4_2500_HZ 4
#define FREQ5_6250_HZ 5
#define FREQ6_16000_HZ 6

/* MSGEQ7 Frequency Bands 63, 160, 400, 1000, 2500,6250, 16000 */
/* There are 7 freq bands.  So, combine bands where desired as needed if you only have 4 colors */
double freqArr[NUM_BANDS];
double freqArrOld[NUM_BANDS];
  
int arrIndex;
int debugCount;
int opMode;
int oldOpMode;
int opModeBlinkCnt;
int eqPulseWidth;
uint32_t counter = 0;
bool on = true;


void setup() {
  strip.begin();
  strip.setBrightness(255);
  strip.clear();

  pinMode(MSGEQ7_RESET, OUTPUT );
  pinMode(MSGEQ7_STROBE, OUTPUT);
  pinMode(MINI_DEBUG_LED, OUTPUT );

  randomSeed(analogRead(0));
  
  eqPulseWidth = 150;
  opModeBlinkCnt = DEBUG_BLINK_RATE;
    
  Serial.begin(115200);
  delay(10);
  Serial.flush();  
  Serial.println("Music Visualizer Setup");
  Serial.print("In Mode ");  Serial.println(opMode);
}
 
void loop() {
    initializefreqArr();
    guitarHeroLights();
    delay(1);
    strip.show();
}

/*==============================================================initializeFreqArray

 * fills the freq array with 7 values between 0 and 254.
 * These values are raw input values mapped directly to 0 - 254.
 * The values are stored in the freqArr array.
 */ 
void initializefreqArr(void) {
   // Reset the MSGEQ7 chip to start the read sequence   
  digitalWrite (MSGEQ7_RESET, HIGH);   // reset the MSGEQ7's counter
  delayMicroseconds (120);
  digitalWrite (MSGEQ7_RESET, LOW);
  delayMicroseconds (120);
  digitalWrite (MSGEQ7_STROBE, HIGH);  
  delayMicroseconds (eqPulseWidth);    
   for (arrIndex = 0; arrIndex < NUM_BANDS; arrIndex++) {
     digitalWrite (MSGEQ7_STROBE, LOW);     // output each DC value for each freq band
     delayMicroseconds (MSGEQ7_SETTLE_TIME); // to allow the output to settle
     freqArr[arrIndex] = map(analogRead (MSGEQ7_ANAOUT),0,1023,0,254); //Read in the analog input
     delayMicroseconds (eqPulseWidth - MSGEQ7_SETTLE_TIME); // to allow the output to settle
     digitalWrite (MSGEQ7_STROBE, HIGH);
     delayMicroseconds (eqPulseWidth);
  }
}


/*==============================================================calibrateFreqArr
 * 
 * That raw data in freqArr actually looks pretty rough on the leds.
 * So here we have a function that looks at how big the difference is between its current freq
 * number and the freq number that is being read in. It then takes a step in that direction.
 * The size of the step is determined by the stepSize varible. 
 * How fast these steps happen is determined by the delayTime variable.
 * You can also set a threshold for how loud a noise needs to be in order for it to be reconized.
 */

void calibrateFreqArr() {
  if (counter % DELAY_TIME == 0) { // Wait untill the delay time before taking a step.
    uint32_t colArr[7];
    for (int i = 0; i < NUM_BANDS; i++) {
      if (freqArr[i] < LOW_THRESHOLD) { // Noise isn't loud enough to be over the threshold so ignor it. 
        freqArr[i] = LOW_THRESHOLD; 
      }
      // Take the difference between the old and new freq, and divide by the step size.
      // Then take that value and ad or subtract it from the old value. 
      freqArrOld[i] += (freqArr[i] - freqArrOld[i])/STEP_SIZE;
    }
  }
}



/*==============================================================guitareHero
 * 
 * Takes calibrated freq numbers and sends them zipping down the strip starting at 0.
 * This one has two seperate delays for maximum confusion of my future self.
 * The first delay is just the one in the calibrateFreqArr function.
 * The second delay tells the function how long to wait before moving the light value to the next spot in line.
 * This is controlled by the yesItNeedsASecondDelay variable.
 */
 
void guitarHeroLights() {
  uint8_t yesItNeedsASecondDelay = 5;
  counter++;
  calibrateFreqArr();
  if (counter % yesItNeedsASecondDelay == 0) {
    uint32_t ledArr[LED_COUNT]; // combine the frequencies in pairs and use the results as rgb values.
    uint8_t colorArr[3];
    for (int i = 0; i < 3; ++i) {
     int freqOne = freqArrOld[i * 2];
     int freqTwo = freqArrOld[i * 2 + 1]; 
     freqOne <= LOW_THRESHOLD ? freqOne = 0 : freqOne = freqOne; // Display nothing if the frequency is below threshold
     freqTwo <= LOW_THRESHOLD ? freqTwo = 0 : freqTwo = freqTwo;
     colorArr[i] = (freqOne + freqTwo) / 2;
    }
    uint32_t note = strip.Color(colorArr[0], colorArr[1], colorArr[2]);
    ledArr[0] = note;
    for (int i = LED_COUNT; i > 0; --i) { // Shift all the colors over one pixel toward the end of the strip.
      ledArr[i] = ledArr[i - 1];
      strip.setPixelColor(i, ledArr[i]);
    }
    strip.setPixelColor(0, ledArr[0]); // Set pixel 0 the new color.
  }
}


