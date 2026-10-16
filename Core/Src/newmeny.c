#include "main.h"
#include "newglobals.h"
#include "u8g2.h"
#include "globalfunctions.h"

typedef struct {
    const char *name;
    uint16_t *val;
    uint16_t min, max;
    const char *const *labels;
} Item;

typedef struct {
    const char *name;
    const Item *items;
    uint8_t count;
} Category;

static uint16_t baudIdx, accTime;
static const char *const baudLbl[] = {"4800", "9600", "19200", "38400"};

static const Item sysItems[] = {
    {"BAUD", &baudIdx, 0, 3,   baudLbl},
    {"ACC",  &accTime, 0, 999, NULL},
};
static const Category cats[] = { {"SYSTEM", sysItems, 2} };
#define COUNT_CATS ((int)(sizeof cats / sizeof cats[0]))

typedef enum { K_UP, K_DOWN, K_SEL, K_BACK } Key;
static int  cat, item;
static bool inItems, editing, dirty = true;

static int wrap(int v, int n) { return (v % n + n) % n; }

bool menu2Key(Key k)
{
    const Category *c = &cats[cat];
    dirty = true;

    if (editing) {
        const Item *it = &c->items[item];
        if      (k == K_UP   && *it->val < it->max) (*it->val)++;
        else if (k == K_DOWN && *it->val > it->min) (*it->val)--;
        else if (k == K_SEL || k == K_BACK)         editing = false;
    }
    else if (inItems) {
        if      (k == K_UP)   item = wrap(item - 1, c->count);
        else if (k == K_DOWN) item = wrap(item + 1, c->count);
        else if (k == K_SEL)  editing = true;
        else if (k == K_BACK) inItems = false;
    }
    else {
        if      (k == K_UP)   cat = wrap(cat - 1, COUNT_CATS);   // was item - 1
        else if (k == K_DOWN) cat = wrap(cat + 1, COUNT_CATS);
        else if (k == K_SEL)  { inItems = true; item = 0; }
        else if (k == K_BACK) return false;
    }
    return true;
}

void menu2Draw(void)
{
    char buf[24];
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);

    if (!inItems) {
        for (int i = 0; i < COUNT_CATS; i++) {
            u8g2_DrawStr(&u8g2, 0,  12 + i * 12, i == cat ? ">" : " ");
            u8g2_DrawStr(&u8g2, 10, 12 + i * 12, cats[i].name);
        }
    } else {
        const Category *c = &cats[cat];
        for (int i = 0; i < c->count; i++) {
            const Item *it = &c->items[i];
            if (it->labels) snprintf(buf, sizeof buf, "%s: %s", it->name, it->labels[*it->val]);
            else            snprintf(buf, sizeof buf, "%s: %u", it->name, *it->val);
            u8g2_DrawStr(&u8g2, 0,  12 + i * 12, i == item ? (editing ? "*" : ">") : " ");
            u8g2_DrawStr(&u8g2, 10, 12 + i * 12, buf);
        }
    }
    u8g2_SendBuffer(&u8g2);
}

void menu2Poll(void)
{
    if (pressedWithDebounce(GPIOB, UP_Pin,     &lastState[upKey],     &lastPressTime[upKey],     &LongPressupLastPressTime))   menu2Key(K_UP);
    if (pressedWithDebounce(GPIOB, DOWN_Pin,   &lastState[downKey],   &lastPressTime[downKey],   &LongPressdownLastPressTime)) menu2Key(K_DOWN);
    if (pressedWithDebounce(GPIOB, SELECT_Pin, &lastState[selectKey], &lastPressTime[selectKey], NULL))                        menu2Key(K_SEL);
    if (pressedWithDebounce(GPIOB, MODE_Pin,   &lastState[modeKey],   &lastPressTime[modeKey],   NULL)) {
        if (!menu2Key(K_BACK)) { changeMenu = false; modeUpdate = true; menuUpdateMain = true; homeScrenUpdate = true; }
    }
    if (dirty) { menu2Draw(); dirty = false; }
}