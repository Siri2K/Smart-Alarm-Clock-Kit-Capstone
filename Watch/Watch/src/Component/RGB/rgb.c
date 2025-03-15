#include "rgb.h"

static const struct gpio_dt_spec redNode = GPIO_DT_SPEC_GET(RED_NODE, gpios);
static const struct gpio_dt_spec greenNode = GPIO_DT_SPEC_GET(GREEN_NODE, gpios);
static const struct gpio_dt_spec blueNode = GPIO_DT_SPEC_GET(BLUE_NODE, gpios);

int8_t initializeRGB(){
    // Status Check
    int8_t status = 0;
    struct gpio_dt_spec rgbNodes[3] = {redNode,greenNode,blueNode};

    // Check and Configure Each LED
    for(uint8_t i=0;i<sizeof(rgbNodes)/sizeof(rgbNodes[0]);i++){
        status = (int8_t)gpio_is_ready_dt(&rgbNodes[i]);
        if(status != 1){
            printk("LED Pin %d is not ready and returns with status %d\n", rgbNodes[i].pin, status);
            return status;
        }

        status = (int8_t)gpio_pin_configure_dt(&rgbNodes[i],GPIO_OUTPUT_ACTIVE);
        if(status != 0){
            printk("LED Pin %d is not configured and returns with status %d\n", rgbNodes[i].pin,status);
            return status;
        }

    }

    status = turnOffRGB();
    return 0;
}

int8_t turnOffRGB(){
    // Status Check
    int8_t status = 0;
    struct gpio_dt_spec rgbNodes[3] = {redNode,greenNode,blueNode};

    // Turn off All RGB GPIO
    for(uint8_t i=0;i<sizeof(rgbNodes)/sizeof(rgbNodes[0]);i++){
        status = (int8_t)gpio_pin_set_dt(&rgbNodes[i],0);
        if(status != 0){ // Break Early
            printk("LED Pin %d did not turn off and returns with status %d\n", rgbNodes[i].pin, status);
            return status;
        }
    }
    return 0;
}

int8_t turnOnRGB(rgb_colors_t color){
    // Status Check
    int8_t status = 0;
    struct gpio_dt_spec rgbNodes[3] = {redNode,greenNode,blueNode};

    // Check if all LED's are ready
    status = turnOffRGB();
    
    switch (color){
    case RED:
        status = (int8_t)gpio_pin_set_dt(&rgbNodes[0],1);
        if(status != 0){ // Break Early
            printk("LED Pin %d did not turn off and returns with status %d\n", rgbNodes[0].pin, status);
            return status;
        }
        break;
    case YELLOW:
        status = (int8_t)gpio_pin_set_dt(&rgbNodes[0],1);
        if(status != 0){ // Break Early
            printk("LED Pin %d did not turn off and returns with status %d\n", rgbNodes[0].pin, status);
            return status;
        }

        status = (int8_t)gpio_pin_set_dt(&rgbNodes[1],1);
        if(status != 0){ // Break Early
            printk("LED Pin %d did not turn off and returns with status %d\n", rgbNodes[1].pin, status);
            return status;
        }
        break;
    case GREEN:
        status = (int8_t)gpio_pin_set_dt(&rgbNodes[1],1);
        if(status != 0){ // Break Early
            printk("LED Pin %d did not turn off and returns with status %d\n", rgbNodes[1].pin, status);
            return status;
        }
        break;
    default:
        break;
    }

    return 0;
}

void printRGBColor(rgb_colors_t color){
    // Choose Color
    switch (color)
    {
    case RED: // Red Light
        break;
    case YELLOW: // Yellow Light
        break;
    case GREEN: // Green Light
        break;
    default:
        break;
    }
}