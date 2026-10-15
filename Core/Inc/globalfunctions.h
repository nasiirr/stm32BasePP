#pragma once   
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "newglobals.h"


// dispense functions
void tickDisp();
void dispModeHandle();
     void menudownMain();
    void  menuUpMain();
     void SelectButtonMain();
     void  BackButtonMain();
     void resetMenuStateDisp();
int32_t dispRuntime(void);
int32_t dispPauseTime(void);     void DispCycle();
void dispCycleRunningScreenUpdate();


//helper functions 
void Buzzer_Beep();
void float2s1(uint16_t val, char *buf, size_t buflen);
void rpmUpdateButtonsHandle();

struct line
{
  int x;
  int y;
  int endx;
  int endy;
};

bool pressedWithDebounce(GPIO_TypeDef *GPIOx,
                         uint16_t pin,
                         bool *lastState,
                         unsigned long *lastPressTime,
                         uint32_t *lastRepeatTime);
extern uint16_t activeHeldPin;
void menuSystemInit();
void menuButtonHandle();
void splashScreen() ;
void clearLoadingBar();
void RotationBUttonHandle();
void maxSpeedKeyHandle();
void startLoading(uint32_t ticks);int boarderTaskTime();
void loadingTask();
void resyncLoadingBar(uint64_t nominalElapsedSoFar);

//home screen functions
extern bool common;
extern void modeButtonHandle();
extern void drawModeName();
extern void intModeHandle();
extern void handleExtI();
extern void handleLev2() ;
extern void handleLev1();
extern void handleExtV();
void mainTask(void* pvParameters);
void boarderTask(void* pvParameters);
void drawMenuMain();
void sound1(bool x);
void wifi2(bool x);
void rotation();
void status();
void rpmUpdateScreen();

void updateDisplayAreaPixel(int x, int y, int w, int h);


//menu functions
static inline int maxInt(int a, int b)
{
    return (a > b) ? a : b;
}
static inline uint16_t bitGet(uint16_t data, uint8_t index, uint8_t bits)
{
  uint16_t mask = (1UL << bits) - 1UL;
    return (data >> index) & mask;
}

static inline void bitsSetValue(uint16_t *data, uint8_t index, uint8_t bits, int value)
{
  uint16_t mask = (1UL << bits) - 1UL;
  *data = (*data & ~(mask << index)) | ((value & mask) << index);
}

#define bitsSet(data, index, bits, value) \
  bitsSetValue((uint16_t *)&(data), index, bits, value)

// Interface wrappers



extern bool needsSave;
extern bool wifiEnabled ;
extern uint32_t settingsData;
void drawMenu();
void UITask(void* pvParameters);
  void SelectButton();
    void BackButton();
    void menuUp();
    void menudown();
    void show();
    void prefsload();
    void settingsSave();

static inline int32_t constrainInt32(int32_t value,
                                     int32_t minValue,
                                     int32_t maxValue)
{
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}


// home screeen functions 

void drawhome();



//flash saving and loading functions
void loadFlashSettings(void) ;

