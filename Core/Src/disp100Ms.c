#include"main.h"
#include "newglobals.h"
#include "globalfunctions.h"
#include "stepperPP.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"


#define DISP_EVT_REVERSE   (1U << 0)
#define DISP_EVT_FINISHED  (1U << 1)
#define DISP_EVT_STOPPED   (1U << 2)

volatile uint8_t dispEvt = 0;
typedef enum {
    D_IDLE,
    D_RUN,
    D_PAUSE,
    D_REVERSING
} DispPhase ;

volatile DispPhase phase = D_IDLE ;

volatile uint32_t phaseRemaining = 0;
volatile uint32_t totalRemaining = 0;
volatile uint32_t cyclesLeft = 0;


volatile bool startReq = false ;
volatile bool stopReq = false ;

uint32_t runTicks = 0 ;
uint32_t pauseTicks = 0;
uint32_t totalTicks = 0;



uint8_t  Disp_TakeEvents(void){
    uint8_t events = dispEvt;
    dispEvt = 0U;
    return events ;
}


void Disp_CompleteReverse(void ){
    if(phase!= D_REVERSING)
    return;
    _SMPP_setDir(!clockwise);
    _SMPP_runReverseAngle(reverseAngle,rpm);
    _SMPP_setDir(clockwise);

    if(dispCycles !=0 && cyclesLeft == 1U){
        phase = D_IDLE;
        DispenseRunning = false ;
        dispEvt |= DISP_EVT_FINISHED;
    }
    else {
        phase = D_PAUSE ;
        phaseRemaining = pauseTicks ;
        PauseStartTime = HAL_GetTick();
    }
}



void DispTick100ms(){
    if(stopReq){
        stopReq = false ;

        if (phase != D_IDLE){
            _SMPP_stop_int();

            pumpRunning = false ;
            DispenseRunning = false ;
            
            phase  = D_IDLE ;

            dispEvt |= DISP_EVT_STOPPED ;
        }
             return;
    }

    if(startReq){
        startReq = false ;

        if (phase  == D_IDLE){
            phase = D_RUN ;

            phaseRemaining = runTicks;
            totalRemaining = totalTicks;

            cyclesLeft = dispCycles ;

            pumpRunning = true ;
            DispenseRunning = true ;
            CycleElapsed = dispCycles;
            PumpRunningTime = HAL_GetTick();

            _SMPP_setDir(clockwise);
            _SMPP_setRPM(rpm);
        }
        return ;

    }

    if(phase==D_IDLE||phase == D_REVERSING)
    return;

    if(dispCycles!=0 && totalRemaining >0) 
    totalRemaining--;

    if (phaseRemaining>0)
    phaseRemaining --;

    if (phase==D_RUN && phaseRemaining == 0)
    {
        pumpRunning = false ;
        PauseStartTime = HAL_GetTick();

        _SMPP_stop_int();

        if(reverseAngle>0){
            phase = D_REVERSING;

            dispEvt |= DISP_EVT_REVERSE;

        }

        else if(dispCycles!=0 && cyclesLeft == 1U){

            phase = D_IDLE;
            DispenseRunning = false ;
            dispEvt |= DISP_EVT_FINISHED ;
        }

        else {
            phase = D_PAUSE ;
            phaseRemaining = pauseTicks;
        }
    }
    

    else if (phase == D_PAUSE && phaseRemaining == 0) {
        if(dispCycles!=0){
            cyclesLeft--;
        }

        phase = D_RUN;
        phaseRemaining = runTicks;

        pumpRunning = true ;
        PumpRunningTime = HAL_GetTick();
        if (dispCycles!=0){
            CycleElapsed--;
        }

        _SMPP_setDir(clockwise);
        _SMPP_setRPM(rpm);
    
    }
    
   
}


void tickDisp(){



    if (pressedWithDebounce(GPIOB,
                            START_Pin,
                            &lastState[startStop],
                            &lastPressTime[startStop],
                            NULL)) {
                                

        if (!DispenseRunning) {

            // savedispenseSettings();

            runTicks = dispRuntime() ;
            pauseTicks = dispPauseTime() ;

            if (dispCycles == 0) {
                totalTicks = 0;   // infinite mode
            }
            else {
                totalTicks =
                    ((uint64_t)runTicks * dispCycles) +
                    ((uint64_t)pauseTicks * (dispCycles - 1));
            }

            startReq = true;
        }
        else {

            stopReq = true;
        }
    }
}


void Disp_OnTimer100ms(TIM_HandleTypeDef *htim){
    if(htim != NULL && htim->Instance == TIM1) 
    DispTick100ms();
}