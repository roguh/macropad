//MacroPad Demo Code 
//SeanStone86@gmail.com, Chris Rivera, Hugo Rivera
//Version: DEV
//---------------------------------------------------------
/*

NeoPixel(LEDs) Guide: https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use

Binary/Decimal/Hex Converter: https://www.mathsisfun.com/binary-decimal-hexadecimal-converter.html



//-------------------------------------------------------

        $$\            $$$$$$\  $$\   $$\ 
        \__|          $$  __$$\ $$ | $$  |
        $$\  $$$$$$\  $$ /  $$ |$$ |$$  / 
        $$ |$$  __$$\ \$$$$$$$ |$$$$$  /  
        $$ |$$ /  $$ | \____$$ |$$  $$<   
        $$ |$$ |  $$ |$$\   $$ |$$ |\$$\  
        $$ |\$$$$$$  |\$$$$$$  |$$ | \$$\ 
        \__| \______/  \______/ \__|  \__|

*/
//-----------[ SETTINGS ]---------------------------------- 
#define NUM_LEDS            12  //12 is the number of LEDs. You can increase or decrease but there are 12 LEDs on this board. 
#define LED_BRIGHT          150 // 0-255 0 being off, 255 max brightness 
#define OLED_REFRESH_RATE   10  // in milli Seconds
#define INPUT_REFRESH_RATE  5  // in milli Seconds

#define KB_ENABLE           1 //0 = disable 1 = enable | Enables keyboard output. Super annoying when programming.
#define SPK_ENABLE          0 //0 = disable 1 = enable | Enables Speaker output. 
//-----------[ PINOUT ]------------------------------------

//---- [PINS FOR ROTARY ENCODER] ----
#define ROT_A_PIN      17
#define ROT_B_PIN      18
#define ROT_BUTT_PIN   0

//---- [PINS FOR OLED SCREEN] -------
#define OLED_CS_PIN    22
#define OLED_RST_PIN   23
#define OLED_DC_PIN    24
#define OLED_SCK_PIN   26
#define OLED_MOSI_PIN  27
#define OLED_MISO_PIN  28

//---- [PINS FOR SPEAKER] ----------
#define SPK_PIN        16
#define SPK_SHDWN_PIN  14

//---- [PINS FOR WS2812B LEDs] -----
#define NEO_PXL_PIN    19   
#define LED_PIN        13 

//-----------[ LIBRARIES ]---------------------------------
#include <Adafruit_SH110X.h> //OLED Screen 
#include <RotaryEncoder.h>  //duh 
#include <Adafruit_NeoPixel.h> //WS2812B Leds *Hate this library but its the only one that works with the RaspberryPi. 
#include <Keyboard.h>
#include <EEPROM.h>


const char KERBAL_MSG[] = "KSP takeoff";

//-----------[ VARIABLES ]--------------------------------- 

//------ [ Function Variables ] ------------
uint8_t encoder_pos = 0;
uint8_t newPos = 0;

uint8_t swiSpc = 0; 
uint8_t gHue,xHue = 0; 
uint16_t gHue16,xHue16 = 0; 
uint8_t flagA = 0; 
bool keyModeEN,cirModeEN,hatModeEN,kerbalMode = 0; 
uint8_t kerbalIndex = 0;

float angleRad = 0;
uint16_t sinResult, sinResultMap, angleDeg = 0; 
int16_t cosResult = 0;    

//----- [Time Delay] ------- 
uint16_t currentMillis[9];  
uint16_t prevMillis[9]; 
//--------------------------------

//-----------[ INIT ]--------------------------------------                             
Adafruit_SH1106G display = Adafruit_SH1106G(128,64,OLED_MOSI_PIN,OLED_SCK_PIN,OLED_DC_PIN,OLED_RST_PIN,OLED_CS_PIN);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS,NEO_PXL_PIN,NEO_GRB+NEO_KHZ800); //12 is the number of LEDs. You can increase or decrease but there are 12 LEDs on this board. 
RotaryEncoder encoder(ROT_A_PIN,ROT_B_PIN,RotaryEncoder::LatchMode::FOUR3);

//-----------[ SETUP ]------------------------------------- 
void setup()
{
  //----- [Keyboard Setup]-----------
  Keyboard.begin();
  
  //----- [Speaker Setup] -------
  pinMode(SPK_PIN,OUTPUT);
  digitalWrite(SPK_PIN,LOW);
  pinMode(SPK_SHDWN_PIN,OUTPUT);
  digitalWrite(SPK_SHDWN_PIN,HIGH);
  SoundOut(9,100,0,0); //frequency,duration,repetitions,delay (
 
  //----- [Starts LEDs] -------
  pixels.begin();
  pixels.setBrightness(LED_BRIGHT); 
  for (uint8_t i=0;i<NUM_LEDS;i++) pixels.setPixelColor(i,0x000000); // Initialize all pixels to 'off'
  pixels.show(); 
  
  //----- [Input Setup] -------
  for (uint8_t i=0;i<=12;i++) pinMode(i,INPUT_PULLUP); //Sets all mechanical keys to inputs
  
  encPosition(); //Checks the position the Rotary Knob 
  pinMode(ROT_A_PIN,INPUT_PULLUP); //This stuff is for the rotary knob. 
  pinMode(ROT_B_PIN,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ROT_A_PIN),encPosition,CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROT_B_PIN),encPosition,CHANGE);  
  
  //----- [OLED Screen Setup] -------
  display.begin(0,true); 
  delay(50); //The RaspberryPi processor is very fast so delays like this allows for the OLED processor to catch up. 
  OLED_Menu(0,1,0); //Menu Select, Text Size, Text Wrap  
  delay(500); //Splash Screen Delay 
  OLED_Menu(1,1,0); //Menu 1
}
//------------[ MAIN ]-------------------------------------- 
void loop() 
{
//----- [Output Stuff] -------  
  OLED_Dec(millis(),80,10);
  OLED_Text("   ",30,50); 
  OLED_Dec(xHue,30,50); 

  xHue = SinGen16(20,75,250); //Rate, toLow, toHigh
  OLED_Text("   ",60,50); 
  OLED_Dec(sinResult,60,50); 

  OLED_Text("      ",30,38); 
  OLED_Dec(xHue16,30,38); 
 
//----- [Input Stuff] -------
  if (encUpdate() == true) 
  {
    //pixelPond(); 
    OLED_Text("      ",30,10); //Clears the workspace so numbers don't overlap 
    OLED_Dec(gHue16,30,10); 
  }
  
  if (delay_milli(INPUT_REFRESH_RATE,7) == 7)
  {
    for (swiSpc=0;swiSpc<=12;swiSpc++) //Checks the input status of all switches including the rotary encoder switch (not the rotation, the button on the knob) 
    {
      if ((!digitalRead(swiSpc)) && (flagA == 0))
      {
        flagA = 1; 
        //Clears the workspace so key names don't overlap 
        OLED_Text("             ", 25, 24);
        switch (swiSpc) 
        {
          case 1:SoundOut(100,50,1,1); 
                 OLED_Text("HATE",25,24);
                 if (hatModeEN == 1) pixels.setPixelColor(0,0xffffff); 
                 else pixels.setPixelColor(0,0xff0000); 
                // ASCII_Output(0x31);   //Outputs via USB ASCII 
                 hateWords();
                 hatModeEN = !hatModeEN; 
          break; 
                 
          case 2:SoundOut(200,50,1,1);  
                 OLED_Text("x",50,32);
                 if (cirModeEN == 1) pixels.setPixelColor(1,0xffff00); 
                 else pixels.setPixelColor(1,0x00ffff); 
                 ASCII_Output(0x32); 
                 cirModeEN = !cirModeEN;   
          break; 
                 
       case 3:SoundOut(300,50,1,1);   
                 OLED_Text("3",25,24); 
                 if (keyModeEN == 1) pixels.setPixelColor(2,0xff0000); 
                 else pixels.setPixelColor(2,0x0000ff); 
                 ASCII_Output(0x33);
                 keyModeEN = !keyModeEN;  
                 if (keyModeEN == false) {
                  for (uint8_t i=3;i<NUM_LEDS;i++) pixels.setPixelColor(i,0); 
                 }
          break; 
                 
          case 4:SoundOut(400,50,1,1);   
                 OLED_Text(KERBAL_MSG,25,24);
                 pixels.setPixelColor(3,0xff0000);
                 kerbal();
                 kerbalMode = true;
          break; 
                 
          case 5:SoundOut(500,50,1,1);
                 OLED_Text("5",25,24); 
                 pixels.setPixelColor(4,0xff0000);
                 ASCII_Output(0x35);
          break; 
                 
          case 6:SoundOut(600,50,1,1);  
                 OLED_Text("6",25,24);
                 pixels.setPixelColor(5,0xff0000);  
                 ASCII_Output(0x36);  
          break; 
                  
          case 7:SoundOut(700,50,1,1);   
                 OLED_Text("7",25,24);
                 pixels.setPixelColor(6,0xff0000); 
                 ASCII_Output(0x37);   
          break; 
                  
          case 8:SoundOut(800,50,1,1);
                 OLED_Text("8",25,24);
                 pixels.setPixelColor(7,0xff0000);
                 ASCII_Output(0x38);    
          break; 
                 
          case 9:SoundOut(900,50,1,1);   
                 OLED_Text("closing vim",25,24);
                 pixels.setPixelColor(8,0x00ff00);  
                 closeVim();
          break;
                  
          case 10:SoundOut(1000,50,1,1);   
                  OLED_Text("10",25,24);
                  pixels.setPixelColor(9,0x00ffff); 
                  fibonacciPython();
          break; 
                 
          case 11:SoundOut(1300,50,1,1);  
                  OLED_Text("11",25,24);
                  pixels.setPixelColor(10,0xff0000);
                  ASCII_Output(14);     
          break; 
                 
          case 12:SoundOut(1500,50,1,1);
                  OLED_Text("12",25,24);
                  if (cirModeEN == 1) pixels.setPixelColor(1,0xffff00); 
                 else pixels.setPixelColor(1,0x00ffff); 
                  ASCII_Output(10);    
          break; 
                   
          default:SoundOut(25,50,3,1);     //This is for the Rotary knob switch 
                  OLED_Text("ENC",25,24);    
                  ASCII_Output(0x08);                
          break;                  
        }                   
      }
      else flagA = 0; 
    }
  }
  
    
 if (delay_milli(OLED_REFRESH_RATE,8) == 8) 
 {
  if (keyModeEN == 1) for (uint8_t i=3;i<NUM_LEDS;i++) fadeRGB(i,40); //pixel(0-11),color(0-65535), fade rate
  display.display();
  pixels.show(); 
 }

 if (delay_milli(OLED_REFRESH_RATE / 2, 2) == 2) {
   kerbalLights();
 }
 
}

void kerbalLights() {
  if (kerbalMode == false) {
    return;
  }

  if (kerbalIndex == 0) {
    kerbalIndex = 3;
  }

  if (kerbalIndex < 12) {
    const unsigned colors[] = {0xff00ff, 0xff0040, 0xff << 16, 0x40 << 16};
    uint8_t colors_len = sizeof(colors) / sizeof(colors[0]);
    pixels.setPixelColor(kerbalIndex, colors[(kerbalIndex - 3) % colors_len]);
    //pixels.show();
  }

  if (kerbalIndex == 16) {
    for (int i = 3; i < 12; i++) {
      pixels.setPixelColor(i,0); // TODO reset to original color
    }
    pixels.setPixelColor(3,0xff0000);
    kerbalIndex = 0;
    kerbalMode = false;
  }

  kerbalIndex++;
}

//------------[ INPUT FUNCTIONS ]------------------------------
bool encUpdate() 
{  
  newPos = encoder.getPosition(); //Saves this status 
  if (encoder_pos != newPos) //Checks to see if any changes in the encoder have been made, and if so, do stuff. 
  {
    if (encoder_pos > newPos) 
    {
      gHue = gHue-5; //Determines direction encoder is turning
      if (gHue16 < 2000) gHue16 = 54000; 
      gHue16 = gHue16-1000; //Redundant lazy code cause I need 16bit 
    }
    else 
    {
      gHue = gHue+5;
      if (gHue16 > 54000) gHue16 = 0; 
      gHue16 = gHue16+1000;
    }
    //gHue = constrain(gHue,0,255);
    encoder_pos = newPos; //Saves the current encoder position so it doesn't keep running the code above. 
    return 1; 
  }
  return 0; 
}

void encPosition() //This is by itself due to interupts being attached. Fancy for don't mess with this. 
{  
  encoder.tick(); 
}

//------------[ OUTPUT FUNCTIONS ]------------------------------
void SoundOut(uint16_t freq, uint8_t duration, uint8_t reps, uint8_t temp)
{
  if (SPK_ENABLE == 1) 
  {
    for (uint8_t i=0;i<reps;i++)
    {
      tone(SPK_PIN,freq,duration);
      delay(temp); 
    }
  }
}

void ASCII_Output(uint8_t value) 
{
  if (KB_ENABLE == 1) 
  {    Keyboard.write(value);
  }
}

void OLED_Text(String value, uint8_t xAxis, uint8_t yAxis)
{
  /*
    The OLED Screen is 128x64 pixels. Max xAxis is 128 which would 
    be the last pixel near the right edge of the screen. To put something 
    right in the middle of the screen you would do this: OLED_Output("x",64,32); 
    Just remeber that the position you are stating is the starting position of the cursor. 
    So if you wanted to center the word "center" you would have to actually start slightly more
    left of the screen so it aligns correctly after printing the whole word "center". 
    Would be something like this: OLED_Output("center",40,32); 
  */
  display.setCursor(xAxis,yAxis);
  display.print(value); 
  delay(10); 
}

void OLED_Dec(uint32_t value, uint8_t xAxis, uint8_t yAxis)
{
  display.setCursor(xAxis,yAxis);
  display.print(value,DEC); 
  delay(10); 
}

void OLED_Menu(uint8_t screen,uint8_t tSize,bool tWrap) 
{
  display.setTextSize(tSize);
  display.setTextWrap(tWrap);
  display.setTextColor(SH110X_WHITE,SH110X_BLACK);
  display.clearDisplay();
  switch (screen) 
  {
    case 1: OLED_Text("gHue: ",0,10);  
            OLED_Text("Key: ",0,24);  
            OLED_Text("xHue: ",0,38); 
            // OLED_Text("Clock",80,10);  
            OLED_Text("Sin: ",0,50);
    break; 
               
    default://OLED_Text("First.",50,10); //display value/text, position xAxis, position yAxis 
            OLED_Text("roguh.com",50,30); //See comments inside the function called OLED_Output for more hints 
            //OLED_Text("Furthest.",50,50);               
    break;                  
  }
  display.display(); 
  delay(50); 
}
void hateWords()
{
  switch (1)
  {
    case 1: ASCII_Output(0x4D);//M
            ASCII_Output(0x4F);//O
            ASCII_Output(0x4E);//N
            ASCII_Output(0x4B);//K
            ASCII_Output(0x45);//E
            ASCII_Output(0x59);//Y
            ASCII_Output(0x20);//_
            ASCII_Output(0x42);//B
            ASCII_Output(0x52);//R
            ASCII_Output(0x41);//A
            ASCII_Output(0x49);//I
            ASCII_Output(0x4E);//N
            ASCII_Output(0x21);//!
            ASCII_Output(0x20);//_ 
            
  
    break;
    
    case 2: ASCII_Output(0x42);//B
            ASCII_Output(0x52);//R
            ASCII_Output(0x55);//U
            ASCII_Output(0x48);//H
            ASCII_Output(0x20);//_
     
    break;

    case 3: ASCII_Output(0x4F);//O
            ASCII_Output(0x4F);//O
            ASCII_Output(0x46);//F
            ASCII_Output(0x21);//!
            ASCII_Output(0x20);//_    
        
    break;

    case 4: ASCII_Output(0x59);//Y
            ASCII_Output(0x4F);//O 
            ASCII_Output(0x55);//U 
            ASCII_Output(0x20);//_
            ASCII_Output(0x53);//S
            ASCII_Output(0x55);//U
            ASCII_Output(0x43);//C
            ASCII_Output(0x4B);//K
            ASCII_Output(0x21);//!
            ASCII_Output(0x20);//_ 
                  
    break;

    default:ASCII_Output(0x59);//Y
            ASCII_Output(0x4F);//O
            ASCII_Output(0x55);//U
            ASCII_Output(0x20);//_
            ASCII_Output(0x46);//F
            ASCII_Output(0x41);//A
            ASCII_Output(0x54);//T
            ASCII_Output(0x20);//_
            ASCII_Output(0x4D);//M
            ASCII_Output(0x4F);//O
            ASCII_Output(0x4E);//N
            ASCII_Output(0x4B);//K
            ASCII_Output(0x45);//E
            ASCII_Output(0x59);//Y
            ASCII_Output(0x21);//!
            ASCII_Output(0x20);//_  
            delay(10); 

    break; 
  }
}

void kerbal() {
  ASCII_Output("RTZ ");
}

void closeVim() {
  ASCII_Output(":qa!\n");
}

void fibonacciPython() {
  ASCII_Output("golden_ratio = (1 + 5 ** 0.5) * 0.5 ; fib = lambda i: round(golden_ratio ** i / 5 ** 0.5)");
}

void ASCII_Output(String str) {
  for (int i=0; i < str.length(); i++) {
    ASCII_Output(str[i]);
  }
}

void pixelPond()
{
  for (uint8_t i=0;i<NUM_LEDS;i++) pixels.setPixelColor(i,pixels.ColorHSV(gHue16,255,255)); //hue (0-65536), sat(0-255), bright (0-255)
  
  //**** CONSTRUCTION ZONE ***** 
  //xHue = SinGen16(20,75,250); //Rate, toLow, toHigh
  //for (uint8_t i=0;i<NUM_LEDS;i++) pixels.setPixelColor(i,pixels.ColorHSV(gHue16,255,xHue)); 
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

//------------[ LED FUNCTIONS ]------------------------------
uint16_t SinGen16(uint8_t tRate,uint16_t toLow,uint16_t toHigh)
{
  if (delay_milli(tRate,6) == 6) 
  {
    angleDeg++; 
    if (angleDeg > 180) angleDeg = 0; 
    angleRad = (angleDeg * M_PI/180); //M_PI is a built in to Arduino math function. 3.14159... 
    sinResult = (sin(angleRad) * 100); //This returns Sin value 0.00 to 1.00 then make it whole numbers 0-100
    //This makes that 0-100 value above into what is requested when this function is called 
    sinResultMap = map(sinResult,0,100,toLow,toHigh);  //variable, fromLow, fromHigh, toLow, toHigh

    //**** CONSTRUCTION ZONE ***** 
    //cosResult = (cos(angleRad) * 100);//in progress 
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  }
  return sinResultMap; 
}

uint32_t MergeRGB(byte WheelPos) //This function merges 3 byte values of R G B into one large variable so you can output the full spectrum with one variable. 
{
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85)
  {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) 
  {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void fadeRGB(uint8_t i,uint16_t t)
{
  
  if (delay_milli(t,5) == 5) 
  {
    if (xHue16 != gHue16)
    {
      if (xHue16 > gHue16) xHue16 = (xHue16 - 500); 
      else xHue16 = (xHue16 + 500);
    } 
  } 
  
  pixels.setPixelColor(i,pixels.ColorHSV(xHue16 + (i * 10),255,255)); 
  
}
//------------[ DELAY FUNCTION ]------------------------------ **Special code that allows for delays without stopping the code from running. Simulated parallel processing. 
uint8_t delay_milli(uint16_t interval,uint8_t channelx)
{
  switch (channelx) 
  {
    case 1: 
           currentMillis[1] = millis(); 
           if ((uint16_t)(currentMillis[1] - prevMillis[1]) >= interval) { prevMillis[1] = currentMillis[1]; return 1; }
           break;
    case 2: 
           currentMillis[2] = millis(); 
           if ((uint16_t)(currentMillis[2] - prevMillis[2]) >= interval) { prevMillis[2] = currentMillis[2]; return 2; }
           break;
    case 3: 
           currentMillis[3] = millis(); 
           if ((uint16_t)(currentMillis[3] - prevMillis[3]) >= interval) { prevMillis[3] = currentMillis[3]; return 3; }
           break;  
   case 4: 
           currentMillis[4] = millis(); 
           if ((uint16_t)(currentMillis[4] - prevMillis[4]) >= interval) { prevMillis[4] = currentMillis[4]; return 4; }
           break;   
   case 5: 
           currentMillis[5] = millis(); 
           if ((uint16_t)(currentMillis[5] - prevMillis[5]) >= interval) { prevMillis[5] = currentMillis[5]; return 5; }
           break; 
   case 6: 
           currentMillis[6] = millis(); 
           if ((uint16_t)(currentMillis[6] - prevMillis[6]) >= interval) { prevMillis[6] = currentMillis[6]; return 6; }
           break;    
   case 7: 
           currentMillis[7] = millis(); 
           if ((uint16_t)(currentMillis[7] - prevMillis[7]) >= interval) { prevMillis[7] = currentMillis[7]; return 7; }
           break;  
   case 8: 
           currentMillis[8] = millis(); 
           if ((uint16_t)(currentMillis[8] - prevMillis[8]) >= interval) { prevMillis[8] = currentMillis[8]; return 8; }
           break;   
                                      
    default: return 0; break; 
  }   
  return 0; 
}




//--------------------------------------------------------------------------------------
/*                        
        ooooooooo.   ooooo ooooooooo.   
        `888   `Y88. `888' `888   `Y88. 
         888   .d88'  888   888   .d88' 
         888ooo88P'   888   888ooo88P'  
         888`88b.     888   888         
         888  `88b.   888   888         
        o888o  o888o o888o o888o       
                                                                                                                                          
//-------------[ CODE GRAVEYARD ]--------------------------------------------------------

for (uint8_t i=0;i<12;i++) pixels.setPixelColor(i,MergeRGB(gHue)); 

  OLED_Text("   ",80,50); 
  OLED_Dec(cosResult,80,50); 


//uint8_t Global_Bright[]   = { 20,30,60 };   //WARNING!   
//uint8_t Global_Color[]    = { 1,2,3,4,5,6,7 };   //WARNING!   
//uint8_t Mode_Select       = DEFAULT_MODE; //EEPROM.read(2);











 */
