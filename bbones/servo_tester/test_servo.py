#!/usr/bin/env python

import Adafruit_BBIO.PWM as PWM
from time import sleep

freq = 50.0 # hz

fucked_up_bbone_clock_factor = 2.0;
freq = freq / fucked_up_bbone_clock_factor;

pwm_ccw = 1.0 # ms
pwm_center = 1.5 # ms
pwm_cw = 2.0 # ms

duty_ccw = pwm_ccw * freq * .001
duty_center = pwm_center * freq * .001
duty_cw = pwm_cw * freq * .001

PWM.start("P8_13", duty_center*100.0, freq)

wave_period = 1

while True:
    PWM.set_duty_cycle("P8_13", duty_ccw*100.0)
    sleep(wave_period * 0.5)
    PWM.set_duty_cycle("P8_13", duty_cw*100.0)
    sleep(wave_period * 0.5)

PWM.stop("P8_13")
PWM.cleanup()
