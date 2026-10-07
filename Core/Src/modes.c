#include "main.h"
#include "stm32f1xx_hal.h"
#include "u8g2.h"
#include<newglobals.h>
#include<globalfunctions.h>

bool lastmodeKeyState = 1;
// bool modeUpdate = true ;
bool intRunning = false ;
bool lev1Running = false ;
uint8_t modeCounter = 0 ;
bool changeMenu = false;
bool displayDirty = true;
const char* modesarr[] = {
  "DISP ", "INT", "EXTV", "EXTI", "LEV1", "LEV2",
};

void dispModeHandle(){
    // DispCycle() ;
    tickDisp();
    Disp_ProcessEvents();
    
         if(modeCounter==0 && DispenseRunning==false){
        rpmUpdateButtonsHandle();
  menudownMain();
  menuUpMain();
  SelectButtonMain();
  BackButtonMain();
      
        }

       status();
      if (DispenseRunning)
            {  

                rpmUpdateButtonsHandle();
                rpmUpdateScreen();
                dispCycleRunningScreenUpdate();
                if(dispCycles!=0){
                loadingTask();
                }

            }
      if (menuUpdateMain) {
      drawMenuMain();
      menuUpdateMain = false;
    }
}

void drawModeName(){
  if (currentScreen == SCREEN_TIME) return;

  u8g2_SetDrawColor(&u8g2, 0);
  u8g2_DrawBox(&u8g2, 29, 5, 32, 15);
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
  u8g2_DrawStr(&u8g2, 29, 20, modesarr[modeCounter]);
}

void modeButtonHandle(){
  if(intRunning == false && dispTimeEdit == false && DispenseRunning==false && maxSpeedRunning==false && 
    pressedWithDebounce( GPIOB , MODE_Pin , &lastState[modeKey],&lastPressTime[modeKey] , &LongPresslastModeKEyPressedTime )){
    modeCounter ++;
    modeUpdate=true;
    homeScrenUpdate = true ;

    u8g2_ClearBuffer(&u8g2);

    if (modeCounter>5){
      modeCounter = 0;
    }
    if(modeCounter==0){
      menuUpdateMain = true ;
      resetMenuStateDisp() ;
      drawMenuMain() ;
      
    }
  }
   
    if(modeUpdate ){
      drawModeName();
  if(DispenseRunning){
      u8g2_SetDrawColor(&u8g2,0);

      u8g2_DrawBox(&u8g2,65,3,60,22);

      u8g2_SetDrawColor(&u8g2,1);
  }

  }
}
void intModeHandle(){
  if (modeCounter==1)
  {
    rpmUpdateButtonsHandle();
  }
  
  if (modeCounter==1 && pressedWithDebounce(GPIOB, START_Pin,&lastState[startStop],&lastPressTime[startStop] , NULL) ){
    if (DispenseRunning==false )
    { if(pumpRunning == false && maxSpeedRunning==false){
    pumpRunning =true ;
    intRunning = true ;
    currentPumpState = pumpRunningDisp;  
    _SMPP_setDir(clockwise);
    _SMPP_setRPM(rpm);              
    }
    else{
    pumpRunning =false ;
    intRunning= false ;
    currentPumpState = pumpStoppedDisp;  
    _SMPP_stop();  
    maxSpeedRunning=false;
    }
  }
}
}
bool lastcurrDirectionDB15 = 1;
bool lastcurrentPinState = 1 ;
bool db15InputStateInitialized = false;
// bool currDirectionDB15 = 0;
int lastExtRpm = 0;
// ---- shared: DB15 direction pin (used by EXTV, LEV2 — LEV1 has no direction input) ----
void handleDB15Direction() {
  GPIO_PinState directionPinState = HAL_GPIO_ReadPin(EXT_DIR_GPIO_Port, EXT_DIR_Pin);

  if (!db15InputStateInitialized) {
    currDirectionDB15 = (directionPinState == GPIO_PIN_SET);
    lastcurrDirectionDB15 = currDirectionDB15;
    _SMPP_setDir(currDirectionDB15);
    db15InputStateInitialized = true;
    return;
  }

  currDirectionDB15 = (directionPinState == GPIO_PIN_SET);
  if (currDirectionDB15 != lastcurrDirectionDB15) {
    lastcurrDirectionDB15 = currDirectionDB15;
    _SMPP_stop();
    unsigned long timeout = HAL_GetTick();
    while (_SMPP_isRunning()) {
      if (HAL_GetTick() - timeout > 1500) break;
    }
    _SMPP_setDir(currDirectionDB15);
    lastcurrentPinState = 1;
    pumpRunning = false;
    rotation();
  }
}

// ---- shared: DB15 start/stop pin (used by EXTV, LEV1, LEV2) ----
void handleDB15StartStop(int rpmToUse) {
  if (DispenseRunning) {
    return;
  }

  GPIO_PinState currentPinState = HAL_GPIO_ReadPin(EXT_START_GPIO_Port, EXT_START_Pin);
  GPIO_PinState activePinState = levelTrigHigh ? GPIO_PIN_SET : GPIO_PIN_RESET;
  bool triggerActive = (currentPinState == activePinState);

  if (triggerActive && !pumpRunning && !maxSpeedRunning) {
    pumpRunning = true;
    lev1Running = true;
    currentPumpState = pumpRunningDisp;
    _SMPP_setDir(currDirectionDB15);
    _SMPP_setRPM(rpmToUse);
  }
  else if (!triggerActive && pumpRunning) {
    pumpRunning = false;
    lev1Running = false;
    currentPumpState = pumpStoppedDisp;
    _SMPP_stop();
    maxSpeedRunning = false;
  }

  lastcurrentPinState = currentPinState;
}

// ---- modeCounter == 2 : EXTV ----
void handleExtV() {
int adcval = 400;
rpmext = ((long)adcval * 600L) / 4095;
printf("ADC:%d,RPM:%d\n", adcval, rpmext);
  if (lastExtRpm != rpmext) {
    lastExtRpm = rpmext;

    rpmUpdateScreen();
    modeUpdate = true;
  
    if (pumpRunning) {
      _SMPP_setRPM(rpmext);
       lastExtRpm = rpmext;
    }
  }
  handleDB15Direction();
  handleDB15StartStop(rpmext);
}

// ---- modeCounter == 4 : LEV1 ----
void handleLev1() {
  handleDB15StartStop(rpm);
  rpmUpdateButtonsHandle();
}

// ---- modeCounter == 3 : EXTI ----
void handleExtI() {
  handleDB15StartStop(rpm);
}

// ---- modeCounter == 5 : LEV2 ----
void handleLev2() {
  handleDB15Direction();
  handleDB15StartStop(rpm);
    rpmUpdateButtonsHandle();
}


