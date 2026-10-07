//important global variables and constants used across the project
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "cmsis_os2.h"

#include "STM32_u8g2_hal.h"
#include <Bitmap.h>
#include<stepperPP.h>

void drawhome(void);
extern u8g2_t u8g2;



// spi 
#define OLED_SCK   6
#define OLED_MOSI  7
#define OLED_CS   10
#define OLED_DC   25
#define OLED_RST  26

//previous pins 
static const int buzzerPin = 0;  // esp 
static const int maxSpeedKey = 0;  // mcp
static const int selectKey  = 1;  // SELECT
static const int upKey    = 2;  // UP
static const int changeDirPin = 3;
static const int menuKey = 4;  // Invert display colour
static const int modeKey = 5 ;  // Mode button
static const int lockKey = 6;  // Lock button
static const int startStop = 7 ;  //Start/Stop Button
static const int downKey = 8;  // DOWN



//pumpstate 
#define MAX_RPM 600.0f
#define MIN_RPM 0.1f
extern int maxSpeed;
extern bool maxSpeedRunning;
extern float rpm;
extern int rpmext ;
extern int reverseAngle ;

extern bool clockwise ;
extern bool pumpRunning ;
extern bool intRunning ;
extern bool lev1Running ;

typedef enum pumpStateDisp {
  pumpRunningDisp,
  pumpPausedDisp,
  pumpStoppingDisp,
  pumpStoppedDisp
} pumpStateDisp;
extern pumpStateDisp currentPumpState;  

extern unsigned long PumpRunningTime;  //time when pump started running
extern unsigned long PauseStartTime ;  //time when pump paused
//hardware configuration

extern TIM_HandleTypeDef htim3;   //timer for buzzer


extern bool lastState[9];     //used to store the last state of each button for debouncing
extern unsigned long lastPressTime[9];  //used to store the last time each button was pressed for debouncing

extern unsigned long LongPressPumpRunningTime ;
extern unsigned long LongPressPauseStartTime ;
extern unsigned long LongPressupLastPressTime ;
extern unsigned long LongPressdownLastPressTime ;
extern unsigned long LongPresslastModeKEyPressedTime ;


//disp cycle 
extern volatile uint32_t totalRemaining;
extern volatile uint32_t phaseRemaining;
extern bool  dispTimeEdit ;    // true when the dispense time is being edited in the menu
extern float runTime;
extern float pauseTime;
extern int dispCycles;
extern int CycleElapsed;
extern bool startelapsedupdate ;
extern float timeElapsedSeconds ;
extern bool DispenseRunning ;
extern unsigned long lastdispCycleRunningScreenUpdateTime;
extern const char* currentTitleMain;
extern bool rpmEditing;
extern bool isEditingMain;
extern int selectedMain;
extern int load ;    //used to store the total load time for the dispense cycle  and pass it to boarder animation function

extern uint32_t dispenseSettings;   // 30 bits for runTime, pauseTime, dispCycles   

typedef enum MenuScreen { SCREEN_HOME, SCREEN_TIME } MenuScreen;
extern MenuScreen  currentScreen ;


//ui 
#define RIGHT_EDGE 122

extern bool changeMenu ; // true when menu is active, false when home screen is active
extern bool modeUpdate ;   // true when mode has changed and needs to be reflected on the display
extern bool menuUpdate ;   // true when menu has changed and needs to be reflected on the display
extern bool menuUpdateMain ; // true when dispense pt,rt ,cycles ,rpm or menu screen  has changed and needs to be reflected on the display


//modes 
extern uint8_t modeCounter ;
extern bool intRunning ;
extern bool lev1Running;
extern bool currDirectionDB15 ;
void   modeButtonHandle();
extern bool homeScrenUpdate;

enum homeScreenState{
        dispMode,
        intMode,
        extMode,
        extVMode,
        extIMode,
        lev1Mode,
        lev2Mode
};

//settings save 
extern bool buzzerStart ;
extern bool  toneproduce;
typedef enum runtimeUnit{
    days, hours , minutes, seconds 
} runtimeUnit ;
extern runtimeUnit currentRunTimeUnit ;
extern runtimeUnit currentPauseTimeUnit;

extern int currPTUnitSel;
extern int currRTUnitSel;


extern bool levelTrigHigh;