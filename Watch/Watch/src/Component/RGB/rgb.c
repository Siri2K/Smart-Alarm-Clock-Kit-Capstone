#include "rgb.h"

static const struct gpio_dt_spec redNode = GPIO_DT_SPEC_GET_OR(RED_NODE, gpios,{0});
static const struct gpio_dt_spec greenNode = GPIO_DT_SPEC_GET_OR(GREEN_NODE, gpios,{0});
static const struct gpio_dt_spec blueNode = GPIO_DT_SPEC_GET_OR(BLUE_NODE, gpios,{0});

uint8_t initializeRGB(){
    // Status Check
    struct gpio_dt_spec rgbNodes[3] = {redNode,greenNode,blueNode};

    // Check and Configure Each LED
    for(uint8_t i=0;i<3;i++){
        if((gpio_is_ready_dt(&rgbNodes[i]) && gpio_pin_configure_dt(&rgbNodes[i],GPIO_OUTPUT_INACTIVE)) != 0){ // Break Early
            break;
        }
    }
    return 0;
}

uint8_t turnOffRGB(){
    // Status Check
    struct gpio_dt_spec rgbNodes[3] = {redNode,greenNode,blueNode};

    // Turn off All RGB GPIO
    for(uint8_t i=0;i<3;i++){
        if(gpio_pin_set_dt(&rgbNodes[i],1) != 0){ // Break Early
            break;
        }
    }
    return 0;
}

uint8_t turnOnRGB(rgb_colors_t color){
    // Turn off all LEDs before setting the desired color
    if(turnOffRGB() != 0){
        return 1; // Error
    }

    // Choose Color
    switch (color)
    {
    case RED: // Red Light
        return (uint8_t)gpio_pin_set_dt(&redNode,0);
        break;
    case YELLOW: // Yellow Light
        return (uint8_t)gpio_pin_set_dt(&redNode,0) && (uint8_t)gpio_pin_set_dt(&greenNode,0);
        break;
    case GREEN: // Green Light
        return (uint8_t)gpio_pin_set_dt(&blueNode,0);
        break;
    }

    return 0;
} 