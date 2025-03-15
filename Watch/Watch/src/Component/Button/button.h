#ifndef BUTTON_H
#define BUTTON_H

/* Zephyr library */
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/sys/printk.h>

/* C Library */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Definitions*/
#define BUTTON_NODE DT_ALIAS(button) // Red LED Node
#define BUTTON_PRESSED_MAX 5000 // 10s

typedef enum button_pos_t{
    BUTTON_ON = 1,
    BUTTON_OFF = 0
}button_pos_t;


/* Tasks */
extern int8_t initializeButton();

extern int8_t pressed();

extern int64_t calculatePressTime();

#endif