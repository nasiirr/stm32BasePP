#include "flashStore.h"
#include "stm32f1xx_hal.h"
#include <stddef.h>
#include <string.h>
#include<newglobals.h>
#include "globalfunctions.h"

#define META_COMMIT_POS  15U
#define META_CRC_POS      7U
#define META_SEQ_POS      0U

#define META_SEQ_MASK    0x007FU
#define META_CRC_MASK    0x00FFU

#define MOTOR_FLASH_PAGE_SIZE  1024U
#define MOTOR_RECORD_SIZE 4U
#define MOTOR_RECORD_COUNT (MOTOR_FLASH_PAGE_SIZE / MOTOR_RECORD_SIZE)
#define MOTOR_META_OFFSET 2U

#define MOTOR_SAVE_DELAY_MS 5000U
#define MENU_SETTINGS_MAGIC 0x4D53U
#define MENU_SETTINGS_WORDS 9U
#define MENU_SAVE_DELAY_MS 5000U

static MotorSettings motorLastObserved;
static MotorSettings motorLastSaved;
static uint32_t motorChangedAt;
static bool motorSaveStateInitialized;
static MenuSettings menuLastSaved;
static uint32_t menuChangedAt;
static bool menuSaveStateInitialized;

void loadFlashSettings(void)
{
   MotorSettings motorSettings;
    MenuSettings menuSettings;


if (FlashStorage_LoadMotor(&motorSettings))
{
    rpm = motorSettings.rpm;
    modeCounter = motorSettings.mode;
    clockwise = motorSettings.clockwise;
}
else
{
    // Defaults used when no valid saved record exists
    rpm = 6000U;
    modeCounter = 1U;
    clockwise = true;
}

if (FlashStorage_LoadMenu(&menuSettings))
    menuSettingsApply(&menuSettings);
}


static uint16_t packMotorData(const MotorSettings *settings)
{
    uint16_t data = 0xFFFFU;

    bitsSet(data, 0U, 13U, settings->rpm);
    bitsSet(data, 13U, 2U, settings->mode);
    bitsSet(data, 15U, 1U, settings->clockwise);

    return data;
}
static uint8_t crc8Update(uint8_t crc, uint8_t value)
{
    crc ^= value;

    for (uint8_t bit = 0U; bit < 8U; ++bit) {
        crc = (crc & 0x80U)
            ? (uint8_t)((crc << 1U) ^ 0x07U)
            : (uint8_t)(crc << 1U);
    }

    return crc;
}

static uint8_t crc8MotorData(uint16_t motorData, uint8_t sequence)
{
    uint8_t crc = 0U;

    crc = crc8Update(crc, (uint8_t)(motorData & 0xFFU));
    crc = crc8Update(crc, (uint8_t)(motorData >> 8U));
    crc = crc8Update(crc, sequence);

    return crc;
}

static bool sequenceIsNewer(uint8_t candidate, uint8_t current)
{
    uint8_t difference = (uint8_t)((candidate - current) & META_SEQ_MASK);

    return difference != 0U && difference < 64U;
}

static uint16_t packMetadata(uint8_t sequence,
                             uint8_t crc,
                             bool committed)
{
    uint16_t data = 0xFFFFU;

    bitsSet(data, META_SEQ_POS, 7, sequence);
    bitsSet(data, META_CRC_POS, 8, crc);

    // Erased bit = 1; committed bit = 0.
    if (committed)
        data &= ~(1U << META_COMMIT_POS);

    return data;
}



static bool readMotorRecord(uint32_t address,
                            MotorSettings *out,
                            uint8_t *sequenceOut)
{
    uint16_t motorData = *(const uint16_t *)address;
    uint16_t metadata = *(const uint16_t *)(address + MOTOR_META_OFFSET);
    uint8_t sequence;
    uint8_t storedCrc;

    if ((metadata & (1U << META_COMMIT_POS)) != 0U) {
        return false;
    }

    sequence = (uint8_t)(metadata & META_SEQ_MASK);
    storedCrc = (uint8_t)((metadata >> META_CRC_POS) & META_CRC_MASK);

    if (storedCrc != crc8MotorData(motorData, sequence)) {
        return false;
    }

    out->rpm = bitGet(motorData, 0U, 13U);
    out->mode = bitGet(motorData, 13U, 2U);
    out->clockwise = bitGet(motorData, 15U, 1U) != 0U;
    *sequenceOut = sequence;

    return true;
}


static bool findLatestMotorRecord(MotorSettings *out,
                                  uint8_t *sequenceOut)
{
    MotorSettings candidate;
    uint8_t candidateSequence;
    bool found = false;

    for (uint32_t slot = 0U;
         slot < MOTOR_RECORD_COUNT;
         ++slot)
    {
        uint32_t slotAddress =
            RPM_MODE_PAGE + (slot * MOTOR_RECORD_SIZE);

        if (readMotorRecord(slotAddress, &candidate, &candidateSequence) &&
            (!found || sequenceIsNewer(candidateSequence, *sequenceOut))) {
            *out = candidate;
            *sequenceOut = candidateSequence;
            found = true;
        }
    }

    return found;
}



bool FlashStorage_SaveMotor(const MotorSettings *in)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t pageError = 0U;
    MotorSettings current;
    uint16_t motorData;
    uint16_t metadata;
    uint16_t existingData;
    uint16_t existingMetadata;
    uint32_t address = 0U;
    uint8_t latestSequence = 0U;
    uint8_t sequence;
    uint8_t crc;
    bool hasErasedSlot = false;

  if (in == NULL ||
    in->rpm > 6000U ||
    in->mode > 3U)
{
    return false;
}

    (void)findLatestMotorRecord(&current, &latestSequence);
    sequence = (uint8_t)((latestSequence + 1U) & META_SEQ_MASK);

    for (uint32_t slot = 0U; slot < MOTOR_RECORD_COUNT; ++slot) {
        uint32_t candidateAddress =
            RPM_MODE_PAGE + (slot * MOTOR_RECORD_SIZE);

        existingData = *(const uint16_t *)candidateAddress;
        existingMetadata = *(const uint16_t *)
            (candidateAddress + MOTOR_META_OFFSET);

        if (existingData == 0xFFFFU && existingMetadata == 0xFFFFU) {
            address = candidateAddress;
            hasErasedSlot = true;
            break;
        }
    }

    HAL_FLASH_Unlock();

    if (!hasErasedSlot) {
        erase.TypeErase = FLASH_TYPEERASE_PAGES;
        erase.PageAddress = RPM_MODE_PAGE;
        erase.NbPages = 1U;

        if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }

        address = RPM_MODE_PAGE;
        sequence = 0U;
    }

    motorData = packMotorData(in);
    crc = crc8MotorData(motorData, sequence);
    metadata = packMetadata(sequence, crc, true);

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                          address,
                          motorData) != HAL_OK ||
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                          address + MOTOR_META_OFFSET,
                          metadata) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }

    
HAL_FLASH_Lock();

MotorSettings verify;
uint8_t verifySequence;

if (!readMotorRecord(address, &verify, &verifySequence))
{
    return false;
}

if (verify.rpm != in->rpm ||
    verify.mode != in->mode ||
    verify.clockwise != in->clockwise)
{
    return false;
}

return true;

}


bool FlashStorage_LoadMotor(MotorSettings *out)
{
    uint8_t sequence;

    if (out == NULL) {
        return false;
    }

    return findLatestMotorRecord(out, &sequence);
}

void FlashStorage_ProcessMotorSave(void)
{
    MotorSettings current;
    uint32_t now = HAL_GetTick();
    bool valuesChanged;
    bool shouldSave;

    current.rpm = rpm;
    current.mode = modeCounter;
    current.clockwise = clockwise;

    if (!motorSaveStateInitialized) {
        motorLastObserved = current;
        motorLastSaved = current;
        motorChangedAt = now;
        motorSaveStateInitialized = true;
        return;
    }

    valuesChanged =
        current.rpm != motorLastObserved.rpm ||
        current.mode != motorLastObserved.mode ||
        current.clockwise != motorLastObserved.clockwise;

    if (valuesChanged) {
        motorLastObserved = current;
        motorChangedAt = now;
    }

    valuesChanged =
        current.rpm != motorLastSaved.rpm ||
        current.mode != motorLastSaved.mode ||
        current.clockwise != motorLastSaved.clockwise;

    shouldSave = valuesChanged &&
        (uint32_t)(now - motorChangedAt) >= MOTOR_SAVE_DELAY_MS;

    if (shouldSave && FlashStorage_SaveMotor(&current)) {
        motorLastSaved = current;
    }
}

bool FlashStorage_LoadMenu(MenuSettings *settings)
{
    const uint16_t *stored = (const uint16_t *)MENU_SETTINGS_PAGE;

    if (settings == NULL || stored[0] != MENU_SETTINGS_MAGIC)
        return false;

    settings->settings1Bit = stored[1];
    settings->settings2Bit = stored[2];

    for (uint8_t index = 0; index < 6; index++)
        settings->settingsInt[index] = stored[index + 3U];

    return true;
}

bool FlashStorage_SaveMenu(const MenuSettings *settings)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t pageError = 0U;
    uint16_t words[MENU_SETTINGS_WORDS];

    if (settings == NULL)
        return false;

    words[0] = MENU_SETTINGS_MAGIC;
    words[1] = settings->settings1Bit;
    words[2] = settings->settings2Bit;

    for (uint8_t index = 0; index < 6; index++)
        words[index + 3U] = settings->settingsInt[index];

    HAL_FLASH_Unlock();
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = MENU_SETTINGS_PAGE;
    erase.NbPages = 1U;

    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }

    for (uint8_t index = 0; index < MENU_SETTINGS_WORDS; index++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                              MENU_SETTINGS_PAGE + (index * 2U),
                              words[index]) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();

    MenuSettings verify;
    if (!FlashStorage_LoadMenu(&verify))
        return false;

    return memcmp(&verify, settings, sizeof(MenuSettings)) == 0;
}

void FlashStorage_ProcessMenuSave(void)
{
    MenuSettings current;
    uint32_t now = HAL_GetTick();

    menuSettingsCollect(&current);

    if (!menuSaveStateInitialized) {
        menuLastSaved = current;
        menuSaveStateInitialized = true;
        menuChangedAt = now;
        return;
    }

    if (memcmp(&current, &menuLastSaved, sizeof(MenuSettings)) != 0) {
        menuChangedAt = now;
        menuLastSaved = current;
    }

    if (needsSave && (uint32_t)(now - menuChangedAt) >= MENU_SAVE_DELAY_MS) {
        if (FlashStorage_SaveMenu(&current))
            needsSave = false;
    }
}