#ifndef RGB_H
#define RGB_H

/* Zephyr library */
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* C Library */
#include <stdio.h>
#include <stdlib.h>

/* Definitions*/
#define RED_NODE DT_ALIAS(red_led) // Red LED Node
#define GREEN_NODE DT_ALIAS(green_led) // Green LED Node
#define BLUE_NODE DT_ALIAS(blue_led) // Blue LED Node

typedef enum rgb_colors_t{
    GREEN = 0,
    YELLOW = 1,
    RED = 2
}rgb_colors_t;

/* Functions */
extern uint8_t initializeRGB();

extern uint8_t turnOffRGB();

extern uint8_t turnOnRGB(rgb_colors_t color); 

#endif