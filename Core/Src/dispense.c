#include "cmsis_os2.h"
#include "main.h"
#include<newglobals.h>
#include<globalfunctions.h>

float runTime = 10.0f;
float pauseTime = 2.0f;
int dispCycles = 2 ;
float rpm = 100.0f;
bool modeUpdate = true;
// uint8_t modeCounter=0;
//calculate the dispense time in milliseconds based on the current unit
int32_t dispRuntime(void)
{
    if (currentRunTimeUnit == seconds) return (int32_t)(runTime * 10.0f);
    if (currentRunTimeUnit == minutes) return (int32_t)(runTime * 60.0f * 10.0f);
    if (currentRunTimeUnit == hours)   return (int32_t)(runTime * 3600.0f * 10.0f);
    if (currentRunTimeUnit == days)    return (int32_t)(runTime * 86400.0f * 10.0f);

    return (int32_t)(runTime * 10.0f);
}
//calculate the pause time in milliseconds based on the current unit
int32_t dispPauseTime(void){
  if(currentPauseTimeUnit==seconds) return (int32_t) (pauseTime *10);
  else if(currentPauseTimeUnit==minutes) return (int32_t) (pauseTime*60*10);
  else if(currentPauseTimeUnit==hours) return (int32_t) (pauseTime*3600*10);
  else if(currentPauseTimeUnit==days) return (int32_t) (pauseTime*86400*10);
  else return (int32_t) (pauseTime *10);
}
//enum to represent the type of value in the dispense menu
typedef enum{
  VALUE_INT,
  VALUE_FLOAT
} ValueType;

//struct to represent a menu item in the dispense menu
typedef struct {
  const char* name;
  ValueType type;
  void* value;
  float minValue;
  float maxValue;
} MenuItem;

MenuItem tMenu[] = {
  { "RT",    VALUE_FLOAT, &runTime,    0.1f, 999.9f },
  { "PT",    VALUE_FLOAT, &pauseTime,   0.1f, 999.9f },
  { "Cycle", VALUE_INT,   &dispCycles,  0.0f, 999.0f }
};

MenuItem homeMenu[] = {
  { "RPM",  VALUE_FLOAT, &rpm,  MIN_RPM, MAX_RPM },
  { "TIME", VALUE_INT,   NULL, 0.0f, 0.0f }
};




// ---- navigation state ----
// current screen, menu, and selection state
MenuScreen  currentScreen   = SCREEN_HOME;
MenuItem*   currentMenuMain = homeMenu;
int         currentCountMain = 2;
int         visibleLinesMain = 3;
int         selectedMain     = 0;
int         firstVisibleMain = 0;
bool        dispTimeEdit     = false;
bool        menuUpdateMain   = true;
bool isEditingMain = false;
bool rpmEditing = false;

unsigned long PumpRunningTime =0;    // logs the time when the pump started running
unsigned long PauseStartTime =0;     // logs the time when the pump paused 
bool DispenseRunning = false ;
bool pumpRunning = false ;
int CycleElapsed = 0 ;
unsigned long lastStartStopPressedTime = 0;
bool startStopLastState = 1;
int indexdrawmenu = 0;


unsigned long lastdispCycleRunningScreenUpdateTime = 0;
// unsigned long LongPressupLastPressTime =0;
// unsigned long LongPressdownLastPressTime =0;
unsigned long LongPresslastModeKEyPressedTime = 0;

bool startelapsedupdate = false ;
float timeElapsedSeconds = 0;
int64_t nominalElapsedSoFar = 0;

// enum pumpStateDisp {
//   pumpRunningDisp,
//   pumpPausedDisp
// };

 // global, start paused


uint32_t dispenseSettings = 0; 
bool dispNeedsSave = false;
void savedispenseSettings() {
  if (!dispNeedsSave) return;
  dispNeedsSave = false;
  bitsSet(dispenseSettings, 0, 10, runTime);
  bitsSet(dispenseSettings, 10, 10, pauseTime);
  bitsSet(dispenseSettings, 20, 10, dispCycles);
  // prefs.begin(NVS_NS, false);
  // prefs.putBytes("dispset", &dispenseSettings, sizeof(dispenseSettings));
  // prefs.end();
  // Serial.println(dispenseSettings, BIN);
  //  Serial.println(dispenseSettings);
}


void loadDispenseSettings() {
  // prefs.begin(NVS_NS, true);
  // prefs.getBytes("dispset", &dispenseSettings, sizeof(dispenseSettings));
  // prefs.end();

}

void resetMenuStateDisp() {
  currentScreen    = SCREEN_HOME;
  currentMenuMain  = homeMenu;
  currentCountMain = 2;
  selectedMain     = 0;
  firstVisibleMain = 0;
  isEditingMain    = true;
  dispTimeEdit     = false;
  modeUpdate       = true;
  menuUpdateMain   = true;
rpmEditing = false;
}



void DispCycle(){

  if (modeCounter==0 && pressedWithDebounce(GPIOB, START_Pin, &lastState[startStop], &lastPressTime[startStop] , NULL )){
    if(maxSpeedRunning==true){
      _SMPP_stop();
      maxSpeedRunning=false;
    }
    else if (DispenseRunning==false)
    { 
      //menu state reset 
      savedispenseSettings();
      u8g2_SetDrawColor(&u8g2, 0);

    u8g2_DrawBox(&u8g2, 65,3,60,22);

    u8g2_SetDrawColor(&u8g2, 1);
          resetMenuStateDisp();
          modeUpdate=true;
      rpmUpdateScreen();
   
     DispenseRunning = true;
     pumpRunning =true ;
    CycleElapsed = dispCycles;
    PumpRunningTime = HAL_GetTick();
    startelapsedupdate = true;
     if (dispCycles!=0)  {
      nominalElapsedSoFar = 0;
      startLoading(boarderTaskTime());}
    
    currentPumpState = pumpRunningDisp;  
        _SMPP_setDir(clockwise);
         _SMPP_setRPM(rpm); 

    
                     
    }


    else {
     DispenseRunning = false ;
     clearLoadingBar();
     if(!maxSpeedRunning) 
     _SMPP_stop();

    if(pumpRunning && reverseAngle > 0){
      while(_SMPP_isStopping())
        osDelay(1);
      _SMPP_setDir(!clockwise);
      _SMPP_runReverseAngle(reverseAngle,rpm);
      _SMPP_setDir(clockwise);
    }
           pumpRunning = false ;

             unsigned long timeout = HAL_GetTick();
        while(_SMPP_isRunning()){
          if(HAL_GetTick()-timeout>1500) {
            break;
          } 
        }
     
     startelapsedupdate = false ;
     menuUpdateMain = true ;
  
  }
}
   if(DispenseRunning==false || maxSpeedRunning==true){
        // clearLoadingBar();
    return;
   }


   if (pumpRunning)
{ 
    // Pump is running
    if (HAL_GetTick() - PumpRunningTime >= dispRuntime())
    {      
              pumpRunning = false;
        PauseStartTime = HAL_GetTick();
        
        currentPumpState = pumpPausedDisp;  
      _SMPP_stop();
        while(_SMPP_isStopping())
          osDelay(1);

        if(reverseAngle > 0){
          _SMPP_setDir(!clockwise);
          _SMPP_runReverseAngle(reverseAngle,rpm);
          _SMPP_setDir(clockwise);
          currentPumpState=pumpStoppingDisp;
        }
        
  // when paused
      if (dispCycles != 0) {
    nominalElapsedSoFar += dispRuntime();
    resyncLoadingBar(nominalElapsedSoFar);

        if(CycleElapsed==1){
            currentPumpState = pumpPausedDisp; 
    DispenseRunning = false;
    pumpRunning = false;
    _SMPP_stop();
    unsigned long timeout = HAL_GetTick();
        while(_SMPP_isRunning()){
          if(HAL_GetTick()-timeout>1500) {
            break;
          }
        }
    
     clearLoadingBar();
     menuUpdateMain = true;
     startelapsedupdate = false;
       return;
        
     
    }
   
}

}
}
else
{
    // Pump is paused
    if (( CycleElapsed > 0 || dispCycles==0) &&
        HAL_GetTick() - PauseStartTime >= dispPauseTime())
    {   currentPumpState = pumpRunningDisp; 
        pumpRunning = true;
        if(dispCycles!=0) CycleElapsed--;
        PumpRunningTime = HAL_GetTick();
        _SMPP_setRPM(rpm);
        if (dispCycles != 0) {
            nominalElapsedSoFar += dispPauseTime();
            resyncLoadingBar(nominalElapsedSoFar);
        }
                 
    }
}


osDelay(1);
}

int getMenuYMain(uint8_t totalItems, uint8_t index) {
  static const uint8_t yPos[][3] = {
    { 54 },
    { 22, 54 },
    { 17, 38, 59 },
  };
  if (totalItems < 1) totalItems = 1;
  else if (totalItems > 3) totalItems = 3;
  return yPos[totalItems - 1][index];
}



void drawMenuRow(const char* title, MenuItem* item, int y, bool selected, bool editing, bool showUnit) {
  bool hasValue = (item->value != NULL);
  char buf[10];
  if (hasValue) {
    if (item->type == VALUE_FLOAT) {
      float value = *(float*)item->value;

      float2s1(value, buf, sizeof(buf));
      // snprintf(buf, sizeof(buf), "%.1f", value);
    } else {
      int value = *(int*)item->value;
      snprintf(buf, sizeof(buf), "%d", value);
    }
  }

  // Resolve unit char up front so we can measure it
  const char* unitStr = "";
  if (hasValue && showUnit) {
    runtimeUnit unit = (indexdrawmenu == 0) ? currentRunTimeUnit : currentPauseTimeUnit;
    switch (unit) {
      case seconds: unitStr = "s"; break;
      case minutes: unitStr = "m"; break;
      case hours:   unitStr = "h"; break;
      case days:    unitStr = "d"; break;
    }
  }

  // Measure every piece
  int titleW = u8g2_GetStrWidth(&u8g2, title);
  int colonW = hasValue ? u8g2_GetStrWidth(&u8g2, ":") : 0;
  int valW   = hasValue ? u8g2_GetStrWidth(&u8g2, buf) : 0;
  int unitW  = (hasValue && showUnit) ? u8g2_GetStrWidth(&u8g2, unitStr) : 0;

  const int GAP = 1;
  int totalW = titleW
             + (hasValue ? GAP + colonW + GAP + valW : 0)
             + (hasValue && showUnit ? GAP + unitW : 0);


  int x = RIGHT_EDGE - totalW;

  if (selected) {
    const int boxPad = 2;
    if (editing) {
      u8g2_DrawRBox(&u8g2, x - boxPad, y - 9, totalW + boxPad * 2, 11, 2);
      u8g2_SetDrawColor(&u8g2, 0);
    } else {
      u8g2_DrawRFrame(&u8g2, x - boxPad, y - 9, totalW + boxPad * 2, 11, 2);
    }
  }

u8g2_DrawStr(&u8g2, x, y, title);
x += titleW;


  if (hasValue) {
    x += GAP;
u8g2_DrawStr(&u8g2, x, y, ":");
    x += colonW + GAP;
u8g2_DrawStr(&u8g2, x, y, buf);    
x += valW;

    if (showUnit) {
      x += GAP;
      u8g2_DrawStr(&u8g2, x, y, unitStr);
    }
  }

  if (selected && editing) u8g2_SetDrawColor(&u8g2, 1);
}
// ============================================================
// Draw the whole menu
// ============================================================
void drawMenuMain() {
  if (DispenseRunning) return;

  // u8g2.setFont(u8g2_font_profont11_tr);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
  u8g2_SetDrawColor(&u8g2, 0);
  u8g2_DrawBox(&u8g2, 66, 4, 58, 58);
  //  u8g2.drawBox(62,4,4,34);
  u8g2_SetDrawColor(&u8g2, 1);

  if (currentScreen == SCREEN_TIME) {
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, 29, 5, 32, 15);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawStr(&u8g2, 29, 20, "TIME");
  }

  for (int i = 0; i < visibleLinesMain; i++) {
    int item = firstVisibleMain + i;
    if (item >= currentCountMain) break;

    int y = getMenuYMain(currentCountMain, i);
    bool selected = (item == selectedMain);
    bool showUnit = (currentScreen == SCREEN_TIME && dispTimeEdit);
    indexdrawmenu = i;
    if(currentScreen == SCREEN_TIME && i==2) {
      showUnit=false;
    }

    drawMenuRow(currentMenuMain[item].name, &currentMenuMain[item],
                y, selected, isEditingMain, showUnit);
  }

  u8g2_SendBuffer(&u8g2);
}


void menuUpMain() {
  if (!pressedWithDebounce(GPIOB,UP_Pin , &lastState[upKey], &lastPressTime[upKey], &LongPressupLastPressTime)) return;
  menuUpdateMain = true;

  if (isEditingMain) {
    dispNeedsSave = true;
    MenuItem *item = &currentMenuMain[selectedMain];
    if (item->type == VALUE_FLOAT) {
    float* var = (float*)item->value;

    float step = 0.1f;
    if (HAL_GetTick() - lastPressTime[upKey] > 8000) step = 10.0f;
    else if (HAL_GetTick() - lastPressTime[upKey] > 2000) step = 1.0f;

    *var = constrain(*var + step, item->minValue, item->maxValue);
  } else {
    int* var = (int*)item->value;

    int step = 1;
    if (HAL_GetTick() - lastPressTime[upKey] > 8000) step = 10;
    else if (HAL_GetTick() - lastPressTime[upKey] > 2000) step = 5;

    *var = constrain(*var + step, (int)item->minValue, (int)item->maxValue);
  }

  } else {
    selectedMain--;
    if (selectedMain < 0) {
      selectedMain = currentCountMain - 1;
      firstVisibleMain = maxInt(0, currentCountMain - visibleLinesMain);
    }
    if (selectedMain < firstVisibleMain) firstVisibleMain--;
  }
}

// ============================================================
// Down button
// ============================================================
void menudownMain() {
  if (!pressedWithDebounce(GPIOB, DOWN_Pin , &lastState[downKey], &lastPressTime[downKey], &LongPressdownLastPressTime)) return;
  menuUpdateMain = true;

  if (isEditingMain) {
    dispNeedsSave = true;
    MenuItem *item = &currentMenuMain[selectedMain];
    if (item->type == VALUE_FLOAT) {
  float* var = (float*)item->value;
  float step = 0.1f;
  if (HAL_GetTick() - lastPressTime[downKey] > 8000) step = 10.0f;
  else if (HAL_GetTick() - lastPressTime[downKey] > 2000) step = 1.0f;

  *var = constrain(*var - step, item->minValue, item->maxValue);
} else {
  int* var = (int*)item->value;
  int step = 1;
  if (HAL_GetTick() - lastPressTime[downKey] > 8000) step = 10;
  else if (HAL_GetTick() - lastPressTime[downKey] > 2000) step = 5;

  *var = constrain(*var - step, (int)item->minValue, (int)item->maxValue);
}
  } else {
    selectedMain++;
    if (selectedMain >= currentCountMain) {
      selectedMain = 0;
      firstVisibleMain = 0;
    } else if (selectedMain >= firstVisibleMain + visibleLinesMain) {
      firstVisibleMain = selectedMain - (visibleLinesMain - 1);
    }
  }
}

// ============================================================
// Select button — confirm edit, open TIME screen, or start editing
// ============================================================
void SelectButtonMain() {
  if (!pressedWithDebounce(GPIOB, SELECT_Pin , &lastState[selectKey], &lastPressTime[selectKey] , NULL) ) return;
  menuUpdateMain = true;

  if (isEditingMain) {
    isEditingMain = false;
    savedispenseSettings();
    return;
  }

  if (currentScreen == SCREEN_HOME && selectedMain == 1) {   // "TIME" row
    currentScreen    = SCREEN_TIME;
    currentMenuMain  = tMenu;
    currentCountMain = 3;
    selectedMain     = 0;
    firstVisibleMain = 0;
    dispTimeEdit     = true;
  } else if (currentMenuMain[selectedMain].value != NULL) {
    isEditingMain = true;
  }
}

// ============================================================
// Back button — cancel edit or leave TIME screen
// ============================================================
void BackButtonMain() {

  if (currentScreen != SCREEN_TIME && !isEditingMain) return;
  if (!pressedWithDebounce(GPIOB , MODE_Pin, &lastState[modeKey], &lastPressTime[modeKey] , NULL)) return;
  menuUpdateMain = true;
  modeUpdate = false;
  if (isEditingMain) {
    isEditingMain = false;
    savedispenseSettings();
  } 
  else if (currentScreen == SCREEN_TIME) {
    modeUpdate = true;
    dispTimeEdit     = false;
    currentScreen    = SCREEN_HOME;
    currentMenuMain  = homeMenu;
    currentCountMain = 2;
selectedMain = 1;
isEditingMain = false; // land back on TIME row
    firstVisibleMain = 0;
  }
}









