#include "main.h"
#include <newglobals.h>
#include <globalfunctions.h>

//initializer 
pumpStateDisp currentPumpState = pumpStoppedDisp;
int rpmext = 0;

// const char* modesarr[] = {
//   "DISP ", "INT", "EXTV", "EXTI", "LEV1", "LEV2",
// }; 
// Converts a float to "X.XX" with proper rounding, no dtoa/mprec needed.
// 1-decimal float-to-string, no dtoa/mprec/double needed
 void float2s1(uint16_t val, char *buf, size_t buflen) {
if (val < 1000) {
    // Below 100 RPM: one decimal place
    snprintf(buf, buflen, "%u.%u",
             val / 10u, val % 10u);
} else {
    // 100 RPM and above: integer
    snprintf(buf, buflen, "%u",
             val / 10u);
}
}


void menuButtonHandle() {
  
  if (DispenseRunning==false && maxSpeedRunning==false && pumpRunning == false && pressedWithDebounce(GPIOB, MENU_Pin, &lastState[menuKey], &lastPressTime[menuKey], NULL)  ) {
    changeMenu = !changeMenu;
    // nextLoop = !nextLoop ;
    clearLoadingBar();

    u8g2_ClearBuffer(&u8g2);
    if (changeMenu) {
      menuUpdate = true;  
    }
    if(!changeMenu){
      // if(needsSave){
      // settingsSave() ;
      // needsSave = false;
      // }
      
      modeUpdate=true;
      menuUpdateMain=true;
      homeScrenUpdate = true ;
    }
    
  }
}
// void modeButtonHandle(){
//   if(intRunning == false && dispTimeEdit == false && DispenseRunning==false && maxSpeedRunning==false && 
//     pressedWithDebounce( GPIOB, MODE_Pin, &lastState[modeKey],&lastPressTime[modeKey] , &LongPresslastModeKEyPressedTime )){
//     modeCounter ++;
//     modeUpdate=true;
//     if (modeCounter>5){
//       modeCounter = 0;
//     }
//     if(modeCounter==0){
//       menuUpdateMain = true ;
//       resetMenuStateDisp() ;
//       drawMenuMain();
      
//     }
//   }
   

//     if(modeUpdate && currentScreen != SCREEN_TIME){
      
//   u8g2_SetDrawColor(&u8g2, 0);
//   u8g2_DrawBox( &u8g2, 29, 5, 32, 15);
//   u8g2_SetDrawColor(&u8g2, 1);
//   u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
//   u8g2_DrawStr(&u8g2 , 29, 20, modesarr[modeCounter]);
//   if(DispenseRunning){
//       u8g2_SetDrawColor(&u8g2, 0);

//       u8g2_DrawBox(&u8g2, 65,3,60,22);

//       u8g2_SetDrawColor(&u8g2, 1);
//   }

//   }
// }
 
void rpmUpdateScreen() {
  u8g2_SetDrawColor(&u8g2, 0);
  u8g2_DrawBox(&u8g2, 66, 4, 58, 20);
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);

  char bufRPM[12];
  bool isExt = (modeCounter == 2);

  if (!isExt) {
    float2s1(rpm, bufRPM, sizeof(bufRPM));
} else {
    float2s1(rpmext, bufRPM, sizeof(bufRPM));
}

  const int gap = 1;
  int titleWidth = u8g2_GetStrWidth(&u8g2, "RPM");
  int colonWidth = u8g2_GetStrWidth(&u8g2, ":");
  int valueWidth = u8g2_GetStrWidth(&u8g2, bufRPM);

  int totalWidth = titleWidth + gap + colonWidth + gap + valueWidth;
  int x = RIGHT_EDGE - totalWidth;
  int boxPad = 2;

      u8g2_DrawRBox(&u8g2, x - boxPad, 13,
                    totalWidth + boxPad * 2, 11, 2);

      // Draw text in inverse color inside the box.
      u8g2_SetDrawColor(&u8g2, 0);

      u8g2_DrawStr(&u8g2, x, 22, "RPM");
  // u8g2.setCursor(x, 22);
  // u8g2.print("RPM");

  x += titleWidth + gap;
  u8g2_DrawStr(&u8g2, x, 22, ":");
  // u8g2.setCursor(x, 22);
  // u8g2.print(":");

  x += colonWidth + gap;
  u8g2_DrawStr(&u8g2, x, 22, bufRPM);
  u8g2_SetDrawColor(&u8g2,1);
}
void rpmUpdateButtonsHandle(){

if (DispenseRunning == true || modeCounter==1 || modeCounter==4 || modeCounter==5 ){ 
      if (pressedWithDebounce(GPIOB , UP_Pin, &lastState[upKey], &lastPressTime[upKey] ,&LongPressupLastPressTime))
  {
     modeUpdate = true;
    uint16_t step = rpm < 1000 ? 1 : 10;
  if (HAL_GetTick() - lastPressTime[upKey] > 8000) step = 100;
  else if (HAL_GetTick() - lastPressTime[upKey] > 2500) step = 10;

  rpm = (uint16_t)constrainInt32(rpm + step, MIN_RPM10, MAX_RPM10);
     if (_SMPP_isRunning() && maxSpeedRunning==false)
      {
        _SMPP_setRPM10(rpm);
      }
    }

if (pressedWithDebounce(GPIOB, DOWN_Pin, &lastState[downKey], &lastPressTime[downKey] ,&LongPressdownLastPressTime ))
  { 
    modeUpdate = true ;
    uint16_t step = rpm <= 1000 ? 1 : 10;
  if (HAL_GetTick() - lastPressTime[downKey] > 8000) step = 100;
  else if (HAL_GetTick() - lastPressTime[downKey] > 2500) step = 10;

  rpm = (uint16_t)constrainInt32(rpm - step, MIN_RPM10, MAX_RPM10);
       if (_SMPP_isRunning() && maxSpeedRunning==false)
      {
        _SMPP_setRPM10(rpm);
      
    }

    }

if(modeUpdate)rpmUpdateScreen();
modeUpdate = false;
}
}


  //dispense .ccp

// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_U8G2_R2, U8X8_PIN_NONE, U8X8_PIN_NONE);  // i2c





//partial display updates
// void updateDisplayAreaPixel(int x, int y, int w, int h)
// {
//     // Convert pixel rectangle to 8x8 tile rectangle
//     int tx = x / 8;
//     int ty = y / 8;

//     int tx2 = (x + w - 1) / 8;
//     int ty2 = (y + h - 1) / 8;

//     int tw = tx2 - tx + 1;
//     int th = ty2 - ty + 1;

//     u8g2_updateDisplayArea(tx, ty, tw, th);
// }



bool maxSpeedRunning = false;
bool lastmaxSpeedKeyState =  true;
unsigned long lastMaxSpeedKeyPressedTime = 0;
uint16_t portSnapshot = 0xFFFF;   
void maxSpeedKeyHandle(){
  if(modeCounter!=5 && modeCounter!=4 && modeCounter!=2 && DispenseRunning==false && pressedWithDebounce(GPIOA, MAX_SPEED_Pin,&lastState[maxSpeedKey],&lastPressTime[maxSpeedKey] , NULL) ){
    maxSpeedRunning=!maxSpeedRunning;
    if(maxSpeedRunning){
      _SMPP_setRPM10((uint16_t)maxSpeed);
      _SMPP_setDir(clockwise);
    }
    if(!maxSpeedRunning){
      if(intRunning==true  || lev1Running == true){
        _SMPP_setRPM10(rpm);
      }
      else _SMPP_stop();
    }
  }

}
bool buzzeronOff = false ;
void lockKeyHandle(){
          
}



bool changeDirPinLastState = true;
unsigned long lastChangedirPinPressedTime=0;
bool clockwise = true ;

void RotationBUttonHandle()
{ 
  if(DispenseRunning == false && modeCounter!=5 && modeCounter!=2 && pressedWithDebounce(GPIOB, DIR_Pin,&lastState[changeDirPin],&lastPressTime[changeDirPin] , NULL)){
  clockwise = !clockwise ;
  // menuUpdateMain=true;
  rotation();
  _SMPP_stop();
  if(pumpRunning==true ||maxSpeedRunning==true){
    unsigned long timeout = 0;
    timeout = HAL_GetTick();
    while(_SMPP_isRunning()){
      if (HAL_GetTick()-timeout>1500){
        break;
      }
    }
    _SMPP_setDir(clockwise);
    if(maxSpeedRunning==true)_SMPP_setRPM10((uint16_t)maxSpeed);
    if(pumpRunning == true && maxSpeedRunning== false) _SMPP_setRPM10(rpm);

  }
  else{
  _SMPP_setDir(clockwise);
  }
  modeUpdate = true;
}
}
int load = 0 ;
int boarderTaskTime(){
  if (dispCycles<=1){
    load = dispRuntime() ;
  }
  else 
  {
    load = (dispRuntime() + (dispCycles-1)*(dispRuntime()+dispPauseTime())) ;
  }
   return load ;
}




// void menuButtonHandle() {
 
//   if (DispenseRunning==false && maxSpeedRunning==false && pumpRunning == false && pressedWithDebounce(GPIOB , MENU_Pin, &lastState[menuKey], &lastPressTime[menuKey] , NULL)) {
//     changeMenu = !changeMenu;
//     // nextLoop = !nextLoop ;
//     clearLoadingBar();

//     u8g2_ClearBuffer(&u8g2);
//     if (changeMenu) {
//       menuUpdate = true;  
//     }
//     if(!changeMenu){
//       if(needsSave){
//       settingsSave() ;
//       needsSave = false;
//       }
//       modeUpdate=true;
//       menuUpdateMain=true;
//     }
    
//   }
// }




//boarder Animations;
typedef struct {
  int x;
  int y;
  int endx;
  int endy;
} BorderLine;

BorderLine boarderr[] = {
  { 63, 1, 126, 1 },
  { 126, 1, 126, 62 },
  { 126, 62, 1, 62 },
  { 1, 62, 1, 1 },
  { 1, 1, 62, 1 },
};

const uint16_t totalPixels = 373;
uint32_t loadingTime;  /* total job length, 100 ms ticks */
uint32_t loadStartTick;   /* logicalTick100ms when the job began */

int pixelsDrawn = 0;
int currLine = 0;
int xc = 0;
int yc = 0;


// void resyncLoadingBar(uint64_t nominalElapsedSoFar)
// {
//   if (nominalTotalLoadTime == 0) return;        // not currently loading
//   uint64_t actualElapsed = HAL_GetTick() - startTime; // real ms since load started
//   int64_t drift = (int64_t)actualElapsed - (int64_t)nominalElapsedSoFar;
//   loadingTime = (long)(nominalTotalLoadTime + drift);
//   if (loadingTime < 1) loadingTime = 1;          // guard divide-by-zero in loadingTask()
// }

void boarder()
{
  u8g2_DrawPixel(&u8g2 , xc, yc);

  if (xc == boarderr[currLine].endx && yc == boarderr[currLine].endy)
  {
    currLine++;
    if (currLine >= 5)
    {
      currLine = 0;
    }

    xc = boarderr[currLine].x;
    yc = boarderr[currLine].y;
  }

  if (xc < boarderr[currLine].endx)
    xc++;
  else if (xc > boarderr[currLine].endx)
    xc--;

  if (yc < boarderr[currLine].endy)
    yc++;
  else if (yc > boarderr[currLine].endy)
    yc--;
}

void clearLoadingBar(void)
{
    currLine = 0;
    xc = boarderr[0].x;
    yc = boarderr[0].y;
    pixelsDrawn = 0;

    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawLine(&u8g2, 1, 1, 126, 1);
    u8g2_DrawLine(&u8g2, 126, 1, 126, 62);
    u8g2_DrawLine(&u8g2, 126, 62, 1, 62);
    u8g2_DrawLine(&u8g2, 1, 62, 1, 1);
    u8g2_SetDrawColor(&u8g2, 1);
}
void startLoading(uint32_t ticks)
 {

clearLoadingBar();
  loadingTime   = ticks;
 loadStartTick = logicalTick100ms;
   pixelsDrawn = 0;

   currLine = 0;
}
// void startLoading(long time)
// {
// clearLoadingBar();
//   loadingTime = time;
//   nominalTotalLoadTime = time;      
//   startTime = HAL_GetTick();
//   pixelsDrawn = 0;
//   // nextLoop = false;
//   currLine = 0;
//   xc = boarderr[currLine].x;
//   yc = boarderr[currLine].y;
// }

void loadingTask()
{
if (loadingTime == 0U) return;

  uint32_t elapsed = logicalTick100ms - loadStartTick;
  if (elapsed > loadingTime) elapsed = loadingTime;

  /* 32-bit safe: exact up to ~13 days of ticks, then coarse scaling */
  uint32_t req = (elapsed <= 0xFFFFFFFFU / (uint32_t)totalPixels)
               ? (elapsed * (uint32_t)totalPixels) / loadingTime
               : elapsed / (loadingTime / (uint32_t)totalPixels + 1U);
  int requiredPixels = (int)req;

  while (pixelsDrawn < requiredPixels)
  {
    boarder();
    pixelsDrawn++;
  }

  // if (pixelsDrawn >= totalPixels)
  // {
  //   nextLoop = true;   
  // }
}

void dispCycleRunningScreenUpdate(void)
{
    static uint32_t lastUpdateTick = 0;

    char fullStrTime[20];
    char fullStrCycles[20];

    uint32_t nowTick;
    uint32_t elapsedTicks;
    uint32_t whole;
    uint32_t fraction;
    const char *unit;

    if (!startelapsedupdate || !DispenseRunning)
        return;

    nowTick = logicalTick100ms;

    if (nowTick == lastUpdateTick)
        return;

    lastUpdateTick = nowTick;

    elapsedTicks = dispenseElapsedTicks;

    if (elapsedTicks >= 864000U)
    {
        whole = elapsedTicks / 864000U;
        fraction = (elapsedTicks % 864000U) / 86400U;
        unit = "days";
    }
    else if (elapsedTicks >= 36000U)
    {
        whole = elapsedTicks / 36000U;
        fraction = (elapsedTicks % 36000U) / 3600U;
        unit = "hours";
    }
    else if (elapsedTicks >= 600U)
    {
        whole = elapsedTicks / 600U;
        fraction = (elapsedTicks % 600U) / 60U;
        unit = "mins";
    }
    else
    {
        whole = elapsedTicks / 10U;
        fraction = elapsedTicks % 10U;
        unit = "secs";
    }

    timeElapsedTenths = elapsedTicks;

    snprintf(fullStrTime,
             sizeof(fullStrTime),
             "%lu.%lu %s",
             (unsigned long)whole,
             (unsigned long)fraction,
             unit);

    snprintf(fullStrCycles,
             sizeof(fullStrCycles),
             "%d cycles",
             CycleElapsed);

    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, 65, 25, 59, 37);

    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);

    u8g2_DrawStr(&u8g2,
                 122 - u8g2_GetStrWidth(&u8g2, fullStrTime),
                 42,
                 fullStrTime);

    u8g2_DrawStr(&u8g2,
                 122 - u8g2_GetStrWidth(&u8g2, fullStrCycles),
                 55,
                 fullStrCycles);
}
// void dispCycleRunningScreenUpdate(){
//   static uint32_t lastUpdate = 0;
//   char fullStrTime[20];
//   char fullStrCycles[20];
//   uint32_t now = HAL_GetTick();
//   uint32_t elapsedTenths;
//   uint32_t phaseStart;
//   uint32_t whole;
//   uint32_t fraction;
//   const char *unit;

//   if (!startelapsedupdate || !DispenseRunning) {
//     return;
//   }

//   if (now - lastUpdate < 100U) {
//     return;
//   }

//   if (currentPumpState == pumpPausedDisp) {
//     phaseStart = PauseStartTime;
//   } else if (currentPumpState == pumpRunningDisp) {
//     phaseStart = PumpRunningTime;
//   } else {
//     return;
//   }

//   elapsedTenths = (now - phaseStart) / 100U;
//   timeElapsedSeconds = (float)elapsedTenths;

//   if (elapsedTenths >= 864000U) {
//     whole = elapsedTenths / 864000U;
//     fraction = (elapsedTenths % 864000U) / 86400U;
//     unit = "days";
//   } else if (elapsedTenths >= 36000U) {
//     whole = elapsedTenths / 36000U;
//     fraction = (elapsedTenths % 36000U) / 3600U;
//     unit = "hours";
//   } else if (elapsedTenths >= 600U) {
//     whole = elapsedTenths / 600U;
//     fraction = (elapsedTenths % 600U) / 60U;
//     unit = "mins";
//   } else {
//     whole = elapsedTenths / 10U;
//     fraction = elapsedTenths % 10U;
//     unit = "secs";
//   }

//   snprintf(fullStrTime, sizeof(fullStrTime), "%lu.%lu %s",
//            (unsigned long)whole, (unsigned long)fraction, unit);
//   snprintf(fullStrCycles, sizeof(fullStrCycles), "%d cycles", CycleElapsed);

//   u8g2_SetDrawColor(&u8g2, 0);
//   u8g2_DrawBox(&u8g2, 65, 25, 59, 37);
//   u8g2_SetDrawColor(&u8g2, 1);
//   u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);

//   u8g2_DrawStr(&u8g2, 122 - u8g2_GetStrWidth(&u8g2, fullStrTime), 42,
//                fullStrTime);
//   u8g2_DrawStr(&u8g2, 122 - u8g2_GetStrWidth(&u8g2, fullStrCycles), 55,
//                fullStrCycles);

//   lastUpdate = now;
// }
// void dispCycleRunningScreenUpdate(){
//     if (HAL_GetTick() - lastdispCycleRunningScreenUpdateTime < 50 || !startelapsedupdate) return;

//     status();
//     lastdispCycleRunningScreenUpdateTime = HAL_GetTick();

//     uint32_t elapsed = (currentPumpState == pumpRunningDisp)
//         ? (HAL_GetTick() - PumpRunningTime)
//         : (HAL_GetTick() - PauseStartTime);
//     uint32_t target = (currentPumpState == pumpRunningDisp) ? dispRuntime() : dispPauseTime();
//     uint32_t remainingMs = (elapsed >= target) ? 0 : (target - elapsed);
//     timeElapsedSeconds = remainingMs;   // keep as float only if display code needs it

//     u8g2_SetDrawColor(&u8g2, 0);
//     u8g2_DrawBox(&u8g2, 65, 25, 59, 37);
//     u8g2_DrawBox(&u8g2, 50, 53, 15, 9);
//     u8g2_SetDrawColor(&u8g2, 1);
//     u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);   // was set twice, pointless

//     static char buf[16];
//     char valStr[10];
//     uint32_t ms = remainingMs;
//     const char *unit;
//     uint32_t whole; uint32_t frac;

//     if      (ms > 3596400000UL) { whole = ms / 86400000UL; frac = (ms / 8640000UL) % 10; unit = "days";  }
//     else if (ms > 59940000UL)   { whole = ms / 3600000UL;  frac = (ms / 360000UL) % 10;  unit = "hours"; }
//     else if (ms > 999000UL)     { whole = ms / 60000UL;    frac = (ms / 6000UL) % 10;    unit = "mins";  }
//     else                         { whole = ms / 1000UL;     frac = (ms / 100UL) % 10;     unit = "secs";  }

//     snprintf(valStr, sizeof(valStr), "%lu.%lu", (unsigned long)whole, (unsigned long)frac);
//     snprintf(buf, sizeof(buf), "%s %s", valStr, unit);
//     u8g2_DrawStr(&u8g2, 122 - u8g2_GetStrWidth(&u8g2, buf), 42, buf);

//     snprintf(buf, sizeof(buf), "%d cycles", CycleElapsed);
//     u8g2_DrawStr(&u8g2, 122 - u8g2_GetStrWidth(&u8g2, buf), 55, buf);
// }
// void boarderTask(void *pvParameter)
// {
//     while (1)
//     {
//         vTaskSuspend(NULL);

//         if (DispenseRunning)
//         {
//             xSemaphoreTake(displayMutex, portMAX_DELAY);
//             startLoading(boarderTaskTime());
//             xSemaphoreGive(displayMutex);

//             vTaskDelay(pdMS_TO_TICKS(1));

//             while (DispenseRunning)
//             {
//                 xSemaphoreTake(displayMutex, portMAX_DELAY);
//                   modeButtonHandle();
//                 dispCycleRunningScreenUpdate();
//                 loadingTask();
//                 xSemaphoreGive(displayMutex);

//                 vTaskDelay(pdMS_TO_TICKS(1));   // Prevent CPU hogging
//             }
//         }
//     }
// }
