#include "main.h"
#include "newglobals.h"
#include "globalfunctions.h"
#include <stdio.h>

#define LEN(a)   ((int)(sizeof(a) / sizeof((a)[0])))
#define VISIBLE  4
#define RIGHT_X  120

/* ================= settings: ONE variable per setting ================= */
typedef struct {
  uint16_t tone, contrast, language, tubeAlert, wifi, wifiReset, stopBit, levelTrig;
  uint16_t rtUnit, ptUnit, revAngle, extCtrl, baud, parity;
  uint16_t timeSet, pumpNo, dispStop, accTime, decTime;
} Settings;

#define DEFAULTS { .rtUnit = 3, .ptUnit = 3 }       /* 3 = SECONDS */
Settings cfg = DEFAULTS;
uint16_t maxSpeed = 6000;                           /* rpm x10, used by other files */

/* ---- legacy shim: the rest of the firmware still reads these.
 *      Delete each one as you switch its users over to cfg.x ---- */
bool toneproduce, wifiEnabled, levelTrigHigh, needsSave;
bool menuUpdate = true;                             /* doubles as the redraw flag */
uint16_t reverseAngle;
runtimeUnit currentRunTimeUnit = seconds, currentPauseTimeUnit = seconds;

static const uint16_t revAngles[] = {0, 180, 360, 720};

void menu2Apply(void)                               /* call once at boot too */
{
  toneproduce         = cfg.tone;
  wifiEnabled         = cfg.wifi;
  levelTrigHigh       = cfg.levelTrig;
  reverseAngle        = revAngles[cfg.revAngle];
  currentRunTimeUnit  = (runtimeUnit)cfg.rtUnit;
  currentPauseTimeUnit = (runtimeUnit)cfg.ptUnit;
  /* contrast: u8g2_SendF(&u8g2, "c", cfg.contrast ? 0xA7 : 0xA6); */
}

void menuSettingsApply(const MenuSettings *settings)
{
  if (settings == NULL)
    return;

  cfg.tone = bitGet(settings->settings1Bit, 0, 1);
  cfg.contrast = bitGet(settings->settings1Bit, 1, 1);
  cfg.language = bitGet(settings->settings1Bit, 2, 1);
  cfg.tubeAlert = bitGet(settings->settings1Bit, 3, 1);
  cfg.wifi = bitGet(settings->settings1Bit, 4, 1);
  cfg.wifiReset = bitGet(settings->settings1Bit, 5, 1);
  cfg.stopBit = bitGet(settings->settings1Bit, 6, 1);
  cfg.levelTrig = bitGet(settings->settings1Bit, 7, 1);

  cfg.rtUnit = bitGet(settings->settings2Bit, 0, 2);
  cfg.ptUnit = bitGet(settings->settings2Bit, 2, 2);
  cfg.revAngle = bitGet(settings->settings2Bit, 4, 2);
  cfg.extCtrl = bitGet(settings->settings2Bit, 6, 2);
  cfg.baud = bitGet(settings->settings2Bit, 8, 2);
  cfg.parity = bitGet(settings->settings2Bit, 10, 2);

  cfg.timeSet = bitGet(settings->settingsInt[0], 0, 10);
  cfg.pumpNo = bitGet(settings->settingsInt[1], 0, 10);
  maxSpeed = bitGet(settings->settingsInt[2], 0, 13);
  cfg.dispStop = bitGet(settings->settingsInt[3], 0, 4);
  cfg.accTime = bitGet(settings->settingsInt[4], 0, 10);
  cfg.decTime = bitGet(settings->settingsInt[5], 0, 10);

  menu2Apply();
  menuUpdate = true;
}

void menuSettingsCollect(MenuSettings *settings)
{
  if (settings == NULL)
    return;

  settings->settings1Bit = 0;
  bitsSet(settings->settings1Bit, 0, 1, cfg.tone);
  bitsSet(settings->settings1Bit, 1, 1, cfg.contrast);
  bitsSet(settings->settings1Bit, 2, 1, cfg.language);
  bitsSet(settings->settings1Bit, 3, 1, cfg.tubeAlert);
  bitsSet(settings->settings1Bit, 4, 1, cfg.wifi);
  bitsSet(settings->settings1Bit, 5, 1, cfg.wifiReset);
  bitsSet(settings->settings1Bit, 6, 1, cfg.stopBit);
  bitsSet(settings->settings1Bit, 7, 1, cfg.levelTrig);

  settings->settings2Bit = 0;
  bitsSet(settings->settings2Bit, 0, 2, cfg.rtUnit);
  bitsSet(settings->settings2Bit, 2, 2, cfg.ptUnit);
  bitsSet(settings->settings2Bit, 4, 2, cfg.revAngle);
  bitsSet(settings->settings2Bit, 6, 2, cfg.extCtrl);
  bitsSet(settings->settings2Bit, 8, 2, cfg.baud);
  bitsSet(settings->settings2Bit, 10, 2, cfg.parity);

  settings->settingsInt[0] = cfg.timeSet;
  settings->settingsInt[1] = cfg.pumpNo;
  settings->settingsInt[2] = maxSpeed;
  settings->settingsInt[3] = cfg.dispStop;
  settings->settingsInt[4] = cfg.accTime;
  settings->settingsInt[5] = cfg.decTime;
}

static void paramReset(void)
{
  cfg = (Settings)DEFAULTS;
  maxSpeed = 6000;
  menu2Apply();
  needsSave = true;
}

/* ================= menu data ================= */
typedef struct {
  const char *name;
  uint16_t   *val;                 /* NULL = not editable                    */
  uint16_t    min, max;
  const char *const *labels;       /* non-NULL = choice (labels[*val])       */
  void      (*fn)(void);           /* val == NULL && fn != NULL = action     */
} Item;

typedef struct { const char *name, *title; const Item *items; uint8_t count; } Category;

#define NUM(n, v, lo, hi)  { n, &(v), lo, hi, NULL, NULL }
#define CHOICE(n, v, l)    { n, &(v), 0, LEN(l) - 1, l, NULL }
#define INFO(n)            { n, NULL, 0, 0, NULL, NULL }
#define ACTION(n, f)       { n, NULL, 0, 0, NULL, f }

/* index 1 = "on" for every bool-like list, so the value IS the bool */
static const char *const L_TONE[]   = {"DISABLE", "ENABLE"};
static const char *const L_CONTR[]  = {"DARK", "LIGHT"};
static const char *const L_LANG[]   = {"ENGLISH", "HINDI"};
static const char *const L_YESNO[]  = {"NO", "YES"};
static const char *const L_WIFI[]   = {"DISABLED", "ENABLE"};
static const char *const L_STOP[]   = {"1", "2"};
static const char *const L_LEVEL[]  = {"LOW", "HIGH"};
static const char *const L_UNIT[]   = {"DAYS", "HOURS", "MINUTES", "SECONDS"};
static const char *const L_REV[]    = {"0", "180", "360", "720"};
static const char *const L_EXT[]    = {"CURRENT", "0-5V", "0-10V"};
static const char *const L_BAUD[]   = {"4800", "9600", "19200", "38400"};
static const char *const L_PARITY[] = {"NONE", "EVEN", "ODD"};


static const Item generalItems[] = {
  NUM   ("TIMESET",    cfg.timeSet,  0, 999),
  CHOICE("LANGUAGE",   cfg.language, L_LANG),
  CHOICE("CONTRAST",   cfg.contrast, L_CONTR),
  CHOICE("TONE SET",   cfg.tone,     L_TONE),
};
static const Item functionalItems[] = {
  CHOICE("RT UNIT",    cfg.rtUnit,   L_UNIT),
  CHOICE("PT UNIT",    cfg.ptUnit,   L_UNIT),
  CHOICE("REV ANGLE",  cfg.revAngle, L_REV),
  NUM   ("PUMP NO.",   cfg.pumpNo,   0, 999),
  NUM   ("MAX SPEED",  maxSpeed,     MIN_RPM10, MAX_RPM10),
  CHOICE("TUBE ALERT", cfg.tubeAlert, L_YESNO),
};
static const Item systemItems[] = {
  CHOICE("BAUD",        cfg.baud,     L_BAUD),
  CHOICE("PARITY",      cfg.parity,   L_PARITY),
  INFO  ("PLS TRIG"),
  CHOICE("LVL TRIG",    cfg.levelTrig, L_LEVEL),
  NUM   ("DISP STOP",   cfg.dispStop, 0, 10),
  NUM   ("ACC TIME",    cfg.accTime,  0, 999),
  NUM   ("DEC TIME",    cfg.decTime,  0, 999),
  ACTION("PARAM RESET", paramReset),
  CHOICE("STOP BIT",    cfg.stopBit,  L_STOP),
};
static const Item externalItems[] = {
  CHOICE("WIFI SET",    cfg.wifi,      L_WIFI),
  CHOICE("WIFI RST",    cfg.wifiReset, L_YESNO),
  CHOICE("EXT CONT",    cfg.extCtrl,   L_EXT),
  INFO  ("BOOT V3.1.0"),
  INFO  ("USER v3.2.3"),
};

static const Category cats[] = {
  {"GENERAL",    "GEN",  generalItems,    LEN(generalItems)},
  {"FUNCTIONAL", "FUNC", functionalItems, LEN(functionalItems)},
  {"SYSTEM",     "SYS",  systemItems,     LEN(systemItems)},
  {"EXTERNAL",   "EXT",  externalItems,   LEN(externalItems)},
};

/* ================= state + key handling (no drawing here) ================= */
typedef enum { K_UP, K_DOWN, K_SEL, K_BACK } Key;
static int  cat, item, first;
static bool inItems, editing;

static int wrap(int v, int n) { return (v % n + n) % n; }

static void clampWindow(int sel)
{
  if      (sel < first)           first = sel;
  else if (sel >= first + VISIBLE) first = sel - VISIBLE + 1;
}

static void menu2Key(Key k)
{
  const Category *c = &cats[cat];
  menuUpdate = true;

  if (editing) {
    const Item *it = &c->items[item];
    if (k == K_UP || k == K_DOWN) {
      int d = (k == K_UP) ? 1 : -1;
      if (it->labels) *it->val = wrap(*it->val + d, it->max + 1);
      else            *it->val = constrainInt32(*it->val + d, it->min, it->max);
      menu2Apply();
      needsSave = true;
    } else {
      editing = false;                              /* SEL or BACK */
    }
  }
  else if (inItems) {
    if (k == K_UP)        { item = wrap(item - 1, c->count); clampWindow(item); }
    else if (k == K_DOWN) { item = wrap(item + 1, c->count); clampWindow(item); }
    else if (k == K_SEL) {
      const Item *it = &c->items[item];
      if (it->val)     editing = true;
      else if (it->fn) it->fn();
    }
    else                  { inItems = false; first = 0; }   /* BACK */
  }
  else {
    if (k == K_UP)        cat = wrap(cat - 1, LEN(cats));
    else if (k == K_DOWN) cat = wrap(cat + 1, LEN(cats));
    else if (k == K_SEL)  { inItems = true; item = 0; first = 0; }
    /* BACK at top level: nothing, MENU key closes (menuButtonHandle) */
  }
}

/* ================= drawing (reads state, never changes it) ================= */
static int rowY(int n, int i)
{
  static const uint8_t y[][5] = {
    { 38 }, { 22, 54 }, { 17, 38, 59 }, { 12, 28, 44, 59 }, { 12, 24, 36, 48, 60 }
  };
  n = (n < 1) ? 1 : (n > 5 ? 5 : n);
  return y[n - 1][i];
}

static void drawScrollbar(int n, int sel)
{
  if (n <= VISIBLE) return;
  int h   = (VISIBLE * 63) / n;
  if (h < 3) h = 3;
  int top = (sel * (63 - h)) / (n - 1);
  u8g2_DrawLine(&u8g2, 126, top, 126, top + h - 1);
}

void menu2Draw(void)
{
  const Category *c = &cats[cat];
  const Item *it = editing ? &c->items[item] : NULL;
  char buf[12];

  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);

  /* title pill */
  const char *title = editing ? it->name : (inItems ? c->title : "MENU");
  u8g2_DrawRBox(&u8g2, 2, 27, u8g2_GetStrWidth(&u8g2, title) + 4, 13, 4);
  u8g2_SetDrawColor(&u8g2, 0);
  u8g2_DrawStr(&u8g2, 4, 37, title);
  u8g2_SetDrawColor(&u8g2, 1);

  if (editing) {
    if (it->labels) {
      int optionCount = it->max + 1;

      for (int i = 0; i < optionCount && i < VISIBLE; i++) {
        const char *s = it->labels[i];
        int w = u8g2_GetStrWidth(&u8g2, s);
        int x = RIGHT_X - w, y = rowY(optionCount, i);

        if (i == *it->val) u8g2_DrawRFrame(&u8g2, x - 2, y - 10, w + 4, 13, 2);
        u8g2_DrawStr(&u8g2, x, y, s);
      }
    } else {
      snprintf(buf, sizeof buf, "%u", *it->val);
      u8g2_DrawStr(&u8g2, RIGHT_X - u8g2_GetStrWidth(&u8g2, buf), 37, buf);
    }
  } else {                                          /* list on the right */
    int n   = inItems ? c->count : LEN(cats);
    int sel = inItems ? item : cat;
    drawScrollbar(n, sel);

    for (int i = 0; i < VISIBLE && first + i < n; i++) {
      int idx = first + i;
      const char *s = inItems ? c->items[idx].name : cats[idx].name;
      int w = u8g2_GetStrWidth(&u8g2, s);
      int x = RIGHT_X - w, y = rowY(VISIBLE, i);
      if (idx == sel) u8g2_DrawRFrame(&u8g2, x - 2, y - 10, w + 4, 13, 2);
      u8g2_DrawStr(&u8g2, x, y, s);
    }
  }
  u8g2_SendBuffer(&u8g2);
}

/* ================= glue: one call from the task loop ================= */
void menu2Poll(void)
{
  if (pressedWithDebounce(GPIOB, SELECT_Pin, &lastState[selectKey], &lastPressTime[selectKey], NULL))                         menu2Key(K_SEL);
  if (pressedWithDebounce(GPIOB, MODE_Pin,   &lastState[modeKey],   &lastPressTime[modeKey],   NULL))                         menu2Key(K_BACK);
  if (pressedWithDebounce(GPIOB, UP_Pin,     &lastState[upKey],     &lastPressTime[upKey],     &LongPressupLastPressTime))   menu2Key(K_UP);
  if (pressedWithDebounce(GPIOB, DOWN_Pin,   &lastState[downKey],   &lastPressTime[downKey],   &LongPressdownLastPressTime)) menu2Key(K_DOWN);

  if (menuUpdate) { menu2Draw(); menuUpdate = false; }
}
