/*
 * Copyright (c) 2015, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sam.h"
#include "tusb.h"
#include "timer.h"

#include "hal_gpio.h"
#include "hal_init.h"
#include "hri_nvmctrl_d21.h"

#include "hpl_gclk_base.h"
#include "hpl_pm_config.h"
#include "hpl_pm_base.h"

#define KEY_PIN_MASK ((1 << 10) | (1 << 11))
#define UNUSED(x) ((void)x)

/* Referenced GCLKs, should be initialized firstly */
#define _GCLK_INIT_1ST (0 << 0 | 1 << 1)

/* Not referenced GCLKs, initialized last */
#define _GCLK_INIT_LAST (~_GCLK_INIT_1ST)

//-----------------------------------------------------------------------------

volatile uint32_t system_ticks = 0;
uint8_t lastPinState = 0;
void USB_Handler(void) {
    tud_int_handler(0);
}
void SysTick_Handler(void) {
    system_ticks++;
}


void board_init(void) {
    // Clock init ( follow hpl_init.c )
    hri_nvmctrl_set_CTRLB_RWS_bf(NVMCTRL, 2);
    _pm_init();
    _sysctrl_init_sources();
#if _GCLK_INIT_1ST
    _gclk_init_generators_by_fref(_GCLK_INIT_1ST);
#endif
    _sysctrl_init_referenced_generators();
    _gclk_init_generators_by_fref(_GCLK_INIT_LAST);
#if CFG_TUSB_OS  == OPT_OS_NONE
    SysTick_Config(CONF_CPU_FREQUENCY / 1000);
#endif
    /* USB Clock init
     * The USB module requires a GCLK_USB of 48 MHz ~ 0.25% clock
     * for low speed and full speed operation. */
    _pm_enable_bus_clock(PM_BUS_APBB, USB);
    _pm_enable_bus_clock(PM_BUS_AHB, USB);
    _gclk_enable_channel(USB_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);
    // USB Pin Init
    gpio_set_pin_direction(PIN_PA24, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(PIN_PA24, false);
    gpio_set_pin_pull_mode(PIN_PA24, GPIO_PULL_OFF);
    gpio_set_pin_direction(PIN_PA25, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(PIN_PA25, false);
    gpio_set_pin_pull_mode(PIN_PA25, GPIO_PULL_OFF);
    gpio_set_pin_function(PIN_PA24, PINMUX_PA24G_USB_DM);
    gpio_set_pin_function(PIN_PA25, PINMUX_PA25G_USB_DP);
    PORT->Group[0].CTRL.reg |= PORT_CTRL_SAMPLING(10) | PORT_CTRL_SAMPLING(11);
    gpio_set_pin_direction(PIN_PA10, GPIO_DIRECTION_IN);
    gpio_set_pin_pull_mode(PIN_PA10, GPIO_PULL_OFF);
    gpio_set_pin_direction(PIN_PA11, GPIO_DIRECTION_IN);
    gpio_set_pin_pull_mode(PIN_PA11, GPIO_PULL_OFF);
}

void hid_task(void) {
    uint8_t pinState = (PORT->Group[0].IN.reg & KEY_PIN_MASK) != KEY_PIN_MASK;

    if(lastPinState == pinState || !tud_hid_n_ready(0)) {
        return;
    }

    lastPinState = pinState;

    // Remote wakeup
    if(tud_suspended()) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    /*------------- Mouse -------------*/
    uint8_t buttonVal = (pinState ? MOUSE_BUTTON_LEFT : 0);
    tud_hid_mouse_report(0, buttonVal, 0, 0, 0, 0);
}

//-----------------------------------------------------------------------------
int main(void) {
    board_init();
    timerInit();
    timerStart();
    tusb_init();

    while(1) {
        tud_task(); // device task
        hid_task();

        while(!pollingFlag);

        pollingFlag = 0;
    }

    return 0;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    UNUSED(itf);
    UNUSED(report_id);
    UNUSED(report_type);
    UNUSED(buffer);
    UNUSED(reqlen);
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    UNUSED(itf);
    UNUSED(report_id);
    UNUSED(report_type);
    UNUSED(buffer);
    UNUSED(bufsize);
}
