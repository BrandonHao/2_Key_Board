#include "timer.h"
#include "sam.h"

#define SYS_CLOCK   48000000UL
#define TICK_PERIOD (SYS_CLOCK / 1000000)

static volatile uint64_t microTicks = 0;
static volatile uint64_t milliTicks = 0;

volatile uint8_t pollingFlag = 0;

void TCC0_Handler() {
    microTicks++;

    if(microTicks % 1000 == 0) {
        milliTicks++;
        pollingFlag = 1;
    }

    TCC0->INTFLAG.reg = TCC_INTFLAG_CNT;
}

uint64_t millis() {
    return milliTicks;
}

void timerInit() {
    PM->APBCMASK.reg |= PM_APBCMASK_TCC0;
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TCC0_TCC1 | GCLK_CLKCTRL_GEN_GCLK0 |
                        GCLK_CLKCTRL_CLKEN;
    TCC0->CTRLA.bit.SWRST = 1;

    while(TCC0->SYNCBUSY.bit.SWRST);

    TCC0->PER.bit.PER = TICK_PERIOD;
    TCC0->DBGCTRL.bit.DBGRUN = 1;
}

void timerStart() {
    NVIC_EnableIRQ(TCC0_IRQn);
    TCC0->INTENSET.bit.CNT = 1;
    TCC0->CTRLA.bit.ENABLE = 1;

    while(TCC0->SYNCBUSY.bit.ENABLE);

    pollingFlag = 0;
}

void timerStop() {
    NVIC_DisableIRQ(TCC0_IRQn);
    TCC0->INTENSET.bit.CNT = 0;
    TCC0->CTRLA.bit.ENABLE = 0;

    while(TCC0->SYNCBUSY.bit.ENABLE);

    TCC0->COUNT.reg = 0;
    microTicks = 0;
    milliTicks = 0;
    pollingFlag = 0;
}
