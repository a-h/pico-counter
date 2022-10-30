#include <stdio.h>
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Define the pins.
const uint SERVO_PIN = 16;
const uint BUTTON_START_PIN = 15;
const uint BUTTON_STOP_PIN = 14;

int started = 0;

void gpio_callback(uint gpio, uint32_t events) {
    printf("Callback received...\n");
    if(gpio == BUTTON_START_PIN) {
        started = 1;
    }
    if(gpio == BUTTON_STOP_PIN) {
        started = 0;
    }
}

// https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2
uint32_t pwm_set_freq_duty(uint slice_num,
       uint chan,uint32_t f, int d)
{
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / f / 4096 + 
                           (clock % (f * 4096) != 0);
    if (divider16 / 16 == 0)
    divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / f - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16/16,
                                     divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * d / 100);
    return wrap;
}

int main() {
    stdio_init_all();
    sleep_ms(1000);
    printf("Setting up button\n");

    // Setup PWM on the motor.
    // https://raspberrypi.github.io/pico-sdk-doxygen/group__hardware__pwm.html
    // https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    uint chan = pwm_gpio_to_channel(SERVO_PIN);
    pwm_set_freq_duty(slice_num,chan, 50, 10);

    // Setup button.
    gpio_set_irq_enabled_with_callback(BUTTON_START_PIN,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_STOP_PIN,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    while (true) {
        if(started == 1) {
            printf("Moving motor...\n");
            // Set the PWM running
            pwm_set_enabled(slice_num, true);
            while(started == 1) {
                sleep_ms(10);
            }
            pwm_set_enabled(slice_num, false);
            printf("Stopped...\n");
        }
        sleep_ms(1000);
        printf("Looping...\n");
    }
}
