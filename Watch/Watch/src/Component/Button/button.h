#ifndef BUTTON_H
#define BUTTON_H

/* Zephyr library */
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* C Library */
#include <stdio.h>
#include <stdbool.h>

/* Definitions*/
#define BUTTON_NODE DT_ALIAS(button) // Red LED Node

typedef enum button_pos_t{
    BUTTON_ON = (int)0,
    BUTTON_OFF = (int)0
}button_pos_t;


/* Global variables */
extern const struct gpio_dt_spec buttonNode;

/* Tasks */
int initializeButton();

volatile int pressed();

volatile int64_t calculatePressTime();

#endif