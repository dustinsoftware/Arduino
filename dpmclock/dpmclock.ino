#include <phi_big_font.h>
#include <LiquidCrystal.h>
#include <Time.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
long interruptTime = 0;
int buttonState = 0;
int delayTimes = 0;
int lastMinute = -1;

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 2);
  init_big_font(&lcd);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  setTime(0,0,0,1,1,2012);
  
  
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  // print the number of seconds since reset:
  
  int buttonPress = readInput();

  if (buttonPress == 1)
    adjustTime(60);  
  else if (buttonPress == 2)
    adjustTime(-60);
  
  if (minute() == lastMinute)
    return;
  
  lastMinute = minute();
  
  if (hourFormat12() < 10) {
    render_big_number(0,0,0);
    render_big_number(hourFormat12(),4,0);
  } else {
    render_big_number(hourFormat12(),0,0);
  }
  render_big_msg(":",8,0);
  if (minute() < 10) {
    render_big_number(0,12,0);
    render_big_number(minute(),16,0);
  } else {
    render_big_number(minute(),12,0);
  } 
}

int readInput() {  
  int read1 = digitalRead(A0);
  int read2 = digitalRead(A5);

  if (read1 == HIGH)
  {
    return delayButtonState(1);
  }    
  else if (read2 == HIGH)
  {
   return delayButtonState(2);
  }  
  else
  {  
    buttonState = 0;
    delayTimes = 0;
    interruptTime = millis();
    return 0;
  }
}

int delayButtonState(int button)
{
  int ourButtonState = buttonState;
  buttonState = button;
    
  if (delayTimes > 2)
    return button;
  else if (ourButtonState != button || millis() - interruptTime > 500)
  {
    interruptTime = millis();
    delayTimes++;
    return button;
  }
  else
    return 0;
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}
