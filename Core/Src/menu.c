#include "main.h"
#include "newglobals.h"
#include "globalfunctions.h"
#include <stdint.h>
#include <stdio.h>

extern u8g2_t u8g2;

uint16_t settings1Bit = 0;
uint16_t settings2Bit = 0;
uint16_t settingsInt[6] = {0};

typedef enum {
  BOOL_TONE,
  BOOL_CONTRAST,
  BOOL_LANGUAGE,
  BOOL_TUBING_ALERT,
  BOOL_WIFI,
  BOOL_WIFI_RESET,
  BOOL_STOP_BIT,
  BOOL_LEVEL_TRIGGER
} BoolField;


typedef enum {
  OPT_RT_UNIT,
  OPT_PT_UNIT,
  OPT_REVERSE_ANGLE,
  OPT_EXT_CONTROL,
  OPT_BAUD_RATE,
  OPT_PARITY
} TwoBitField;

typedef enum {
  INT_TIME_SET,
  INT_PUMP_NO,
  INT_MAX_SPEED,
  INT_DISP_STOP,
  INT_ACC_TIME,
  INT_DEC_TIME
} IntField;

extern uint16_t timeSet;
extern uint16_t pumpNo;
extern uint16_t maxSpeed;
extern uint16_t accTime;
extern uint16_t decTime;
extern uint16_t dispStop;
extern uint16_t currRevAng;
extern uint16_t currtoneSet;
extern bool contrastflag;
extern uint16_t currentcontrast;
extern bool languageHindi;
extern uint16_t currentLanguage;
extern bool tubingAlertEnabled;
extern uint16_t currtubeAlert;
extern bool wifiEnabled;
extern uint16_t currwifiSet;
extern bool wifiResetFlag;
extern uint16_t currwifiReset;
extern uint16_t currextMenu;
extern bool stopBit;
extern uint16_t currentStopBitIndex;
extern uint16_t currentBaudIndex;
extern uint16_t currentParityIndex;
extern bool levelTrigHigh;
extern uint16_t currentLevelTrigIndex;

static uint16_t getPackedField(uint16_t data, uint8_t width, uint8_t index)
{
  return bitGet(data, (uint8_t)(width * index), width);
}

static void setPackedField(uint16_t *data,
                           uint8_t width,
                           uint8_t index,
                           uint16_t value)
{
  bitsSetValue(data, (uint8_t)(width * index), width, value);
}

static void setIntegerField(uint8_t field, uint16_t value)
{
  switch (field) {
    case INT_TIME_SET:
      bitsSetValue(&settingsInt[0], 0, 10, value);
      break;
    case INT_PUMP_NO:
      bitsSetValue(&settingsInt[1], 0, 10, value);
      break;
    case INT_MAX_SPEED:
      bitsSetValue(&settingsInt[2], 0, 13, value);
      break;
    case INT_DISP_STOP:
      bitsSetValue(&settingsInt[3], 0, 4, value);
      break;
    case INT_ACC_TIME:
      bitsSetValue(&settingsInt[4], 0, 10, value);
      break;
    case INT_DEC_TIME:
      bitsSetValue(&settingsInt[5], 0, 10, value);
      break;
    default:
      break;
  }
}

static const uint16_t integerScale[] = {1, 1, 1, 1, 1, 1};

static uint16_t scaleInteger(uint8_t field, uint16_t value)
{
  return (uint16_t)(value * integerScale[field]);
}

static uint16_t unscaleInteger(uint8_t field, uint16_t value)
{
  return (uint16_t)(value / integerScale[field]);
}

static void packIntegerValue(uint16_t *value)
{
  if (value == &timeSet)
    setIntegerField(INT_TIME_SET, scaleInteger(INT_TIME_SET, *value));
  else if (value == &pumpNo)
    setIntegerField(INT_PUMP_NO, scaleInteger(INT_PUMP_NO, *value));
  else if (value == &maxSpeed)
    setIntegerField(INT_MAX_SPEED, scaleInteger(INT_MAX_SPEED, *value));
  else if (value == &dispStop)
    setIntegerField(INT_DISP_STOP, scaleInteger(INT_DISP_STOP, *value));
  else if (value == &accTime)
    setIntegerField(INT_ACC_TIME, scaleInteger(INT_ACC_TIME, *value));
  else if (value == &decTime)
    setIntegerField(INT_DEC_TIME, scaleInteger(INT_DEC_TIME, *value));
}

void menuSettingsApply(const MenuSettings *settings)
{
  if (settings == NULL)
    return;

  settings1Bit = settings->settings1Bit;
  settings2Bit = settings->settings2Bit;

  for (uint8_t index = 0; index < 6; index++)
    settingsInt[index] = settings->settingsInt[index];

  toneproduce = getPackedField(settings1Bit, 1, BOOL_TONE) != 0;
  contrastflag = getPackedField(settings1Bit, 1, BOOL_CONTRAST) != 0;
  currentcontrast = contrastflag ? 0 : 1;
  languageHindi = getPackedField(settings1Bit, 1, BOOL_LANGUAGE) == 0;
  currentLanguage = languageHindi ? 0 : 1;
  tubingAlertEnabled = getPackedField(settings1Bit, 1, BOOL_TUBING_ALERT) != 0;
  currtubeAlert = tubingAlertEnabled ? 0 : 1;
  wifiEnabled = getPackedField(settings1Bit, 1, BOOL_WIFI) != 0;
  currwifiSet = wifiEnabled ? 0 : 1;
  wifiResetFlag = getPackedField(settings1Bit, 1, BOOL_WIFI_RESET) != 0;
  currwifiReset = wifiResetFlag ? 0 : 1;
  stopBit = getPackedField(settings1Bit, 1, BOOL_STOP_BIT) != 0;
  currentStopBitIndex = stopBit ? 1 : 0;
  levelTrigHigh = getPackedField(settings1Bit, 1, BOOL_LEVEL_TRIGGER) != 0;
  currentLevelTrigIndex = levelTrigHigh ? 0 : 1;

  currRTUnitSel = getPackedField(settings2Bit, 2, OPT_RT_UNIT);
  currentRunTimeUnit = (runtimeUnit)currRTUnitSel;
  currPTUnitSel = getPackedField(settings2Bit, 2, OPT_PT_UNIT);
  currentPauseTimeUnit = (runtimeUnit)currPTUnitSel;
  currRevAng = getPackedField(settings2Bit, 2, OPT_REVERSE_ANGLE);
  currextMenu = getPackedField(settings2Bit, 2, OPT_EXT_CONTROL);
  currentBaudIndex = getPackedField(settings2Bit, 2, OPT_BAUD_RATE);
  currentParityIndex = getPackedField(settings2Bit, 2, OPT_PARITY);

  reverseAngle = (uint16_t[]){0, 180, 360, 720}[currRevAng];
  timeSet = unscaleInteger(INT_TIME_SET, bitGet(settingsInt[0], 0, 10));
  pumpNo = unscaleInteger(INT_PUMP_NO, bitGet(settingsInt[1], 0, 10));
  maxSpeed = unscaleInteger(INT_MAX_SPEED, bitGet(settingsInt[2], 0, 13));
  dispStop = unscaleInteger(INT_DISP_STOP, bitGet(settingsInt[3], 0, 4));
  accTime = unscaleInteger(INT_ACC_TIME, bitGet(settingsInt[4], 0, 10));
  decTime = unscaleInteger(INT_DEC_TIME, bitGet(settingsInt[5], 0, 10));
}

void menuSettingsCollect(MenuSettings *settings)
{
  if (settings == NULL)
    return;

  packIntegerValue(&timeSet);
  packIntegerValue(&pumpNo);
  packIntegerValue(&maxSpeed);
  packIntegerValue(&dispStop);
  packIntegerValue(&accTime);
  packIntegerValue(&decTime);

  settings->settings1Bit = settings1Bit;
  settings->settings2Bit = settings2Bit;

  for (uint8_t index = 0; index < 6; index++)
    settings->settingsInt[index] = settingsInt[index];
}



uint16_t timeSet = 0;
uint16_t cycles = 0;
uint16_t maxSpeed = 6000;
uint16_t pumpNo = 0;
uint16_t accTime=0;
uint16_t decTime = 0;
uint16_t dispStop = 0;
uint16_t selected = 0;
uint16_t firstVisible = 0;
uint16_t parentIndex = 0;
bool menuUpdate = true;
const int lineHeight = 12;
const int visibleLines = 5;


// Action Prototypes

// void primePump() {};
// void calibration() {};
// void diagnostics() {};
// void wifiReset() {};
// void factoryReset() {};
// void firmwareInfo() {};
// void aboutInfo() {};

uint16_t reverseAngle = 0;
uint16_t currRevAng = 0;
uint16_t currtoneSet = 0;
bool needsSave = false;
bool contrastflag = true;
uint16_t currentcontrast = 0;
bool languageHindi = false;
uint16_t currentLanguage = 0;
bool tubingAlertEnabled = false;
uint16_t currtubeAlert = 0;
bool wifiEnabled = false;
uint16_t currwifiSet = 0;
bool wifiResetFlag = false;
uint16_t currwifiReset = 0;
uint16_t currextMenu = 0;
bool stopBit = false;
uint16_t currentStopBitIndex = 0;
uint16_t currentBaudIndex = 0;
uint16_t currentParityIndex = 0;
bool levelTrigHigh = false;
uint16_t currentLevelTrigIndex = 0;
uint16_t currPTUnitSel = 0;
uint16_t currRTUnitSel = 0;
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

//  // menu settings 
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
  const MenuNode* children;
  uint8_t childCount;
  void (*action)();
  uint16_t* value;
  uint16_t minValue;
  uint16_t maxValue;
};

extern const MenuNode *currentMenu;
extern uint16_t selected;

static void setBoolSetting(void)
{
  const MenuNode *item = &currentMenu[selected];
  uint16_t option = item->minValue;
  uint8_t index = (uint8_t)item->maxValue;
  bool enabled = option == 0;

  setPackedField(&settings1Bit, 1, index, enabled);

  switch (index) {
    case BOOL_TONE: toneproduce = enabled; currtoneSet = option; break;
    case BOOL_CONTRAST: contrastflag = enabled; currentcontrast = option; break;
    case BOOL_LANGUAGE: languageHindi = enabled; currentLanguage = option; break;
    case BOOL_TUBING_ALERT: tubingAlertEnabled = enabled; currtubeAlert = option; break;
    case BOOL_WIFI: wifiEnabled = enabled; currwifiSet = option; break;
    case BOOL_WIFI_RESET: wifiResetFlag = enabled; currwifiReset = option; break;
    case BOOL_STOP_BIT: stopBit = enabled; currentStopBitIndex = option; break;
    case BOOL_LEVEL_TRIGGER: levelTrigHigh = enabled; currentLevelTrigIndex = option; break;
    default: break;
  }

  needsSave = true;
}

static void setTwoBitSetting(void)
{
  const MenuNode *item = &currentMenu[selected];
  uint16_t option = item->minValue;
  uint8_t index = (uint8_t)item->maxValue;

  setPackedField(&settings2Bit, 2, index, option);

  switch (index) {
    case OPT_RT_UNIT: currRTUnitSel = option; currentRunTimeUnit = (runtimeUnit)option; break;
    case OPT_PT_UNIT: currPTUnitSel = option; currentPauseTimeUnit = (runtimeUnit)option; break;
    case OPT_REVERSE_ANGLE:
      currRevAng = option;
      reverseAngle = (uint16_t[]){0, 180, 360, 720}[option];
      break;
    case OPT_EXT_CONTROL: currextMenu = option; break;
    case OPT_BAUD_RATE: currentBaudIndex = option; break;
    case OPT_PARITY: currentParityIndex = option; break;
    default: break;
  }

  needsSave = true;
}

// Functional
const MenuNode language[] = {
  { "HINDI", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_LANGUAGE },
  { "ENGLISH", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_LANGUAGE },
};
const MenuNode contrastMenu[] = {
  { "LIGHT", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_CONTRAST },
  { "DARK", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_CONTRAST },
};
bool  toneproduce = false;

const MenuNode toneSet[] = {
  { "ENABLE", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_TONE },
  { "DISABLE", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_TONE },
};

const MenuNode generalMenu[] = {
  { "TIMESET", NULL, NULL, 0, NULL, &timeSet, 0, 999 },
  { "LANGUAGE", NULL, language, 2, NULL, &currentLanguage, 0, 0 },
  { "CONTRAST", NULL, contrastMenu, 2, NULL, &currentcontrast, 0, 0 },
  { "TONE SET", NULL, toneSet, 2, NULL, &currtoneSet, 0, 0 },
}; 
// enum runtimeUnit{
//     days, hours , minutes, seconds 
// };
// runtimeUnit currentRunTimeUnit = seconds ;


const MenuNode rtUnitMenu[] = {
  { "DAYS", NULL, NULL, 0, setTwoBitSetting, NULL, 0, OPT_RT_UNIT },
  { "HOURS", NULL, NULL, 0, setTwoBitSetting, NULL, 1, OPT_RT_UNIT },
  { "MINUTES", NULL, NULL, 0, setTwoBitSetting, NULL, 2, OPT_RT_UNIT },
  { "SECONDS", NULL, NULL, 0, setTwoBitSetting, NULL, 3, OPT_RT_UNIT },
};

// runtimeUnit currentPauseTimeUnit = seconds ;

const MenuNode pauseTimeUnitMenu[] = {
  { "DAYS", NULL, NULL, 0, setTwoBitSetting, NULL, 0, OPT_PT_UNIT },
  { "HOURS", NULL, NULL, 0, setTwoBitSetting, NULL, 1, OPT_PT_UNIT },
  { "MINUTES", NULL, NULL, 0, setTwoBitSetting, NULL, 2, OPT_PT_UNIT },
  { "SECONDS", NULL, NULL, 0, setTwoBitSetting, NULL, 3, OPT_PT_UNIT },
};

const MenuNode revAngMenu[] = {
  { "0", NULL, NULL, 0, setTwoBitSetting, NULL, 0, OPT_REVERSE_ANGLE },
  { "180", NULL, NULL, 0, setTwoBitSetting, NULL, 1, OPT_REVERSE_ANGLE },
  { "360", NULL, NULL, 0, setTwoBitSetting, NULL, 2, OPT_REVERSE_ANGLE },
  { "720", NULL, NULL, 0, setTwoBitSetting, NULL, 3, OPT_REVERSE_ANGLE },
};

const MenuNode tubeAlertMenu[] = {
  { "YES", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_TUBING_ALERT },
  { "NO", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_TUBING_ALERT },
};

const MenuNode funtionalMenu[] = {
  { "RT UNIT", NULL, rtUnitMenu, 4, NULL, &currRTUnitSel, 0, 0 },
  { "PT UNIT", NULL, pauseTimeUnitMenu, 4, NULL, &currPTUnitSel, 0, 0 },
  { "REVERSE ANGLE", "REV ANGLE", revAngMenu, 4 , NULL, &currRevAng, 0, 0 },
  { "PUMP NO.", NULL, NULL, 0, NULL, &pumpNo, 0, 999 },
  { "MAX SPEED", NULL, NULL, 0, NULL, &maxSpeed, 0, 999 },
  { "TUBING ALERT", "TUBE ALERT", tubeAlertMenu, 2, NULL, &currtubeAlert, 0, 0 },
};

const MenuNode wifiMenu[] = {
  { "ENABLE", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_WIFI },
  { "DISABLED", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_WIFI },
};

const MenuNode wifiResetMenu[] = {
  { "YES", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_WIFI_RESET },
  { "NO", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_WIFI_RESET },
};

const MenuNode extMenu[] = {
  { "CURRENT", NULL, NULL, 0, setTwoBitSetting, NULL, 0, OPT_EXT_CONTROL },
  { "VOLT 0-5V", NULL, NULL, 0, setTwoBitSetting, NULL, 1, OPT_EXT_CONTROL },
  { "VOLT 0-10V", NULL, NULL, 0, setTwoBitSetting, NULL, 2, OPT_EXT_CONTROL },
};

const MenuNode softwareMenu[] = {
  { "BOOT V3.1.0", NULL, NULL, 0, NULL, NULL, 0, 0 },
  { "USER v3.2.3", NULL, NULL, 0, NULL, NULL, 0, 0 },
};

const MenuNode ExternalMenu[] = {
  { "WIFI SETTINGS", "WIFI SET", wifiMenu, 2, NULL, &currwifiSet, 0, 0 },
  { "WIFI RST", NULL, wifiResetMenu, 2, NULL, &currwifiReset, 0, 0 },
  { "EXT CONTROLS", "EXT CONT", extMenu, 3, NULL, &currextMenu, 0, 0 },
  { "SOFTWARE", NULL, softwareMenu, 2, NULL, NULL, 0, 0 },
};


const MenuNode baudMenu[] = {
  { "4800", NULL, NULL, 0, setTwoBitSetting, NULL, 0, OPT_BAUD_RATE },
  { "9800", NULL, NULL, 0, setTwoBitSetting, NULL, 1, OPT_BAUD_RATE },
  { "19200", NULL, NULL, 0, setTwoBitSetting, NULL, 2, OPT_BAUD_RATE },
  { "38400", NULL, NULL, 0, setTwoBitSetting, NULL, 3, OPT_BAUD_RATE },
};


const MenuNode parityMenu[] = {
  { "NONE", NULL, NULL, 0, setTwoBitSetting, NULL, 0, OPT_PARITY },
  { "EVEN", NULL, NULL, 0, setTwoBitSetting, NULL, 1, OPT_PARITY },
  { "ODD", NULL, NULL, 0, setTwoBitSetting, NULL, 2, OPT_PARITY },
};

const MenuNode stopBitMenu[] = {
  { "1", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_STOP_BIT },
  { "0", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_STOP_BIT },
};

const MenuNode leveltrigmenu[] = {
  { "HIGH", NULL, NULL, 0, setBoolSetting, NULL, 0, BOOL_LEVEL_TRIGGER },
  { "LOW", NULL, NULL, 0, setBoolSetting, NULL, 1, BOOL_LEVEL_TRIGGER },
};

const MenuNode systemMenu[] = {
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

const MenuNode mainMenu[] = {
  { "GENERAL", "GEN", generalMenu, 4, NULL, NULL, 0, 0 },
  { "FUNCTIONAL", "FUNC", funtionalMenu, 6, NULL, NULL, 0, 0 },
  { "SYSTEM", "SYS", systemMenu, 9, NULL, NULL, 0, 0 },
  { "EXTERNAL", "EXT", ExternalMenu, 4, NULL, NULL, 0, 0 },
};

const MenuNode* currentMenu = mainMenu;  // Pointer to the active menu array
int currentCount = 4;              // How many items in the active array (mainMenu has 4)

enum { MAX_DEPTH = 5 };
const char* currentTitle = "MENU";
const MenuNode* menuStack[MAX_DEPTH];
int countStack[MAX_DEPTH];
int selectedStack[MAX_DEPTH];
int firstVisibleStack[MAX_DEPTH];
const char* titleStack[MAX_DEPTH];
int stackIndex = 0;  // Current depth level


GPIO_TypeDef *activeHeldPort = NULL;
uint16_t activeHeldPin = 0;
unsigned long longPressTime = 500; // ms to wait before auto-repeat starts
unsigned long debounceDelay = 50;   // ms to wait for debounce
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
      uint16_t* var = currentMenu[selected].value;
      if (*var < currentMenu[selected].maxValue)
      {
        (*var)++;
        packIntegerValue(var);
        needsSave = true;
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
      uint16_t* var = currentMenu[selected].value;

      // If we are going down, we subtract.
      if (*var > currentMenu[selected].minValue)
      {
        (*var)--;
        packIntegerValue(var);
        needsSave = true;
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
      const MenuNode SelectedItem = currentMenu[selected];

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

