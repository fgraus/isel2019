#include <string.h>
#include <stdio.h>
#include "esp_common.h"
#include "gpio.h"
#include "freertos/task.h"
#include "fsm.h"
#define TIMEOUT 60000 // un minuto

static volatile int* boton;
static int luz;
static portTickType timeoutPort;


static int isButtonPressed(fsm_t* fsm){
    return *boton;
}

static int timeoutHappened(fsm_t* fsm){
    return xTaskGetTickCount() >= timeoutPort;
}

static void encender(){
    GPIO_OUPUT_SET(luz, 1);
    *boton = 0;
    timeoutPort = xTaskGetTickCount() + TIMEOUT;
}

static void apagar(){
    GPIO_OUPUT_SET(luz, 0);
    *boton = 0;
}

fsm_t* fsm_luz (volatile int* buttom, int light){
    static fsm_trans_t luz_tt[] = {
        {0, isButtonPressed, 1, encender},
        {1, timeoutHappened, 0, apagar},
        {-1, NULL, -1, NULL}
    };
    boton = buttom;
    luz = light;
    return fsm_new(luz_tt);
}

static void apagar(){

}