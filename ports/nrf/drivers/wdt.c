/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Daniel Thompson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/mphal.h"

#if MICROPY_PY_MACHINE_WDT

#include "wdt.h"
#include "nrf_wdt.h"

bool wdt_starve;

#if MICROPY_HW_HAS_WDT_BUTTON
static void button_init(void)
{
    nrf_gpio_cfg_sense_input(
            MICROPY_HW_WDT_BUTTON,
            MICROPY_HW_WDT_BUTTON_PULL < 0 ? NRF_GPIO_PIN_PULLDOWN :
            MICROPY_HW_WDT_BUTTON_PULL > 0 ? NRF_GPIO_PIN_PULLUP :
                                             NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_NOSENSE);

#if MICROPY_HW_WDT_BUTTON_ENABLE
    nrf_gpio_cfg_output(MICROPY_HW_WDT_BUTTON_ENABLE);
    nrf_gpio_pin_write(MICROPY_HW_WDT_BUTTON_ENABLE,
                       MICROPY_HW_WDT_BUTTON_ACTIVE);
#endif
}

static bool button_pressed(void)
{
    return nrf_gpio_pin_read(MICROPY_HW_WDT_BUTTON) == MICROPY_HW_WDT_BUTTON_ACTIVE;
}
#endif

void wdt_init(void)
{
    if (!nrf_wdt_started()) {
        // 1 => keep running during a sleep, stop during SWD debug
        nrf_wdt_behaviour_set(1);

        // timeout after 5 seconds
        nrf_wdt_reload_value_set(5 * 32768);

        // enable the 0th channel
        nrf_wdt_reload_request_enable(NRF_WDT_RR0);

        // set it running
        nrf_wdt_task_trigger(NRF_WDT_TASK_START);

#if MICROPY_HW_HAS_WDT_BUTTON
	// if there was no bootloader to configure the WDT then it
	// probably didn't setup the pins for the button either
	button_init();
#endif
    }
}

void wdt_feed(bool isr)
{
    /*
     * A WDT button allows us to feed the dog from somewhere that would
     * normally be "silly", such as a periodic timer interrupt. By providing
     * the user direct control over feeding the dog we are, in effect,
     * implementing a (reasonably robust) long-press reset button.
     */
#if MICROPY_HW_HAS_WDT_BUTTON
    if (button_pressed())
        return;
#else
    if (isr)
	return;
#endif

    if (wdt_starve)
	return;

    nrf_wdt_reload_request_set(0);
}

#endif // MICROPY_PY_MACHINE_WDT
