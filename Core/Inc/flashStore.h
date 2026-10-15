#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>
#include <stdbool.h>

#define RPM_MODE_PAGE       0x0800F400UL
#define DISP_PARAM_PAGE     0x0800F800UL
#define MENU_SETTINGS_PAGE  0x0800FC00UL

typedef struct
{
    uint16_t rpm;
    uint8_t mode;
    bool clockwise;
} MotorSettings;


typedef struct
{
    uint16_t runTime;
    uint16_t pauseTime;
    uint16_t cycles;
} DispSettings;

bool FlashStorage_Init(void);

bool FlashStorage_LoadMotor(MotorSettings *out);
bool FlashStorage_SaveMotor(const MotorSettings *in);
void FlashStorage_ProcessMotorSave(void);

bool FlashStorage_LoadDisp(DispSettings *out);
bool FlashStorage_SaveDisp(const DispSettings *in);

bool FlashStorage_LoadMenu(uint32_t *settingsData);
bool FlashStorage_SaveMenu(uint32_t settingsData);

#endif