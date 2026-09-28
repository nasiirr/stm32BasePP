/*
 * stepperPP.h
 *
 *  Created on: Sep 30, 2025
 *      Author: Dell
 *
 *  DECLARATIONS ONLY -- include-safe. All variable storage and function bodies
 *  live in Core/Src/stepperPP.c, so this header may be #included from any number
 *  of translation units without multiple-definition linker errors.
 */

#ifndef APPLICATION_USER_CORE_STEPPERPP_H_
#define APPLICATION_USER_CORE_STEPPERPP_H_

//#define _SMPP_STEPPERPIN 26
//#define _SMPP_DIRECTIONPIN 27
#define _SMPP_SPR 1600                      // was 12800
#define _SMPP_MINVEL 20                     // unchanged
#define _SMPP_MAXVEL ((_SMPP_SPR*1000)/60)  // ≈26667 Hz, was SPR*12
#define _SMPP_ACCELHZ 20                    // unchanged — matches TIM4 update rate
#define _SMPP_ACCEL_TIME_S   1U      // startup ramp time from MINVEL to MAXVEL
#define _SMPP_DECEL_TIME_MS  300U    // stop ramp time from MAXVEL to MINVEL (300ms full-range => ~0.3s from 600 RPM to 0)
#define _SMPP_ACCEL  (((_SMPP_MAXVEL) - (_SMPP_MINVEL)) / (_SMPP_ACCEL_TIME_S * _SMPP_ACCELHZ))
#define _SMPP_DECEL  ((((_SMPP_MAXVEL) - (_SMPP_MINVEL)) * 1000U) / (_SMPP_DECEL_TIME_MS * _SMPP_ACCELHZ))
// #define _SMPP_ACCEL ((_SMPP_SPR*15)/_SMPP_ACCELHZ)   // match accstepp's ramp rate
#define _SMPP_T1PRE 1200000                 // was 275000000 — TIM2 tick after PSC=59
/* State (defined in stepperPP.c) */
extern volatile int          _SMPP_to_velocity;
extern volatile int          _SMPP_cur_velocity;
extern volatile int          _SMPP_to_velocity1;
extern volatile int          _SMPP_cur_velocity1;
extern volatile int          _SMPP_decStepsCounter;
extern volatile unsigned int _SMPP_cur_Ocr;
extern volatile int          _SMPP_OFFHIGH;
extern volatile unsigned int _SMPP_stepsCounter;
extern volatile unsigned int _SMPP_toStepsCounter;
extern volatile int          _SMPP_autoStop;
extern volatile int          _SMPP_stopFlag;
extern volatile int          _SMPP_stepFlip;
extern int                   _SMPP_curDirection;

/* API (defined in stepperPP.c) */
void _SMPP_stepsGen(void);            /* call from TIM2 update ISR (after setting ARR=_SMPP_cur_Ocr) */
void _SMPP_accelGen(void);            /* call from TIM4 update ISR @_SMPP_ACCELHZ */
void _SMPP_runReverseAngle(int ang, int rpmr);
void _SMPP_setVel(int v);
void _SMPP_setRPM(float rpm);
void _SMPP_setRPMDur(float rpm, unsigned long dur);
void _SMPP_stop_int(void);
void _SMPP_stop(void);
int  _SMPP_isRunning(void);
int  _SMPP_isStopping(void);
void _SMPP_setDir(int dir);
int  _SMPP_fdaCalc(int v, int a, int s);

#endif /* APPLICATION_USER_CORE_STEPPERPP_H_ */