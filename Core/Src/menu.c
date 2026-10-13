#include "main.h"
#include "newglobals.h"
#include "globalfunctions.h"
#include <stdio.h>

extern u8g2_t u8g2;

uint32_t settingsData = 0;

int timeSet = 0;
int cycles = 0;
int maxSpeed = 350;
int pumpNo = 0;
int accTime = 0;
int decTime = 0;
int dispStop = 0;
int selected = 0;
int firstVisible = 0;
int parentIndex = 0;
bool menuUpdate = true;
const int lineHeight = 12;
const int visibleLines = 5;

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
runtimeUnit currentRunTimeUnit = seconds;
runtimeUnit currentPauseTimeUnit = seconds;
bool toneproduce = false;

void primePump(void) {}
void calibration(void) {}
void diagnostics(void) {}
void wifiReset(void) {}
void factoryReset(void) {}
void firmwareInfo(void) {}
void aboutInfo(void) {}

typedef enum {
  K_NONE = 0,
  K_MENU,
  K_OPT,
  K_ACTION,
  K_EDIT
} Kind;

typedef struct {
  uint8_t pos;
  uint8_t width;
  uint8_t inv;
} BitField;

typedef struct MenuNode MenuNode;

struct MenuNode {
  const char *name;
  const char *title;
  union {
    const MenuNode *children;
    void (*action)(void);
    int *value;
  };
  const BitField *bf;
  int16_t min;
  int16_t max;
  uint8_t kind;
  uint8_t count;
};

_Static_assert(sizeof(MenuNode) == 24, "Unexpected MenuNode size");

static inline uint8_t optGet(const BitField *bf)
{
  if (bf == NULL || bf->width == 0 || bf->width > 8 ||
      bf->pos >= 32 || (uint16_t)bf->pos + bf->width > 32) {
    return 0;
  }
  return (uint8_t)(bitGet(settingsData, bf->pos, bf->width) ^ bf->inv);
}

static bool optSet(const BitField *bf, uint8_t index, uint8_t count)
{
  uint32_t limit;

  if (bf == NULL || bf->width == 0 || bf->width > 8 ||
      bf->pos >= 32 || (uint16_t)bf->pos + bf->width > 32 ||
      count == 0) {
    return false;
  }

  limit = 1UL << bf->width;
  if (index >= count || index >= limit) {
    return false;
  }

  bitsSet(settingsData, bf->pos, bf->width, (uint8_t)(index ^ bf->inv));
  needsSave = true;
  return true;
}

static const BitField bfTone = { 0, 1, 1 };
static const BitField bfContrast = { 1, 1, 1 };
static const BitField bfReverseAngle = { 2, 2, 0 };
static const BitField bfLanguage = { 5, 1, 0 };
static const BitField bfTubeAlert = { 6, 1, 1 };
static const BitField bfWifi = { 7, 1, 1 };
static const BitField bfWifiReset = { 8, 1, 1 };
static const BitField bfExternal = { 9, 2, 0 };
static const BitField bfBaud = { 11, 2, 0 };
static const BitField bfParity = { 13, 2, 0 };
static const BitField bfStopBit = { 15, 1, 0 };
static const BitField bfLevelTrigger = { 16, 1, 1 };
static const BitField bfRuntimeUnit = { 17, 2, 0 };
static const BitField bfPauseUnit = { 19, 2, 0 };

static void syncOption(const BitField *bf, uint8_t index)
{
  if (bf == &bfTone) {
    toneproduce = index == 0;
    currtoneSet = index;
  } else if (bf == &bfContrast) {
    contrastflag = index == 0;
    currentcontrast = index;
  } else if (bf == &bfReverseAngle) {
    static const int angles[] = { 0, 180, 360, 720 };
    reverseAngle = angles[index];
    currRevAng = index;
  } else if (bf == &bfLanguage) {
    languageHindi = index == 0;
    currentLanguage = index;
  } else if (bf == &bfTubeAlert) {
    tubingAlertEnabled = index == 0;
    currtubeAlert = index;
  } else if (bf == &bfWifi) {
    wifiEnabled = index == 0;
    currwifiSet = index;
  } else if (bf == &bfWifiReset) {
    wifiResetFlag = index == 0;
    currwifiReset = index;
  } else if (bf == &bfExternal) {
    currextMenu = index;
  } else if (bf == &bfBaud) {
    currentBaudIndex = index;
  } else if (bf == &bfParity) {
    currentParityIndex = index;
  } else if (bf == &bfStopBit) {
    currentStopBitIndex = index;
    stopBit = index != 0;
  } else if (bf == &bfLevelTrigger) {
    levelTrigHigh = index == 0;
    currentLevelTrigIndex = index;
  } else if (bf == &bfRuntimeUnit) {
    currRTUnitSel = index;
    currentRunTimeUnit = (runtimeUnit)index;
  } else if (bf == &bfPauseUnit) {
    currPTUnitSel = index;
    currentPauseTimeUnit = (runtimeUnit)index;
  }
}

static const MenuNode language[];
static const MenuNode contrastMenu[];
static const MenuNode toneSet[];
static const MenuNode generalMenu[];
static const MenuNode rtUnitMenu[];
static const MenuNode pauseTimeUnitMenu[];
static const MenuNode revAngMenu[];
static const MenuNode tubeAlertMenu[];
static const MenuNode funtionalMenu[];
static const MenuNode wifiMenu[];
static const MenuNode wifiResetMenu[];
static const MenuNode extMenu[];
static const MenuNode softwareMenu[];
static const MenuNode ExternalMenu[];
static const MenuNode baudMenu[];
static const MenuNode parityMenu[];
static const MenuNode stopBitMenu[];
static const MenuNode leveltrigmenu[];
static const MenuNode systemMenu[];
static const MenuNode mainMenu[];

#define OPT(name) { name, NULL, { NULL }, NULL, 0, 0, K_OPT, 0 }

static const MenuNode language[] = {
  OPT("HINDI"), OPT("ENGLISH")
};
static const MenuNode contrastMenu[] = {
  OPT("LIGHT"), OPT("DARK")
};
static const MenuNode toneSet[] = {
  OPT("ENABLE"), OPT("DISABLE")
};
static const MenuNode generalMenu[] = {
  { "TIMESET", NULL, { .value = &timeSet }, NULL, 0, 999, K_EDIT, 0 },
  { "LANGUAGE", NULL, { .children = language }, &bfLanguage, 0, 0, K_MENU, 2 },
  { "CONTRAST", NULL, { .children = contrastMenu }, &bfContrast, 0, 0, K_MENU, 2 },
  { "TONE SET", NULL, { .children = toneSet }, &bfTone, 0, 0, K_MENU, 2 }
};
static const MenuNode rtUnitMenu[] = {
  OPT("DAYS"), OPT("HOURS"), OPT("MINUTES"), OPT("SECONDS")
};
static const MenuNode pauseTimeUnitMenu[] = {
  OPT("DAYS"), OPT("HOURS"), OPT("MINUTES"), OPT("SECONDS")
};
static const MenuNode revAngMenu[] = {
  OPT("0"), OPT("180"), OPT("360"), OPT("720")
};
static const MenuNode tubeAlertMenu[] = {
  OPT("YES"), OPT("NO")
};
static const MenuNode funtionalMenu[] = {
  { "RT UNIT", NULL, { .children = rtUnitMenu }, &bfRuntimeUnit, 0, 0, K_MENU, 4 },
  { "PT UNIT", NULL, { .children = pauseTimeUnitMenu }, &bfPauseUnit, 0, 0, K_MENU, 4 },
  { "REVERSE ANGLE", "REV ANGLE", { .children = revAngMenu }, &bfReverseAngle, 0, 0, K_MENU, 4 },
  { "PUMP NO.", NULL, { .value = &pumpNo }, NULL, 0, 999, K_EDIT, 0 },
  { "MAX SPEED", NULL, { .value = &maxSpeed }, NULL, 0, 999, K_EDIT, 0 },
  { "TUBING ALERT", "TUBE ALERT", { .children = tubeAlertMenu }, &bfTubeAlert, 0, 0, K_MENU, 2 }
};
static const MenuNode wifiMenu[] = {
  OPT("ENABLE"), OPT("DISABLED")
};
static const MenuNode wifiResetMenu[] = {
  OPT("YES"), OPT("NO")
};
static const MenuNode extMenu[] = {
  OPT("CURRENT"), OPT("VOLT 0-5V"), OPT("VOLT 0-10V")
};
static const MenuNode softwareMenu[] = {
  { "BOOT V3.1.0", NULL, { .action = wifiReset }, NULL, 0, 0, K_ACTION, 0 },
  { "USER v3.2.3", NULL, { 0 }, NULL, 0, 0, K_NONE, 0 }
};
static const MenuNode ExternalMenu[] = {
  { "WIFI SETTINGS", "WIFI SET", { .children = wifiMenu }, &bfWifi, 0, 0, K_MENU, 2 },
  { "WIFI RST", NULL, { .children = wifiResetMenu }, &bfWifiReset, 0, 0, K_MENU, 2 },
  { "EXT CONTROLS", "EXT CONT", { .children = extMenu }, &bfExternal, 0, 0, K_MENU, 3 },
  { "SOFTWARE", NULL, { .children = softwareMenu }, NULL, 0, 0, K_MENU, 2 }
};
static const MenuNode baudMenu[] = {
  OPT("4800"), OPT("9800"), OPT("19200"), OPT("38400")
};
static const MenuNode parityMenu[] = {
  OPT("NONE"), OPT("EVEN"), OPT("ODD")
};
static const MenuNode stopBitMenu[] = {
  OPT("1"), OPT("0")
};
static const MenuNode leveltrigmenu[] = {
  OPT("HIGH"), OPT("LOW")
};
static const MenuNode systemMenu[] = {
  { "BAUD", NULL, { .children = baudMenu }, &bfBaud, 0, 0, K_MENU, 4 },
  { "PARITY", NULL, { .children = parityMenu }, &bfParity, 0, 0, K_MENU, 3 },
  { "PULSE TRIGGER", "PLS TRIS", { 0 }, NULL, 0, 0, K_NONE, 0 },
  { "LEVEL TRIGGER", "LVL TRIG", { .children = leveltrigmenu }, &bfLevelTrigger, 0, 0, K_MENU, 2 },
  { "DISP STOP", NULL, { .value = &dispStop }, NULL, 0, 10, K_EDIT, 0 },
  { "ACC TIME", NULL, { .value = &accTime }, NULL, 0, 999, K_EDIT, 0 },
  { "DEC TIME", NULL, { .value = &decTime }, NULL, 0, 999, K_EDIT, 0 },
  { "PARAM RESET", NULL, { .children = wifiResetMenu }, &bfWifiReset, 0, 0, K_MENU, 2 },
  { "STOP BIT", NULL, { .children = stopBitMenu }, &bfStopBit, 0, 0, K_MENU, 2 }
};
static const MenuNode mainMenu[] = {
  { "GENERAL", "GEN", { .children = generalMenu }, NULL, 0, 0, K_MENU, 4 },
  { "FUNCTIONAL", "FUNC", { .children = funtionalMenu }, NULL, 0, 0, K_MENU, 6 },
  { "SYSTEM", "SYS", { .children = systemMenu }, NULL, 0, 0, K_MENU, 9 },
  { "EXTERNAL", "EXT", { .children = ExternalMenu }, NULL, 0, 0, K_MENU, 4 }
};

typedef struct {
  const MenuNode *menu;
  uint8_t sel;
  uint8_t first;
} Frame;

_Static_assert(sizeof(Frame) == 8, "Unexpected Frame size");

enum { MAX_DEPTH = 5 };
static const MenuNode ROOT = {
  "MENU", NULL, { .children = mainMenu }, NULL, 0, 0, K_MENU, 4
};

const MenuNode *currentMenu = &ROOT;
static Frame stk[MAX_DEPTH];
static uint8_t stackIndex = 0;
const char *currentTitle = "MENU";

GPIO_TypeDef *activeHeldPort = NULL;
uint16_t activeHeldPin = 0;
unsigned long longPressTime = 500;
unsigned long debounceDelay = 50;
const unsigned long repeatInterval = 0;
unsigned long LongPressupLastPressTime = 0;
unsigned long LongPressdownLastPressTime = 0;
bool lastState[9];
unsigned long lastPressTime[9];

static uint8_t menuCount(const MenuNode *menu)
{
  return menu != NULL && menu->kind == K_MENU ? menu->count : 0;
}

static const MenuNode *menuItems(void)
{
  return currentMenu != NULL ? currentMenu->children : NULL;
}

static const MenuNode *selectedNode(void)
{
  const MenuNode *items = menuItems();
  uint8_t count = menuCount(currentMenu);
  return items != NULL && selected >= 0 && selected < count ? &items[selected] : NULL;
}

static void updateTitle(void)
{
  if (currentMenu == &ROOT) {
    currentTitle = "MENU";
  } else if (currentMenu != NULL) {
    currentTitle = currentMenu->title != NULL ? currentMenu->title : currentMenu->name;
  } else {
    currentTitle = "MENU";
  }
}

static void clampMenuWindow(void)
{
  int count = menuCount(currentMenu);
  if (count == 0) {
    selected = 0;
    firstVisible = 0;
    return;
  }
  if (selected < 0) selected = 0;
  if (selected >= count) selected = count - 1;
  if (firstVisible < 0) firstVisible = 0;
  if (count > visibleLines && firstVisible > count - visibleLines)
    firstVisible = count - visibleLines;
  if (count <= visibleLines) firstVisible = 0;
}

static bool push(const MenuNode *next)
{
  if (next == NULL || next->kind != K_MENU || next->children == NULL ||
      next->count == 0 || stackIndex >= MAX_DEPTH) {
    return false;
  }
  stk[stackIndex].menu = currentMenu;
  stk[stackIndex].sel = (uint8_t)selected;
  stk[stackIndex].first = (uint8_t)firstVisible;
  stackIndex++;
  currentMenu = next;
  selected = next->bf != NULL ? optGet(next->bf) : 0;
  firstVisible = 0;
  updateTitle();
  clampMenuWindow();
  return true;
}

static bool pop(void)
{
  if (stackIndex == 0) return false;
  stackIndex--;
  currentMenu = stk[stackIndex].menu;
  selected = stk[stackIndex].sel;
  firstVisible = stk[stackIndex].first;
  updateTitle();
  clampMenuWindow();
  return true;
}

static void applySelectedOption(void)
{
  const MenuNode *item = selectedNode();
  uint8_t count = menuCount(currentMenu);
  if (item == NULL || item->kind != K_OPT || currentMenu->bf == NULL ||
      !optSet(currentMenu->bf, (uint8_t)selected, count)) {
    return;
  }
  syncOption(currentMenu->bf, (uint8_t)selected);
  pop();
}

bool pressedWithDebounce(GPIO_TypeDef *GPIOx,
                         uint16_t pin,
                         bool *lastStateValue,
                         unsigned long *lastPressTimeValue,
                         uint32_t *lastRepeatTime)
{
  if (activeHeldPort != NULL &&
      (activeHeldPort != GPIOx || activeHeldPin != pin)) {
    return false;
  }

  bool currentState = HAL_GPIO_ReadPin(GPIOx, pin) == GPIO_PIN_SET;
  bool pressed = false;
  uint32_t now = HAL_GetTick();

  if (*lastStateValue && !currentState &&
      (now - *lastPressTimeValue) > debounceDelay) {
    pressed = true;
    if (toneproduce) {
      buzzerStart = true;
      Buzzer_Beep();
    }
    *lastPressTimeValue = now;
    if (lastRepeatTime != NULL) *lastRepeatTime = now;
    activeHeldPort = GPIOx;
    activeHeldPin = pin;
  } else if (lastRepeatTime != NULL && !currentState &&
             !*lastStateValue && (now - *lastPressTimeValue) >= longPressTime &&
             (now - *lastRepeatTime) >= repeatInterval) {
    pressed = true;
    *lastRepeatTime = now;
  }

  if (currentState && !*lastStateValue &&
      activeHeldPort == GPIOx && activeHeldPin == pin) {
    activeHeldPort = NULL;
    activeHeldPin = 0;
  }
  *lastStateValue = currentState;
  return pressed;
}

bool isEditing = false;

void draw1pxScrollbar(int selected_index)
{
  int totalItems = menuCount(currentMenu);
  int visibleItems = visibleLines;
  int sbHeight = 63;
  int barHeight;
  int barY;

  if (totalItems <= visibleItems) return;
  barHeight = (visibleItems * sbHeight) / totalItems;
  if (barHeight < 3) barHeight = 3;
  barY = (selected_index * (sbHeight - barHeight)) / (totalItems - 1);
  if (barY + barHeight > sbHeight) barY = sbHeight - barHeight;
  u8g2_DrawLine(&u8g2, 126, barY, 126, barY + barHeight - 1);
}

int getMenuY(uint8_t totalItems, uint8_t index)
{
  static const uint8_t yPos[][5] = {
    { 38 }, { 22, 54 }, { 17, 38, 59 },
    { 12, 28, 44, 59 }, { 12, 24, 36, 48, 60 }
  };
  if (totalItems < 1) totalItems = 1;
  if (totalItems > 5) totalItems = 5;
  if (index >= totalItems) index = (uint8_t)(totalItems - 1);
  return yPos[totalItems - 1][index];
}

void drawMenu(void)
{
  const MenuNode *items;
  int count;
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
  clampMenuWindow();
  items = menuItems();
  count = menuCount(currentMenu);

  const char *displayTitle = isEditing && selectedNode() != NULL ?
                             selectedNode()->name : currentTitle;
  int titleWidth = u8g2_GetStrWidth(&u8g2, displayTitle);
  int titleX = (60 - titleWidth) / 2;
  if (titleX < 0) titleX = 0;
  u8g2_DrawRBox(&u8g2, 2, 27, titleWidth + 4, 13, 4);
  u8g2_SetDrawColor(&u8g2, 0);
  u8g2_DrawStr(&u8g2, 4, 37, displayTitle);
  u8g2_SetDrawColor(&u8g2, 1);

  if (isEditing) {
    const MenuNode *item = selectedNode();
    if (item != NULL && item->value != NULL) {
      char valueText[12];
      snprintf(valueText, sizeof(valueText), "%d", *item->value);
      u8g2_DrawStr(&u8g2, 90, 37, valueText);
    }
  } else if (items != NULL && count > 0) {
    draw1pxScrollbar(selected);
    for (int i = 0; i < visibleLines; i++) {
      int itemIndex = firstVisible + i;
      if (itemIndex >= count) break;
      const char *name = items[itemIndex].name;
      int width = u8g2_GetStrWidth(&u8g2, name);
      int x = 120 - width;
      int y = getMenuY((uint8_t)count, (uint8_t)i);
      if (itemIndex == selected)
        u8g2_DrawRFrame(&u8g2, x - 2, y - 10, width + 4, 13, 2);
      u8g2_DrawStr(&u8g2, x, y, name);
    }
  }
  u8g2_SendBuffer(&u8g2);
}

void menuUp(void)
{
  if (!pressedWithDebounce(GPIOB, UP_Pin, &lastState[upKey],
                           &lastPressTime[upKey], &LongPressupLastPressTime))
    return;
  menuUpdate = true;
  if (isEditing) {
    const MenuNode *item = selectedNode();
    if (item != NULL && item->value != NULL && *item->value < item->max)
      (*item->value)++;
    return;
  }
  int count = menuCount(currentMenu);
  if (count == 0) return;
  selected--;
  if (selected < 0) {
    selected = count - 1;
    firstVisible = count > visibleLines ? count - visibleLines : 0;
  } else if (selected < firstVisible) {
    firstVisible--;
  }
}

void menudown(void)
{
  if (!pressedWithDebounce(GPIOB, DOWN_Pin, &lastState[downKey],
                           &lastPressTime[downKey], &LongPressdownLastPressTime))
    return;
  menuUpdate = true;
  if (isEditing) {
    const MenuNode *item = selectedNode();
    if (item != NULL && item->value != NULL && *item->value > item->min)
      (*item->value)--;
    return;
  }
  int count = menuCount(currentMenu);
  if (count == 0) return;
  selected++;
  if (selected >= count) {
    selected = 0;
    firstVisible = 0;
  } else if (selected >= firstVisible + visibleLines) {
    firstVisible = selected - visibleLines + 1;
  }
}

void SelectButton(void)
{
  if (!pressedWithDebounce(GPIOB, SELECT_Pin, &lastState[selectKey],
                           &lastPressTime[selectKey], NULL))
    return;
  menuUpdate = true;
  if (isEditing) {
    isEditing = false;
    return;
  }

  const MenuNode *item = selectedNode();
  if (item == NULL) return;
  switch ((Kind)item->kind) {
    case K_MENU:
      push(item);
      break;
    case K_OPT:
      applySelectedOption();
      break;
    case K_ACTION:
      if (item->action != NULL) {
        item->action();
        pop();
      }
      break;
    case K_EDIT:
      if (item->value != NULL) isEditing = true;
      break;
    case K_NONE:
    default:
      break;
  }
}

void BackButton(void)
{
  if (!pressedWithDebounce(GPIOB, MODE_Pin, &lastState[modeKey],
                           &lastPressTime[modeKey], NULL))
    return;
  menuUpdate = true;
  if (isEditing) {
    isEditing = false;
  } else {
    pop();
  }
}

void menuSystemInit(void)
{
  currentMenu = &ROOT;
  selected = 0;
  firstVisible = 0;
  stackIndex = 0;
  isEditing = false;
  updateTitle();
  menuUpdate = true;
}

void menushow(void)
{
  menudown();
  menuUp();
  SelectButton();
  BackButton();
  if (menuUpdate) {
    drawMenu();
    menuUpdate = false;
  }
}
