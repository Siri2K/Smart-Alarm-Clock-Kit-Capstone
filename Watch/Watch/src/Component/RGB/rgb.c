#include "rgb.h"

const struct gpio_dt_spec redNode = GPIO_DT_SPEC_GET_OR(RED_NODE, gpios,{0});
const struct gpio_dt_spec greenNode = GPIO_DT_SPEC_GET_OR(GREEN_NODE, gpios,{0});
const struct gpio_dt_spec blueNode = GPIO_DT_SPEC_GET_OR(BLUE_NODE, gpios,{0});


int initializeRGB(){
    // Status Check
    int status = 1;
    
    // Check if all LED's are ready
    status &= gpio_is_ready_dt(&redNode);
    status &= gpio_is_ready_dt(&greenNode);
    status &= gpio_is_ready_dt(&blueNode);

    // Configure each pin
    status &= gpio_pin_configure_dt(&redNode,GPIO_OUTPUT_ACTIVE);
    status &= gpio_pin_configure_dt(&greenNode,GPIO_OUTPUT_ACTIVE);
    status &= gpio_pin_configure_dt(&blueNode,GPIO_OUTPUT_ACTIVE);

    return status;
}

int turnOffRGB(){
    // Status Check
    int status = 1;

    // Turn off All RGB GPIO
   status &= gpio_pin_configure_dt(&redNode,1);
   status &= gpio_pin_configure_dt(&greenNode,1);
   status &= gpio_pin_configure_dt(&blueNode,1);

    return status;
}

int turnOnRGB(rgb_colors_t color){
    // Status Check
    int status = 1;

    // Choose Color
    switch (color)
    {
    case RED: // Red Light
        status &= gpio_pin_set_dt(&redNode,0);
        break;
    case YELLOW: // Yellow Light
        status &= gpio_pin_set_dt(&redNode,0);
        status &= gpio_pin_set_dt(&greenNode,0);
        break;
    case GREEN: // Green Light
        status &= gpio_pin_set_dt(&blueNode,0);
        break;
    default:
        status &= turnOffRGB();
        break;
    }

    
    return status;
} 