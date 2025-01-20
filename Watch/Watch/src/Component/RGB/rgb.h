#ifndef RGB_H
#define RGB_H

/* Zephyr library */
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* C Library */
#include <stdio.h>

/* Definitions*/
#define RED_NODE DT_ALIAS(red_led) // Red LED Node
#define GREEN_NODE DT_ALIAS(green_led) // Green LED Node
#define BLUE_NODE DT_ALIAS(blue_led) // Blue LED Node

/* Global variables */
extern const struct gpio_dt_spec redNode;
extern const struct gpio_dt_spec greenNode;
extern const struct gpio_dt_spec blueNode;

typedef enum rgb_colors_t{
    GREEN,
    YELLOW,
    RED
}rgb_colors_t;

/* Functions */
int initializeRGB();

int turnOffRGB();

int turnOnRGB(rgb_colors_t color); 

#endif