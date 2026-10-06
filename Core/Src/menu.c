#include "main.h"
#include "newglobals.h"
#include "globalfunctions.h"
#include <stdio.h>

extern u8g2_t u8g2;

uint32_t settingsData = 0;  // Global variable to hold the packed settings data



int timeSet = 0;
int cycles = 0;
int maxSpeed = 350;
int pumpNo = 0;
int accTime=0;
int decTime = 0;
int dispStop = 0;
int selected = 0;
int firstVisible = 0;
int parentIndex = 0;
bool menuUpdate = true;
const int lineHeight = 12;
const int visibleLines = 5;


// Action Prototypes

void primePump() {};
void calibration() {};
void diagnostics() {};
void wifiReset() {};
void factoryReset() {};
void firmwareInfo() {};
void aboutInfo() {};

int reverseAngle = 0;
int currRevAng = 0;
int currtoneSet = 0;
bool needsSave = false;
bool contrastflag = true;
int currentcontrast = 0;
bool languageHindi = false;
int currentLanguage = 0;
bool tubingAlertEnabled = false;
int currtubeAlert = 0;
bool wifiEnabled = false;
int currwifiSet = 0;
bool wifiResetFlag = false;
int currwifiReset = 0;
int currextMenu = 0;
bool stopBit = false;
int currentStopBitIndex = 0;
int currentBaudIndex = 0;
int currentParityIndex = 0;
bool levelTrigHigh = false;
int currentLevelTrigIndex = 0;
int currPTUnitSel = 0;
int currRTUnitSel = 0;
runtimeUnit currentRunTimeUnit = seconds ;
runtimeUnit currentPauseTimeUnit = seconds ;
// void prefsload(){
//   prefs.begin(NVS_NS, true);
//   prefs.getBytes("settingsData", &settingsData, sizeof(settingsData));
//   prefs.getBytes("dispset", &dispenseSettings, sizeof(dispenseSettings));
//   prefs.end();


// //disp cycle  settings 
//   runTime = bitGet(dispenseSettings, 0, 10);
//   pauseTime = bitGet(dispenseSettings, 10, 10);
//   dispCycles = bitGet(dispenseSettings, 20, 10);
//   Serial.println(dispenseSettings, BIN);

//   // menu settings 
//   currRevAng = bitGet(settingsData, 2, 2);
//  int  reverseanglearr[4] = {0, 180, 360, 720};
//   reverseAngle = reverseanglearr[currRevAng];
// //tone settings
//   toneproduce = bitGet(settingsData, 0, 1);
//   currtoneSet = toneproduce ? 0 : 1;
 
//   //contrast settings
//   contrastflag = bitGet(settingsData, 1, 1);
//   currentcontrast = contrastflag ? 0 : 1;
//   if(contrastflag){
//     u8g2.sendF("c", 0x0a7); // Set contrast to light mode
//   }
//   else{
//     u8g2.sendF("c", 0x0a6); // Set contrast to dark mode
//   }

//   //language settings
//   languageHindi = bitGet(settingsData, 5, 1);
//   currentLanguage = languageHindi ? 0 : 1;

//   //tubing alert settings
//   tubingAlertEnabled = bitGet(settingsData, 6, 1);
//   currtubeAlert = tubingAlertEnabled ? 0 : 1;

//   //wifi settings
//   wifiEnabled = bitGet(settingsData, 7, 1);
//   currwifiSet = wifiEnabled ? 0 : 1;

//   //wifi reset settings
//   wifiResetFlag = bitGet(settingsData, 8, 1);
//   currwifiReset = wifiResetFlag ? 0 : 1;

//   //ext controls settings
//   currextMenu = bitGet(settingsData, 9, 2); 

//   //baud settings
//   currentBaudIndex = bitGet(settingsData, 11, 2);

//   //parity settings
//   currentParityIndex = bitGet(settingsData, 13, 2);

//   //stop bit settings
//   stopBit = bitGet(settingsData, 15, 1);
//   currentStopBitIndex = stopBit ? 1 : 0;

//   // level trigger settings
//   levelTrigHigh = bitGet(settingsData, 16, 1);
//   currentLevelTrigIndex = levelTrigHigh ? 0 : 1;

//   // runtime unit settings
//   currRTUnitSel = bitGet(settingsData, 17, 2);
//   currentRunTimeUnit   = (runtimeUnit)currRTUnitSel;
   

//   //pause time unit settings
//   currPTUnitSel = bitGet(settingsData, 19, 2);
//    currentPauseTimeUnit = (runtimeUnit)currPTUnitSel;

// }

// void getUnitsFromSettings(runtimeUnit &currentRunTimeUnit) {
//   switch(currRTUnitSel){
//     case 0: currentRunTimeUnit = days; break;
//     case 1: currentRunTimeUnit = hours; break;
//     case 2: currentRunTimeUnit = minutes; break;
//     case 3: currentRunTimeUnit = seconds; break;
//   }

//   switch(currPTUnitSel){
//     case 0: currentPauseTimeUnit = days; break;
//     case 1: currentPauseTimeUnit = hours; break;
//     case 2: currentPauseTimeUnit = minutes; break;
//     case 3: currentPauseTimeUnit = seconds; break;
//   }
// }

// void settingsSave() {
//   prefs.begin(NVS_NS, false);
//   prefs.putBytes("settingsData", &settingsData, sizeof(settingsData));
//   prefs.end();
// }
typedef struct MenuNode MenuNode;

struct MenuNode {
  const char* name;
  const char* title;   // short title used when this node becomes a submenu header
  MenuNode* children;
  uint8_t childCount;
  void (*action)();
  int* value;
  int minValue;
  int maxValue;
};

// Functional
void languHindi() {  needsSave = true; languageHindi = true; currentLanguage = 0;
  bitsSet(settingsData, 5, 1, 0);
}
void languEnglish() {  needsSave = true; languageHindi = false; currentLanguage = 1;
  bitsSet(settingsData, 5, 1, 1);
}
MenuNode language[] = {
  { "HINDI", NULL, NULL, 0, languHindi, NULL, 0, 0 },
  { "ENGLISH", NULL, NULL, 0, languEnglish, NULL, 0, 0 },
};
void contrastLight(){ contrastflag = true; currentcontrast=0; needsSave = true;
    // u8g2.sendF("c", 0x0a7); // Set contrast to light mode
  bitsSet(settingsData, 1, 1, 1);
}
void contrastDark(){ contrastflag = false; currentcontrast=1; needsSave = true;
  bitsSet(settingsData, 1, 1, 0);
    // u8g2.sendF("c", 0x0a6); // Set contrast to dark mode
} 
MenuNode contrastMenu[] = {
  { "LIGHT", NULL, NULL, 0, contrastLight, NULL, 0, 0 },
  { "DARK", NULL, NULL, 0, contrastDark, NULL, 0, 0 },
};
bool  toneproduce = false;

void toneEnable(){ toneproduce = true;   needsSave = true; currtoneSet = 0;
  bitsSet(settingsData, 0, 1, 1);
  }
void toneDisable(){ toneproduce = false;  needsSave = true; currtoneSet = 1;
  bitsSet(settingsData, 0, 1, 0);
}

MenuNode toneSet[] = {
  { "ENABLE", NULL, NULL, 0, toneEnable, NULL, 0, 0 },
  { "DISABLE", NULL, NULL, 0, toneDisable, NULL, 1, 0 },
};

MenuNode generalMenu[] = {
  { "TIMESET", NULL, NULL, 0, NULL, &timeSet, 0, 999 },
  { "LANGUAGE", NULL, language, 2, NULL, &currentLanguage, 0, 0 },
  { "CONTRAST", NULL, contrastMenu, 2, NULL, &currentcontrast, 0, 0 },
  { "TONE SET", NULL, toneSet, 2, NULL, &currtoneSet, 0, 0 },
}; 
// enum runtimeUnit{
//     days, hours , minutes, seconds 
// };
// runtimeUnit currentRunTimeUnit = seconds ;


void selRTTimeSec(){currentRunTimeUnit = seconds;    currRTUnitSel = 3; needsSave = true; bitsSet(settingsData, 17, 2, 3);}
void selRTTimeMin(){currentRunTimeUnit = minutes;    currRTUnitSel = 2; needsSave = true; bitsSet(settingsData, 17, 2, 2);}
void selRTTimeHours(){currentRunTimeUnit = hours;    currRTUnitSel = 1; needsSave = true; bitsSet(settingsData, 17, 2, 1);}
void selRTTimeDays (){ currentRunTimeUnit = days;    currRTUnitSel = 0; needsSave = true; bitsSet(settingsData, 17, 2, 0);}

MenuNode rtUnitMenu[] = {
  { "DAYS", NULL, NULL, 0, selRTTimeDays, NULL, 0, 0 },
  { "HOURS", NULL, NULL, 0, selRTTimeHours, NULL, 1, 0 },
  { "MINUTES", NULL, NULL, 0, selRTTimeMin, NULL, 2, 0 },
  { "SECONDS", NULL, NULL, 0, selRTTimeSec, NULL, 3, 0 },
};

// runtimeUnit currentPauseTimeUnit = seconds ;

void selPauseTimeSec(){currentPauseTimeUnit = seconds;    currPTUnitSel = 3; needsSave = true; bitsSet(settingsData, 19, 2, 3);}
void selPauseTimeMin(){currentPauseTimeUnit = minutes;    currPTUnitSel = 2; needsSave = true; bitsSet(settingsData, 19, 2, 2);}
void selPauseTimeHours(){currentPauseTimeUnit = hours;    currPTUnitSel = 1; needsSave = true; bitsSet(settingsData, 19, 2, 1);}
void selPauseTimeDays (){ currentPauseTimeUnit = days;    currPTUnitSel = 0; needsSave = true; bitsSet(settingsData, 19, 2, 0);}

MenuNode pauseTimeUnitMenu[] = {
  { "DAYS", NULL, NULL, 0, selPauseTimeDays, NULL, 0, 0 },
  { "HOURS", NULL, NULL, 0, selPauseTimeHours, NULL, 1, 0 },
  { "MINUTES", NULL, NULL, 0, selPauseTimeMin, NULL, 2, 0 },
  { "SECONDS", NULL, NULL, 0, selPauseTimeSec, NULL, 3, 0 },
};

void revAng0(){ reverseAngle=0; currRevAng=0; needsSave = true;
bitsSet(settingsData, 2, 2, 0);
}
void revAng180(){ reverseAngle = 180; currRevAng=1;   needsSave = true;
bitsSet(settingsData, 2, 2, 1);
}
void revAng360(){ reverseAngle = 360; currRevAng=2;    needsSave = true;
bitsSet(settingsData, 2, 2, 2);
}
void revAng720(){ reverseAngle = 720;  currRevAng=3;    needsSave = true;
bitsSet(settingsData, 2, 2, 3);

}



MenuNode revAngMenu[] = {
  { "0", NULL, NULL, 0, revAng0, NULL, 0, 0 },
  { "180", NULL, NULL, 0, revAng180, NULL, 1, 0 },
  { "360", NULL, NULL, 0, revAng360, NULL, 2, 0 },
  { "720", NULL, NULL, 0, revAng720, NULL, 3, 0 },
};

void tubingAlertEnable(){ needsSave = true; tubingAlertEnabled = true;  currtubeAlert=0; 
  bitsSet(settingsData, 6, 1, 1);}

void tubingAlertDisable(){ needsSave = true; tubingAlertEnabled = false;  currtubeAlert=1; 
  bitsSet(settingsData, 6, 1, 0);}

MenuNode tubeAlertMenu[] = {
  { "YES", NULL, NULL, 0, NULL, NULL, 0, 0 },
  { "NO", NULL, NULL, 0, NULL, NULL, 0, 0 },
};

MenuNode funtionalMenu[] = {
  { "RT UNIT", NULL, rtUnitMenu, 4, NULL, &currRTUnitSel, 0, 0 },
  { "PT UNIT", NULL, pauseTimeUnitMenu, 4, NULL, &currPTUnitSel, 0, 0 },
  { "REVERSE ANGLE", "REV ANGLE", revAngMenu, 4 , NULL, &currRevAng, 0, 0 },
  { "PUMP NO.", NULL, NULL, 0, NULL, &pumpNo, 0, 999 },
  { "MAX SPEED", NULL, NULL, 0, NULL, &maxSpeed, 0, 999 },
  { "TUBING ALERT", "TUBE ALERT", tubeAlertMenu, 2, NULL, &currtubeAlert, 0, 0 },
};

void wifiEnable(){ wifiEnabled = true; currwifiSet=0; needsSave = true;
  bitsSet(settingsData, 7, 1, 1);
}
void wifiDisable(){ wifiEnabled = false; currwifiSet=1; needsSave = true;
  bitsSet(settingsData, 7, 1, 0);
}
MenuNode wifiMenu[] = {
  { "ENABLE", NULL, NULL, 0, wifiEnable, NULL, 0, 0 },
  { "DISABLED", NULL, NULL, 0, wifiDisable, NULL, 0, 0 },
};

void wifiResetYes(){ wifiResetFlag = true; currwifiReset=0; needsSave = true;
  bitsSet(settingsData, 8, 1, 1);
}
void wifiResetNo(){ wifiResetFlag = false; currwifiReset=1; needsSave = true;
  bitsSet(settingsData, 8, 1, 0);
}


MenuNode wifiResetMenu[] = {
  { "YES", NULL, NULL, 0, wifiResetYes, NULL, 0, 0 },
  { "NO", NULL, NULL, 0, wifiResetNo, NULL, 0, 0 },
};

void extCurrent(){ currextMenu=0; needsSave = true;
  bitsSet(settingsData, 9, 2, 0);
}
void extVolt5(){ currextMenu=1; needsSave = true;
  bitsSet(settingsData, 9, 2, 1);
}
void extVolt10(){ currextMenu=2; needsSave = true;
  bitsSet(settingsData, 9, 2, 2);
}
MenuNode extMenu[] = {
  { "CURRENT", NULL, NULL, 0, extCurrent, NULL, 0, 0 },
  { "VOLT 0-5V", NULL, NULL, 0, extVolt5, NULL, 0, 0 },
  { "VOLT 0-10V", NULL, NULL, 0, extVolt10, NULL, 0, 0 },
};

MenuNode softwareMenu[] = {
  { "BOOT V3.1.0", NULL, NULL, 0, wifiReset, NULL, 0, 0 },
  { "USER v3.2.3", NULL, NULL, 0, NULL, NULL, 0, 0 },
};

MenuNode ExternalMenu[] = {
  { "WIFI SETTINGS", "WIFI SET", wifiMenu, 2, NULL, &currwifiSet, 0, 0 },
  { "WIFI RST", NULL, wifiResetMenu, 2, NULL, &currwifiReset, 0, 0 },
  { "EXT CONTROLS", "EXT CONT", extMenu, 3, NULL, &currextMenu, 0, 0 },
  { "SOFTWARE", NULL, softwareMenu, 2, NULL, NULL, 0, 0 },
};


void baud4800(){ currentBaudIndex=0; needsSave = true;
  bitsSet(settingsData, 11, 2, 0);
}
void baud9800(){ currentBaudIndex=1; needsSave = true;
  bitsSet(settingsData, 11, 2, 1);
}
void baud19200(){ currentBaudIndex=2; needsSave = true;
  bitsSet(settingsData, 11, 2, 2);
}
void baud38400(){ currentBaudIndex=3; needsSave = true;
  bitsSet(settingsData, 11, 2, 3);
}

MenuNode baudMenu[] = {
  { "4800", NULL, NULL, 0, baud4800, NULL, 0, 0 },
  { "9800", NULL, NULL, 0, baud9800, NULL, 0, 0 },
  { "19200", NULL, NULL, 0, baud19200, NULL, 0, 0 },
  { "38400", NULL, NULL, 0, baud38400, NULL, 0, 0 },
};


void parityNone(){ currentParityIndex=0; needsSave = true;
  bitsSet(settingsData, 13, 2, 0);
}
void parityEven(){ currentParityIndex=1; needsSave = true;
  bitsSet(settingsData, 13, 2, 1);
}
void parityOdd(){ currentParityIndex=2; needsSave = true;
  bitsSet(settingsData, 13, 2, 2);
}
MenuNode parityMenu[] = {
  { "NONE", NULL, NULL, 0, parityNone, NULL, 0, 0 },
  { "EVEN", NULL, NULL, 0, parityEven, NULL, 0, 0 },
  { "ODD", NULL, NULL, 0, parityOdd, NULL, 0, 0 },
};

void stopBit1(){ currentStopBitIndex=0; needsSave = true;
  bitsSet(settingsData, 15, 1, 0);
}
void stopBit2(){ currentStopBitIndex=1; needsSave = true;
  bitsSet(settingsData, 15, 1, 1);
}
MenuNode stopBitMenu[] = {
  { "1", NULL, NULL, 0, stopBit1, NULL, 0, 0 },
  { "0", NULL, NULL, 0, stopBit2, NULL, 0, 0 },
};

void levelTrigHighFunc(){ levelTrigHigh = true; currentLevelTrigIndex=0;    needsSave = true;
  bitsSet(settingsData, 16, 1, 1);
}
void levelTrigLowFunc(){ levelTrigHigh = false; currentLevelTrigIndex=1;  needsSave = true;
  bitsSet(settingsData, 16, 1, 0);
}

MenuNode leveltrigmenu[] = {
  { "HIGH", NULL, NULL, 0, levelTrigHighFunc, NULL, 0, 0 },
  { "LOW", NULL, NULL, 0, levelTrigLowFunc, NULL, 0, 0 },
};

MenuNode systemMenu[] = {
  { "BAUD", NULL, baudMenu, 4, NULL, &currentBaudIndex, 0, 0 },
  { "PARITY", NULL, parityMenu, 3, NULL, &currentParityIndex, 0, 0 },
  { "PULSE TRIGGER", "PLS TRIS", NULL, 0, NULL, NULL, 0, 0 },
  { "LEVEL TRIGGER", "LVL TRIG", leveltrigmenu, 2, NULL, &currentLevelTrigIndex, 0, 0 },
  { "DISP STOP", NULL, NULL, 0, NULL, &dispStop, 0, 10 },
  { "ACC TIME", NULL, NULL, 0, NULL, &accTime, 0, 999 },
  { "DEC TIME", NULL, NULL, 0, NULL, &decTime, 0, 999 },
  { "PARAM RESET", NULL, wifiResetMenu, 2, NULL, NULL, 0, 0 },
  { "STOP BIT", NULL, stopBitMenu, 2, NULL, &currentStopBitIndex, 0, 0 },
};

MenuNode mainMenu[] = {
  { "GENERAL", "GEN", generalMenu, 4, NULL, NULL, 0, 0 },
  { "FUNCTIONAL", "FUNC", funtionalMenu, 6, NULL, NULL, 0, 0 },
  { "SYSTEM", "SYS", systemMenu, 9, NULL, NULL, 0, 0 },
  { "EXTERNAL", "EXT", ExternalMenu, 4, NULL, NULL, 0, 0 },
};

MenuNode* currentMenu = mainMenu;  // Pointer to the active menu array
int currentCount = 4;              // How many items in the active array (mainMenu has 4)

enum { MAX_DEPTH = 5 };
const char* currentTitle = "MENU";
MenuNode* menuStack[MAX_DEPTH];
int countStack[MAX_DEPTH];
int selectedStack[MAX_DEPTH];
int firstVisibleStack[MAX_DEPTH];
const char* titleStack[MAX_DEPTH];
int stackIndex = 0;  // Current depth level


GPIO_TypeDef *activeHeldPort = NULL;
uint16_t activeHeldPin = 0;
unsigned long longPressTime = 500; // ms to wait before auto-repeat starts
uint8_t debounceDelay = 50;   // ms to wait for debounce
const unsigned long repeatInterval = 0; // ms per auto-repeat step once longPressTime elapses
unsigned long LongPressupLastPressTime = 0;
unsigned long LongPressdownLastPressTime = 0;
bool lastState[9];     //used to store the last state of each button for debouncing
unsigned long lastPressTime[9];  //used to store the last time each button was pressed for debouncing 
bool pressedWithDebounce(GPIO_TypeDef *GPIOx,
                         uint16_t pin,
                         bool *lastState,
                         unsigned long *lastPressTime,
                         uint32_t *lastRepeatTime)
{
    // Another button already owns the press
    if (activeHeldPort != NULL &&
        (activeHeldPort != GPIOx || activeHeldPin != pin))
    {
        return false;
    }

    bool currentState =
        (HAL_GPIO_ReadPin(GPIOx, pin) == GPIO_PIN_SET);

    bool pressed = false;
    uint32_t now = HAL_GetTick();

    // New press - falling edge
    if (*lastState == true &&
        currentState == false &&
        (now - *lastPressTime) > debounceDelay)
    {
        pressed = true;
            if(toneproduce) {
        buzzerStart = true;    

            Buzzer_Beep();

            }
        *lastPressTime = now;

        if (lastRepeatTime != NULL)
            *lastRepeatTime = now;

        // Store port + pin
        activeHeldPort = GPIOx;
        activeHeldPin = pin;

    }

    // Auto-repeat
    else if (lastRepeatTime != NULL &&
             currentState == false &&
             *lastState == false &&
             (now - *lastPressTime) >= longPressTime &&
             (now - *lastRepeatTime) >= repeatInterval)
    {
        pressed = true;

        *lastRepeatTime = now;
    }

    // Release
    if (currentState == true &&
        *lastState == false &&
        activeHeldPort == GPIOx &&
        activeHeldPin == pin)
    {
        activeHeldPort = NULL;
        activeHeldPin = 0;
    }

    *lastState = currentState;

    return pressed;
}

// Edit Mode State
bool isEditing = false;

void draw1pxScrollbar(int selected_index)
{
  int total_items = 0;
  int visible_items = 5;

  total_items = currentCount;

  int sb_x = 126;
  int sb_y = 0;
  int sb_h = 63;

  int bar_height = 0;
  int bar_y = sb_y;

  if (total_items <= visible_items)
  {
    return;
   
  }
  // CASE 2: Items exceed visible area (needs scrolling)
  else
  {
    // Calculate the proportional height of the indicator line
    bar_height = (visible_items * sb_h) / total_items;

    // Ensure the indicator remains visible
    if (bar_height < 3) bar_height = 3;

    // Fixed math: Protected against division by zero and clean steps
    int scroll_slots = total_items - 1;
    int travel_dist = sb_h - bar_height;

    bar_y = sb_y + ((selected_index * travel_dist) / scroll_slots);

    // Safety boundary check to prevent drawing off-screen
    if (bar_y + bar_height > sb_h)
    {
      bar_y = sb_h - bar_height;
    }
  }

  // Draw a 1-pixel wide vertical solid line 
  u8g2_DrawLine(&u8g2, sb_x, bar_y, sb_x, bar_y + bar_height - 1);
}
void clampMenuWindow()
{
  if (selected < 0) selected = 0;
  if (selected >= currentCount) selected = currentCount - 1;

  if (firstVisible < 0) firstVisible = 0;

  const int maxFirstVisible = (currentCount > visibleLines) ? (currentCount - visibleLines) : 0;
  if (firstVisible > maxFirstVisible) firstVisible = maxFirstVisible;
}
int getMenuY(uint8_t totalItems, uint8_t index)
{
  static const uint8_t yPos[][5] = {
    { 38 },                 // 1 item
    { 22, 54 },             // 2 items
    { 17, 38, 59 },         // 3 items
    { 12, 28, 44, 59 },     // 4 items
    { 12, 24, 36, 48, 60 }  // 5 or more items
  };

  if (totalItems < 1)
    totalItems = 1;
  else if (totalItems > 5)
    totalItems = 5;

  return yPos[totalItems - 1][index];
}

void drawMenu()
{
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
  const char* displayTitle;

  if (isEditing)
  {
    displayTitle = currentMenu[selected].name;
  }
  else
  {
    displayTitle = currentTitle;
  }

  int titleWidthMenu = u8g2_GetStrWidth(&u8g2, displayTitle);
  int titleX = (60 - titleWidthMenu) / 2;
  if (titleX < 0) titleX = 0;
  u8g2_DrawRBox(&u8g2, 2, 27, titleWidthMenu + 4, 13, 4);
  u8g2_SetDrawColor(&u8g2, 0);
  u8g2_DrawStr(&u8g2, 4, 37, displayTitle);
  u8g2_SetDrawColor(&u8g2, 1);
  if (isEditing)
  {
    char valueText[12];
    snprintf(valueText, sizeof(valueText), "%d", *(currentMenu[selected].value));
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
    u8g2_DrawStr(&u8g2, 90, 37, valueText);
  }
  else
  {
    draw1pxScrollbar(selected);

    for (int i = 0; i < visibleLines; i++)
    {
      int item = firstVisible + i;
      if (item >= currentCount) break;

      int y = getMenuY(currentCount, i);
      const char* title = currentMenu[item].name;
      int titleWidth = u8g2_GetStrWidth(&u8g2, title);
      int titleX = (128 - titleWidth) / 2;

      titleWidth = u8g2_GetStrWidth(&u8g2, title);
      // titleX = titleWidthMenu + (128-titleWidthMenu - titleWidth) / 2;
      titleX = 120 - titleWidth;

      if (item == selected)
      {
        u8g2_DrawRFrame(&u8g2, titleX - 2, y - 10, titleWidth + 4, 13, 2);
        u8g2_DrawStr(&u8g2, titleX, y, title);
      }
      else
      {
        u8g2_DrawStr(&u8g2, titleX, y, title);
      }
    }
  }
  u8g2_SendBuffer(&u8g2);
}

void menuUp()
{
  if (pressedWithDebounce(GPIOB, UP_Pin, &lastState[upKey], &lastPressTime[upKey], &LongPressupLastPressTime))
  {
    menuUpdate = true;
    if (isEditing)
    {
      int* var = currentMenu[selected].value;
      if (*var < currentMenu[selected].maxValue)
      {
        (*var)++;
      }
    }
    else
    {
      selected--;
      if (selected < 0)
      {
        selected = currentCount - 1;
        if (currentCount > visibleLines)
{
    firstVisible = currentCount - visibleLines;
}
else
{
    firstVisible = 0;
}
      }
      if (selected < firstVisible)
      {
        firstVisible--;
      }
    }
  }

}

void menudown()
{
  if (pressedWithDebounce(GPIOB, DOWN_Pin, &lastState[downKey], &lastPressTime[downKey], &LongPressdownLastPressTime))
  {
    menuUpdate = true;

    if (isEditing)
    {
      int* var = currentMenu[selected].value;

      // If we are going down, we subtract.
      if (*var > currentMenu[selected].minValue)
      {
        (*var)--;
      }
    }  // <--- THIS BRACE WAS MISSING
    else
    {
      // Normal list scrolling
      selected++;

      if (selected >= currentCount)
      {
        selected = 0;
        firstVisible = 0;
      }
      else if (selected >= firstVisible + visibleLines)
      {
        // The parentheses here are critical!
        firstVisible = selected - (visibleLines - 1);
      }
    }
  }
}

void SelectButton()
{
  if (pressedWithDebounce(GPIOB, SELECT_Pin, &lastState[selectKey], &lastPressTime[selectKey], NULL))
  {
    menuUpdate = true;

    if (isEditing)
    {
      isEditing = false;  // Confirm edit and return to list
    }
    else
    {
      MenuNode SelectedItem = currentMenu[selected];

      if (SelectedItem.childCount > 0 && SelectedItem.children != NULL)
      {
        if (stackIndex < MAX_DEPTH)
        {
          menuStack[stackIndex] = currentMenu;
          countStack[stackIndex] = currentCount;
          selectedStack[stackIndex] = selected;
          firstVisibleStack[stackIndex] = firstVisible;
          titleStack[stackIndex] = currentTitle;
          stackIndex++;
        }
        if (SelectedItem.title != NULL)
          currentTitle = SelectedItem.title;
        else
          currentTitle = SelectedItem.name;

        currentMenu = SelectedItem.children;
        currentCount = SelectedItem.childCount;
        firstVisible = 0;
        selected = 0;
        if (SelectedItem.value!=NULL)
        {
          selected = *SelectedItem.value;
        }
        
      }
      else if (SelectedItem.action != NULL)
      {
        // 2. It's an executable action (e.g., wifiReset)
        SelectedItem.action();
        if (stackIndex > 0)
    {
      stackIndex--;
      currentMenu = menuStack[stackIndex];
      currentCount = countStack[stackIndex];
      selected = selectedStack[stackIndex];
      firstVisible = firstVisibleStack[stackIndex];
      currentTitle = titleStack[stackIndex];
    }
      }
      else if (SelectedItem.value != NULL)
      {
        isEditing = true;
      }
    }
  }
}

void BackButton()
{
  if (pressedWithDebounce(GPIOB, MODE_Pin, &lastState[modeKey], &lastPressTime[modeKey], NULL))
  {
    menuUpdate = true;
    if (isEditing)
    {
      isEditing = false;
    }
    else if (stackIndex > 0)
    {
      stackIndex--;
      currentMenu = menuStack[stackIndex];
      currentCount = countStack[stackIndex];
      selected = selectedStack[stackIndex];
      firstVisible = firstVisibleStack[stackIndex];
      currentTitle = titleStack[stackIndex];
    }
  }
}
bool invertColour = false;
void menushow(){
  menudown();
  menuUp();
  SelectButton();
  BackButton();
if(menuUpdate){
  drawMenu();
  menuUpdate = false;
}

}

