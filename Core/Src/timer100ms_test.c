/*
 * TIM1 100 ms dispense timing
 *
 * TIM1 owns the run/pause timing. Reverse-angle movement remains in the task
 * context because it waits for the stepper to decelerate.
 *
 * Call Timer100ms_TestOnPeriodElapsed() from the existing
 * HAL_TIM_PeriodElapsedCallback() for TIM1. This file must not define a
 * second HAL callback because the interrupt source already provides one.
 */

#include "main.h"
#include "newglobals.h"
#include "globalfunctions.h"

typedef enum
{
    D_IDLE,
    D_RUN,
    D_PAUSE,
    D_REVERSING
} DispPhase;

volatile DispPhase phase = D_IDLE;

volatile uint32_t phaseRemaining = 0;
volatile uint32_t totalRemaining = 0;
volatile uint32_t cyclesLeft = 0;

volatile uint8_t dispEvt = 0;

volatile bool startReq = false;
volatile bool stopReq = false;

uint32_t runTicks = 0;
uint32_t pauseTicks = 0;
uint32_t totalTicks = 0;

bool infiniteCycles = false;

void Timer100ms_TestOnPeriodElapsed(TIM_HandleTypeDef *htim)
{
    if (htim != NULL && htim->Instance == TIM1)
        Disp_Tick100ms();
}

uint8_t Disp_TakeEvents(void)
{
    uint8_t events = dispEvt;
    dispEvt = 0U;
    return events;
}

void Disp_CompleteReverse(void)
{
    if (phase != D_REVERSING)
        return;

    _SMPP_setDir(!clockwise);
    _SMPP_runReverseAngle(reverseAngle, rpm10 / 10U);
    _SMPP_setDir(clockwise);

    if (!infiniteCycles && cyclesLeft == 1U)
    {
        phase = D_IDLE;
        DispenseRunning = false;
        dispEvt |= DISP_EVT_FINISHED;
    }
    else
    {
        phase = D_PAUSE;
        phaseRemaining = pauseTicks;
        PauseStartTime = HAL_GetTick();
    }
}

void Disp_Tick100ms(void)
{
    /* -------------------------------------------------
     * STOP REQUEST
     * ------------------------------------------------- */
    if (stopReq)
    {
        stopReq = false;

        if (phase != D_IDLE)
        {
            /*
             * Stop the normal dispense operation immediately.
             * _SMPP_stop_int() only requests deceleration;
             * TIM4 continues handling the deceleration.
             */
            _SMPP_stop_int();

            pumpRunning = false;
            DispenseRunning = false;

            phase = D_IDLE;

            dispEvt |= DISP_EVT_STOPPED;
        }

        return;
    }


    /* -------------------------------------------------
     * START REQUEST
     * ------------------------------------------------- */
    if (startReq)
    {
        startReq = false;

        if (phase == D_IDLE)
        {
            phase = D_RUN;

            phaseRemaining = runTicks;
            totalRemaining = totalTicks;

            cyclesLeft = dispCycles;

            pumpRunning = true;
            DispenseRunning = true;
            CycleElapsed = dispCycles;
            PumpRunningTime = HAL_GetTick();

            /*
             * These are short, non-blocking commands in
             * the current stepper library.
             */
            _SMPP_setDir(clockwise);
            _SMPP_setRPM10(rpm10);
        }

        return;
    }


    /* -------------------------------------------------
     * NOTHING RUNNING
     * ------------------------------------------------- */
    if (phase == D_IDLE)
        return;


    /* -------------------------------------------------
     * REVERSE IS EXECUTED BY THE NORMAL TASK
     *
     * Do not advance RUN/PAUSE timing while reverse
     * operation is being performed.
     * ------------------------------------------------- */
    if (phase == D_REVERSING)
        return;


    /* -------------------------------------------------
     * CONSUME ONE 100 ms TICK
     * ------------------------------------------------- */

    if (!infiniteCycles && totalRemaining > 0)
        totalRemaining--;

    if (phaseRemaining > 0)
        phaseRemaining--;


    /* -------------------------------------------------
     * RUN PHASE FINISHED
     * ------------------------------------------------- */
    if (phase == D_RUN && phaseRemaining == 0)
    {
        pumpRunning = false;
        PauseStartTime = HAL_GetTick();

        /*
         * Request motor deceleration at the exact
         * TIM1 timing boundary.
         */
        _SMPP_stop_int();


        /* Reverse required */
        if (reverseAngle > 0)
        {
            phase = D_REVERSING;

            dispEvt |= DISP_EVT_REVERSE;
        }


        /* Last cycle and no reverse */
        else if (!infiniteCycles && cyclesLeft == 1)
        {
            phase = D_IDLE;

            DispenseRunning = false;

            dispEvt |= DISP_EVT_FINISHED;
        }


        /* More cycles remaining */
        else
        {
            phase = D_PAUSE;

            phaseRemaining = pauseTicks;
        }
    }


    /* -------------------------------------------------
     * PAUSE PHASE FINISHED
     * ------------------------------------------------- */
    else if (phase == D_PAUSE && phaseRemaining == 0)
    {
        if (!infiniteCycles)
            cyclesLeft--;

        phase = D_RUN;

        phaseRemaining = runTicks;

        pumpRunning = true;
        PumpRunningTime = HAL_GetTick();
        if (!infiniteCycles)
            CycleElapsed--;

        /*
         * Start the next RUN directly from the timer
         * state transition.
         */
        _SMPP_setDir(clockwise);
        _SMPP_setRPM10(rpm10);
    }
}
