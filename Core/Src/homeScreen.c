
#include<newglobals.h>

extern u8g2_t u8g2;

bool common = true;
uint8_t modecounter = 0;



void status()
{
  u8g2_SetDrawColor(&u8g2, 0);
  u8g2_DrawBox(&u8g2, 50, 38, 15, 15);
  u8g2_SetDrawColor(&u8g2, 1);

  if (_SMPP_isRunning())
  // if(!DispenseRunning)
  {    u8g2_DrawTriangle(&u8g2, 50, 38, 60, 45, 50, 52);
    // u8g2.updateDisplayArea(50, 38, 15, 15);
  }
  else
  {
 u8g2_DrawRBox(&u8g2, 52, 40, 12, 12, 3);
    // u8g2.updateDisplayArea(6, 4, 2, 2); 
  }
  // u8g2.sendBuffer();
}

void wifi2(bool x)
{
  u8g2_DrawXBM(&u8g2, 5, 5, 20, 20, wifiLogo);
  if (!x)
  {
    u8g2_DrawLine(&u8g2, 8, 19, 20, 7);
  }
  // u8g2.sendBuffer();
}

// void modes(int x)
// {
//   u8g2.setDrawColor(0);
//   u8g2.drawBox(29, 5, 32, 15);
//   u8g2.setDrawColor(1);
//   u8g2.setFont(u8g2_font_6x12_tr);
//   u8g2.drawStr(29, 20, modesarr[x]);
// }
void RPM(int x)
{
  u8g2_DrawRBox(&u8g2, 62, 8, 62, 16, 2);
  // u8g2.updateDisplayArea(70, 7, 60, 16);

  u8g2_SetDrawColor(&u8g2, 0);

  char rpmText[16];
  (void)sprintf(rpmText, "%d RPM", x);
  u8g2_DrawStr(&u8g2, 64, 20, rpmText);
  u8g2_SetDrawColor(&u8g2, 1);
}

void sound1(bool x)
{
  if (x)
  {
    u8g2_DrawXBM(&u8g2, 10, 38, 15, 15, sound);
    // u8g2.updateDisplayArea(10, 38, 15, 15);
  }
  if (!x)
  {
    u8g2_DrawXBM(&u8g2, 10, 38, 15, 15, nosound);
    // u8g2.updateDisplayArea(10, 38, 15, 15);
  }
}


void rotation(){
  bool dirState = (modecounter == 5 || modecounter == 2) ? currDirectionDB15 : clockwise;
  if (dirState) {
    u8g2_DrawXBM(&u8g2, 30, 38, 15, 15, rotationTrue);
  } else {
    u8g2_DrawXBM(&u8g2, 30, 38, 15, 15, rotationFalse);
  }
}

void time()
{
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
  u8g2_DrawStr(&u8g2, 70, 42, "2.3 secs");
  u8g2_DrawStr(&u8g2, 70, 55, "8 cycles");
}
int rpmm = 100;
void drawhome(){
  //   u8g2_ClearBuffer(&u8g2);
  //       wifi2(1);
  //    sound1(1);
  //     rotation();
  //     status();
  //     RPM(rpmm);
  //     rpmm++;
  //     if (rpmm > 600) {
  //       rpmm = 100;
  //     }
  //     time();
  // u8g2_SendBuffer(&u8g2);
}


