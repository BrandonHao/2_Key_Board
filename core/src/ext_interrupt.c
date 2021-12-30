#include "ext_interrupt.h"
#include "sam.h"

void EIC_Handler() {
}

void eicInit() {
    PM->APBAMASK.bit.EIC_ = 1;
    EIC->CTRL.bit.SWRST = 1;

    while(EIC->STATUS.bit.SYNCBUSY);

    EIC->EVCTRL.reg = EIC_EVCTRL_EXTINTEO10 | EIC_EVCTRL_EXTINTEO11;
    EIC->CONFIG[1].reg = EIC_CONFIG_SENSE2_LOW | EIC_CONFIG_SENSE1_LOW;
}

void eicStart() {
    EIC->CTRL.bit.ENABLE = 1;

    while(EIC->STATUS.bit.SYNCBUSY);

    NVIC_ClearPendingIRQ(EIC_IRQn);
    NVIC_EnableIRQ(EIC_IRQn);
}

void eicStop() {
    EIC->CTRL.bit.ENABLE = 0;

    while(EIC->STATUS.bit.SYNCBUSY);

    NVIC_ClearPendingIRQ(EIC_IRQn);
    NVIC_DisableIRQ(EIC_IRQn);
}

