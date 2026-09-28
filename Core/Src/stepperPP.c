/*
 * stepperPP.c
 *
 *  Definitions for the stepper pulse/acceleration module declared in stepperPP.h.
 *  (Split out of the header so the header is include-safe across translation units.)
 */
// #include "tim.h"
#include "stepperPP.h"
#include "main.h"   /* HAL_GPIO_WritePin, GPIO_PIN_, STEPPER_ pin macros, GPIOD/GPIOF */
#include <stdbool.h>
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
/* ---- State ---- */
volatile int          _SMPP_to_velocity   = _SMPP_MINVEL;
volatile int          _SMPP_cur_velocity  = _SMPP_MINVEL;
volatile int          _SMPP_to_velocity1  = _SMPP_MINVEL;
volatile int          _SMPP_cur_velocity1 = _SMPP_MINVEL;
volatile int          _SMPP_decStepsCounter = 0;
/* FIX: ARR must be computed the same way accelGen() computes it — one timer
 * period per pin toggle, two toggles per full step, hence (vel*2). The old
 * init (T1PRE/MINVEL - 1) ran the very first period at 2x the intended rate
 * until the first accelGen() tick silently corrected it. */
volatile unsigned int _SMPP_cur_Ocr       = (_SMPP_T1PRE / (_SMPP_MINVEL * 2)) - 1;   /* ARR = period-1 */
volatile int          _SMPP_OFFHIGH       = 0;
volatile unsigned int _SMPP_stepsCounter  = 0;
volatile unsigned int _SMPP_toStepsCounter = -1;
volatile int          _SMPP_autoStop      = 0;
volatile int          _SMPP_stopFlag      = 0;
volatile int          _SMPP_stepFlip      = 0;
int                   _SMPP_curDirection  = 5;

/* Taper the ramp near the target so acceleration and deceleration finish
 * without overshooting the requested velocity. */
#define _SMPP_TAPER_ZONE(step) ((step) * 4)   /* width of the taper zone, in velocity units */
#define _SMPP_TAPER_DIVISOR 3                  /* step is /this inside the taper zone */

/* ---- API ---- */
void _SMPP_stepsGen(void){
  if(_SMPP_stepFlip){
    _SMPP_stepFlip = 0;
    HAL_GPIO_WritePin(STEPPER_STEPS_GPIO_Port, STEPPER_STEPS_Pin, GPIO_PIN_RESET);
    //resetTimer(_SMPP_stepsTimer, _SMPP_cur_Ocr, true);
  } else if(_SMPP_toStepsCounter==-1 || _SMPP_stepsCounter<_SMPP_toStepsCounter){
    _SMPP_stepFlip = 1;
    /* FIX: explicit cast — HAL_GPIO_WritePin's 3rd param is GPIO_PinState, not int */
    HAL_GPIO_WritePin(STEPPER_STEPS_GPIO_Port, STEPPER_STEPS_Pin, (GPIO_PinState)_SMPP_OFFHIGH);
    _SMPP_stepsCounter++;
  }
}

void _SMPP_accelGen(void){
  if(_SMPP_to_velocity != _SMPP_cur_velocity){
    int velocityGap = _SMPP_to_velocity - _SMPP_cur_velocity;
    int velocityMagnitude = velocityGap < 0 ? -velocityGap : velocityGap;
    int rampStep = velocityGap > 0 ? _SMPP_ACCEL : _SMPP_DECEL;
    int taperZone = _SMPP_TAPER_ZONE(rampStep);

    if(velocityMagnitude < taperZone){
      rampStep /= _SMPP_TAPER_DIVISOR;
      if(rampStep < 1) rampStep = 1;
    }
    if(rampStep > velocityMagnitude) rampStep = velocityMagnitude;
    _SMPP_cur_velocity += velocityGap >= 0 ? rampStep : -rampStep;
    _SMPP_cur_Ocr = ((_SMPP_T1PRE)/(_SMPP_cur_velocity*2)) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim2, _SMPP_cur_Ocr);
    if (__HAL_TIM_GET_COUNTER(&htim2) > _SMPP_cur_Ocr) {
      __HAL_TIM_SET_COUNTER(&htim2, 0);
    }

    if(_SMPP_cur_velocity == _SMPP_MINVEL && _SMPP_stopFlag){
      _SMPP_OFFHIGH = GPIO_PIN_RESET;
      _SMPP_stopFlag = 0;
      HAL_GPIO_WritePin(STEPPER_STEPS_GPIO_Port, STEPPER_STEPS_Pin, GPIO_PIN_RESET);
      _SMPP_stepFlip = 0;
    }
  }

  if(_SMPP_autoStop && _SMPP_stepsCounter >= _SMPP_toStepsCounter){
    _SMPP_autoStop = 0;
    _SMPP_stop_int();
  }
}
void _SMPP_runReverseAngle(int ang, int rpmr){
  if(ang <= 0)
    return;

  uint32_t stopWaitStart = HAL_GetTick();

  /* A reverse move must not overwrite a pending deceleration request. */
  while(_SMPP_stopFlag){
    if((HAL_GetTick() - stopWaitStart) > 5000U)
      break;
    HAL_Delay(1);
  }

  _SMPP_to_velocity=_SMPP_MINVEL;_SMPP_cur_velocity=_SMPP_MINVEL;
  ang = (_SMPP_SPR*ang/360);
  // rpmr *= _SMPP_SPR;
  int targetVelocity = (rpmr * _SMPP_SPR) / 60;
  _SMPP_OFFHIGH = 1;
  _SMPP_cur_velocity1 = _SMPP_ACCEL;
  _SMPP_to_velocity1 = _SMPP_fdaCalc(targetVelocity,_SMPP_ACCEL,ang);
  /* FIX: same period formula as accelGen() — was missing the *2 and the -1,
   * which ran reverse-angle moves at ~2x the intended period (half speed). */
  int masterFreq1 = ((_SMPP_T1PRE)/(_SMPP_cur_velocity1*2)) - 1;
  _SMPP_cur_Ocr = masterFreq1;
  /* FIX: ported from ESP32-Arduino timerAlarm()/_SMPP_stepsTimer (undeclared
   * on this target) to STM32 HAL — reuses htim2, same as accelGen(). */
  __HAL_TIM_SET_AUTORELOAD(&htim2, _SMPP_cur_Ocr);
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  HAL_TIM_Base_Start_IT(&htim2);
  int stepsAcc = 0;
  _SMPP_stepsCounter = 0;
  _SMPP_toStepsCounter = ang;
  char update = false;
  _SMPP_OFFHIGH = 1;
  while(_SMPP_stepsCounter<_SMPP_toStepsCounter){
    if(_SMPP_stepsCounter>=(_SMPP_decStepsCounter) && _SMPP_to_velocity1!=_SMPP_ACCEL){
      _SMPP_cur_velocity1=_SMPP_to_velocity1;_SMPP_to_velocity1=_SMPP_ACCEL;
      stepsAcc = _SMPP_decStepsCounter;
      update=true;
    }
    if(update){
      update=false;
      stepsAcc += (2*_SMPP_cur_velocity1)/(_SMPP_ACCELHZ);
      //Serial.print("curvel:");Serial.print(_SMPP_cur_velocity1);Serial.print(" s@cv:");Serial.print(2*_SMPP_cur_velocity1/(_SMPP_ACCELHZ));//think
      if(_SMPP_cur_velocity1<_SMPP_to_velocity1) _SMPP_cur_velocity1+=_SMPP_ACCEL;
      else if(_SMPP_cur_velocity1>_SMPP_to_velocity1) _SMPP_cur_velocity1-=_SMPP_ACCEL;
      /* FIX: same *2/-1 correction as above, applied on every recompute */
      masterFreq1 = ((_SMPP_T1PRE)/(_SMPP_cur_velocity1*2)) - 1;
      //Serial.print(" tovel:");Serial.print(_SMPP_cur_velocity1);Serial.print(" stepsAcc:");Serial.print(stepsAcc);Serial.print(" stepsCounter:");Serial.print(_SMPP_stepsCounter);
      //Serial.print("(");Serial.print(_SMPP_toStepsCounter-_SMPP_stepsCounter);Serial.print(")");Serial.println();
    }
    if(_SMPP_stepsCounter>=stepsAcc){
      /* FIX: ported to HAL — was timerAlarm(_SMPP_stepsTimer, masterFreq1, true, 0) */
      __HAL_TIM_SET_AUTORELOAD(&htim2, masterFreq1);
      update=true;
    }
    if(_SMPP_stepsCounter>=_SMPP_toStepsCounter) break;
  }
  _SMPP_OFFHIGH = 0;
  _SMPP_toStepsCounter=-1;
}
void _SMPP_setVel(int v){
  /* FIX: _SMPP_MAXVEL (1000 RPM ceiling) was defined but never enforced —
   * a bad input or future menu bug could ramp past it with nothing to
   * stop it. Limit kept at its existing value, just now applied. */
  if (v > _SMPP_MAXVEL) v = _SMPP_MAXVEL;
  if (v < _SMPP_MINVEL) v = _SMPP_MINVEL;
  _SMPP_to_velocity = v;
  _SMPP_OFFHIGH = GPIO_PIN_SET;
  _SMPP_toStepsCounter = -1;
}
void _SMPP_setRPM(float rpm){
  _SMPP_stopFlag = 0;          // a new command always overrides a pending stop
  if(rpm<.1) _SMPP_setVel((.1/60.0)*_SMPP_SPR);
  else _SMPP_setVel((rpm/60.0)*_SMPP_SPR);
}
void _SMPP_stop(void){
  if(_SMPP_cur_velocity <= _SMPP_MINVEL){
    _SMPP_to_velocity = _SMPP_MINVEL;
    _SMPP_OFFHIGH = GPIO_PIN_RESET;
    _SMPP_stopFlag = 0;
    HAL_GPIO_WritePin(STEPPER_STEPS_GPIO_Port, STEPPER_STEPS_Pin, GPIO_PIN_RESET);
    _SMPP_stepFlip = 0;
    return;
  }

  _SMPP_stop_int();
}
void _SMPP_stop_int(void){
  _SMPP_to_velocity = _SMPP_MINVEL;
  _SMPP_toStepsCounter=-1;

  if(_SMPP_cur_velocity <= _SMPP_MINVEL){
    _SMPP_cur_velocity = _SMPP_MINVEL;
    _SMPP_OFFHIGH = GPIO_PIN_RESET;
    _SMPP_stopFlag = 0;
    HAL_GPIO_WritePin(STEPPER_STEPS_GPIO_Port, STEPPER_STEPS_Pin, GPIO_PIN_RESET);
    _SMPP_stepFlip = 0;
  } else {
    _SMPP_stopFlag = 1;
  }
}
int _SMPP_isRunning(void){
  return _SMPP_OFFHIGH;
}
int _SMPP_isStopping(void){
  return _SMPP_stopFlag;
}
void _SMPP_setDir(int dir){
    _SMPP_curDirection=dir;
    if(!dir)
        HAL_GPIO_WritePin(STEPPER_DIR_GPIO_Port, STEPPER_DIR_Pin, GPIO_PIN_RESET);
    else HAL_GPIO_WritePin(STEPPER_DIR_GPIO_Port, STEPPER_DIR_Pin, GPIO_PIN_SET);
}

int _SMPP_fdaCalc(int v, int a, int s){
  int v1 = a;
  int s1=0;
  // int secs = 0;
  while(1){
    s1+=v1/_SMPP_ACCELHZ;
    if(s1>s/2){
      if(s1>v1/_SMPP_ACCELHZ) s1-=v1/_SMPP_ACCELHZ; else s1=s-1;
      if(v1>a) v1-=a;
      break;
    }
    if(v1>=v) break;
    v1 += a;
    if(v1>v) break;
  }
  _SMPP_decStepsCounter = s-s1;
  return v1;
}