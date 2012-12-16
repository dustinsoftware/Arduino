#include <TFT.h>
#include <TouchScreen.h>
#include <stdint.h>
#include <X10Firecracker.h>

#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3

#define RTS_PIN     23	                // RTS line for C17A - DB9 pin 7
#define DTR_PIN     22	                // DTR line for C17A - DB9 pin 4
#define BIT_DELAY   1                   // mS delay between bits (1 mS OK)

#define TS_MINX 140 
#define TS_MAXX 900
#define TS_MINY 120
#define TS_MAXY 940
#define C_NUMBUTTONS 4
#define C_OFFBUTTON 3
#define C_MAXCOMMANDS 10

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); //init TouchScreen port pins

int m_buttonX = 10;
int m_buttonY;
int m_width = 180;
int m_height = 70;
int m_consoleLine = 0;
int m_commandIndex = 0;
unsigned long m_clearSnowTimer = 0;
unsigned long m_pressed;
unsigned long m_lastCommandTime = 0;

struct button_t {
  int color;
  int state;
  String text;
  int x;
  int y;
} m_buttons[C_NUMBUTTONS];

struct command_t {
  int number;
  CommandCode command;
} m_commands[C_MAXCOMMANDS];

void setup()
{
  X10.init(RTS_PIN, DTR_PIN, BIT_DELAY);
  for (int i = 0; i < C_NUMBUTTONS; i++)
    m_buttons[i].color = 0xEE00;

  m_buttons[0].text = "Stereo";
  m_buttons[1].text = "Xmas Tree";
  m_buttons[2].text = "Floor Lamp";
  m_buttons[3].text = "All Off";
  m_buttons[3].color = RED;
  redrawScreen();
}

void loop()
{
  Point p = getTouch();  
  
  if (p.z > ts.pressureThreshhold && m_pressed == 0 && m_commandIndex < C_MAXCOMMANDS) 
  {   
    m_pressed = millis();
    m_clearSnowTimer = millis();
    
    for(int i = 0; i < C_NUMBUTTONS; i++)
    {
      button_t *button_p = &m_buttons[i];
      if (buttonPressed(*button_p, p))
      {
        if (i == C_OFFBUTTON)
        {
          for (int i = 0; i < C_NUMBUTTONS && i != C_OFFBUTTON; i++)
          {
            m_buttons[i].state = false;
            drawStatus(m_buttons[i]);
            m_commands[m_commandIndex].number = i + 1;
            m_commands[m_commandIndex++].command = cmdOff;
          }
        }
        else 
        {
          (*button_p).state = !(*button_p).state;
          drawStatus(*button_p);
          m_commands[m_commandIndex].number = i + 1;

          if ((*button_p).state)
            m_commands[m_commandIndex++].command = cmdOn;
          else
            m_commands[m_commandIndex++].command = cmdOff;
        }
      }
    }
  }
  else if (p.z > ts.pressureThreshhold) 
  {
    Tft.fillRectangle(p.x, p.y, 2, 2, WHITE);

    m_pressed = millis();
    m_clearSnowTimer = millis();
  }
  else
  {
    if (m_pressed != 0 && millis() - m_pressed > 200)
      m_pressed = 0;
    if (m_clearSnowTimer != 0 && millis() - m_clearSnowTimer > 10000)
    {
      redrawScreen();
      m_clearSnowTimer = 0;
    }
  }
  
  if (m_commandIndex > 0 && millis() - m_lastCommandTime > 1000)
  {
    m_lastCommandTime = millis();
    
    command_t command = m_commands[0];
    
    m_commandIndex--;
    for (int i = 1; i <= m_commandIndex; i++)
    {
      m_commands[i - 1].number = m_commands[i].number;
      m_commands[i - 1].command = m_commands[i].command;
    }

    X10.sendCmd(hcA, command.number, command.command);
  }
}

Point getTouch()
{
  Point p = ts.getPoint();    
  p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
  p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
  
  if (p.x < 0)
    p.x = 0;
  if (p.y < 0)
    p.y = 0;
  if (p.x > 240)
    p.x = 240;
  if (p.y > 320)
    p.y = 320;
  
  return p;
}

boolean buttonPressed(struct button_t b, Point p)
{
  return p.x >= b.x && p.y >= b.y && p.x <= b.x + m_width && p.y <= b.y + m_height;
}

void redrawScreen()
{
  Tft.init();
  drawButtons();
  
  for (int i = 0; i < C_NUMBUTTONS && i != C_OFFBUTTON; i++)
    drawStatus(m_buttons[i]);
}

void drawButtons()
{
  m_buttonY = 10;
  
  for (int i = 0; i < C_NUMBUTTONS; i++)
    drawButton(m_buttons[i]);
}
void drawButton(struct button_t &b)
{
  b.x = m_buttonX;
  b.y = m_buttonY;
  
  Tft.drawRectangle(m_buttonX, m_buttonY, m_width, m_height, GRAY1);
  Tft.fillRectangle(m_buttonX + 1, m_buttonY + 1, m_width - 1, m_height - 1, b.color);
  drawString(b.text, m_buttonX + 5, m_buttonY + 5, WHITE);
  
  m_buttonY += m_height + 5;
}

void drawString(String s, int x, int y, int color)
{
  char array[255];
  s.toCharArray(array, 255);
  Tft.drawString(array, x, y, 2, color); 
}

void drawStatus(struct button_t b)
{
  int color;
  if (b.state == true)
    color = GREEN;
  else
    color = RED;
    
  Tft.fillCircle(b.x + 200, b.y + 30, 10, color);
  Tft.drawCircle(b.x + 200, b.y + 30, 10, WHITE);
}

void printString(String s)
{
  if (m_consoleLine == 32)
    m_consoleLine = 0;
  
  char array[255];  
  s.toCharArray(array, 255);
  
  Tft.fillRectangle(0,(m_consoleLine) * 10,240,10,0);
  Tft.drawString(array, 0, m_consoleLine * 10, 1, 0xFFFFFF); 
  m_consoleLine++;
}

