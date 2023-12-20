#include <Adafruit_NeoPixel.h>

/* DIGITAL IO */
#define MSGEQ7_RESET 4
#define MSGEQ7_STROBE 6
#define MINI_DEBUG_LED 13
#define LED_PIN 2
#define LED_NUM 50
#define LOW_THRESHOLD 20
#define DELAY_TIME 30
#define STEP_DENOMINATOR 15

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
int freqArray[NUM_BANDS];
int currentFreqArray[NUM_BANDS];
Adafruit_NeoPixel lights = Adafruit_NeoPixel(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);
  
int arrIndex;
int debugCount;
int opMode;
int oldOpMode;
int opModeBlinkCnt;
int eqPulseWidth;
int currentRed;
int currentGreen;
int currentBlue;
int timeStamp;

void setup() {
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:

  pinMode(MSGEQ7_RESET, OUTPUT );
  pinMode(MSGEQ7_STROBE, OUTPUT);
  pinMode(MINI_DEBUG_LED, OUTPUT );
  
  opMode = 1;
  oldOpMode = 0;
  eqPulseWidth = 150;
  opModeBlinkCnt = DEBUG_BLINK_RATE;

  lights.begin();
  lights.setBrightness(255);
  lights.clear();
    
  Serial.begin(115200);
  delay(10);
  Serial.flush();  
  Serial.println("Music Visualizer Setup");
  Serial.print("In Mode ");  Serial.println(opMode);
}
 
void loop() {

  //Serial.flush();  
  int inputVal;
  int cnt;

  debugCount ++;
  //flash the debug LED to let us know it is alive
  if (debugCount == opModeBlinkCnt)
  {
    digitalWrite(MINI_DEBUG_LED, HIGH); 
  }    
  if (debugCount >= (opModeBlinkCnt * 2))
  {
    digitalWrite(MINI_DEBUG_LED, LOW);
    debugCount = 0;
  }   

  // Reset the MSGEQ7 chip to start the read sequence   
  digitalWrite (MSGEQ7_RESET, HIGH);   // reset the MSGEQ7's counter
  delayMicroseconds (120);
  digitalWrite (MSGEQ7_RESET, LOW);
  delayMicroseconds (120);
  digitalWrite (MSGEQ7_STROBE, HIGH);  
  delayMicroseconds (eqPulseWidth);
      
  for (arrIndex = 0; arrIndex < NUM_BANDS; arrIndex++)
  {
    digitalWrite (MSGEQ7_STROBE, LOW);     // output each DC value for each freq band
    delayMicroseconds (MSGEQ7_SETTLE_TIME); // to allow the output to settle
    freqArray[arrIndex] = analogRead (MSGEQ7_ANAOUT); //Read in the analog input
    
    delayMicroseconds (eqPulseWidth - MSGEQ7_SETTLE_TIME); // to allow the output to settle
    digitalWrite (MSGEQ7_STROBE, HIGH);
    delayMicroseconds (eqPulseWidth);
  }
  //Add a little exta delay if resetting chip each time
  //This is kind if pointless because of the delay below. But when you start doing real lights 
  //and going faster, you will need it after the read sequence above
  delayMicroseconds(1500);
  
  if (timeStamp - millis() > DELAY_TIME) {
    timeStamp = millis();
    for (int i = 0; i < NUM_BANDS; ++i) {
      int diff = freqArray[i] - currentFreqArray[i];
      currentFreqArray[i] += diff/STEP_DENOMINATOR;
    }
  }

  int colorArr[3];
  for (int i = 0; i < 3; ++i) {
    colorArr[i] = currentFreqArray[i*2+1] + currentFreqArray[i*2+2];
    colorArr[i] = map(colorArr[i], 0, 2048, 0, 255);
    if (colorArr[i] < LOW_THRESHOLD) { colorArr[i] = LOW_THRESHOLD; }
    Serial.print(colorArr[i]);
    Serial.print(", ");
  }
  Serial.println("");
  
  uint32_t color = lights.Color(colorArr[2], colorArr[1], colorArr[0]);
  lights.clear();
  lights.fill(color, 0, LED_NUM);
  lights.show();
  delay(1);
}
