#pragma once   
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "newglobals.h"


// dispense functions
void dispModeHandle();
     void menudownMain();
    void  menuUpMain();
     void SelectButtonMain();
     void  BackButtonMain();
     void resetMenuStateDisp();
int64_t dispRuntime(void);
int64_t dispPauseTime(void);     void DispCycle();
void dispCycleRunningScreenUpdate();


//helper functions 
void Buzzer_Beep();
 void float2s1(float val, char *buf, size_t buflen);
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
void startLoading(long time);
int boarderTaskTime();
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
static inline uint32_t bitGet(uint32_t data, uint8_t index, uint8_t bits)
{
  uint32_t mask = (1UL << bits) - 1UL;
    return (data >> index) & mask;
}

static inline void bitsSetValue(uint32_t *data, uint8_t index, uint8_t bits, int value)
{
  uint32_t mask = (1UL << bits) - 1UL;
  *data = (*data & ~(mask << index)) | ((value & mask) << index);
}
#define bitsSet(data, index, bits, value) bitsSetValue(&(data), index, bits, value)

// Interface wrappers
static inline void packFloat(uint32_t *data, uint8_t index, uint8_t bits, float value) {
  int scaledInteger = (int)roundf(value * 10.0f);
  bitsSetValue(data, index, bits, scaledInteger);
}

static inline float unpackFloat(uint32_t data, uint8_t index, uint8_t bits) {
  return (float)bitGet(data, index, bits) / 10.0f;
}


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

    float constrain(float value, float minValue, float maxValue);


// home screeen functions 

void drawhome();