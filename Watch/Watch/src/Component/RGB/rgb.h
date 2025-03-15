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
#define RED_NODE DT_ALIAS(red) // Red LED Node
#define GREEN_NODE DT_ALIAS(green) // Green LED Node
#define BLUE_NODE DT_ALIAS(blue) // Blue LED Node

typedef enum rgb_colors_t{
    GREEN = 0,
    YELLOW = 1,
    RED = 2
}rgb_colors_t;

/* Functions */
extern int8_t initializeRGB();

extern int8_t turnOffRGB();

extern int8_t turnOnRGB(rgb_colors_t color); 

#endif