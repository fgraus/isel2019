#include <string.h>
#include <stdio.h>
#include "esp_common.h"
#include "gpio.h"
#include "freertos/task.h"
#include "fsm.h"
#include "freertos/semphr.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

#define PERIODOlUZ 300/portTICK_RATE_MS
#define PERIODOALARMA 100/portTICK_RATE_MS
#define PERIODOCODIGO 300/portTICK_RATE_MS

#define PRESENCIA   1
#define BOTONCODIGO 2
#define BOTONLUZ    3
#define ALARMA      4
#define LUZ         5
#define CODIGO      6

int codigoValido = 0;
xSemaphoreHandle xMutexCodeOK;

fsm_t* fsm_luz (volatile int* buttom, int light);
fsm_t* fsm_alarma (int* valido, int presenciap, int alarmap);
fsm_t* fsm_codigo (int* validop, volatile int* botonp);

static volatile int gpio_activated[16];

static void
io_intr_handler (void)
{
    static portTickType debounce_timeout[16];
    portTickType now = xTaskGetTickCount();
    uint32 status = GPIO_REG_READ (GPIO_STATUS_ADDRESS);

    int i;
    for (i = 0; i < 16; ++i) {
        if ((status & (1 << i)) && (now >= debounce_timeout[i])) {
            debounce_timeout[i] = now + (200 /portTICK_RATE_MS);
            gpio_activated[i] = 1;
        }
    }
    /* rearm interrupts */
    GPIO_REG_WRITE (GPIO_STATUS_W1TC_ADDRESS, status);
}

static void
gpio_setup (GPIOMode_TypeDef mode, GPIO_INT_TYPE intrtype,
            GPIO_Pullup_IF pullup, uint16 mask)
{
    GPIO_ConfigTypeDef io_in_conf;
    io_in_conf.GPIO_Mode = mode;
    io_in_conf.GPIO_IntrType = intrtype;
    io_in_conf.GPIO_Pullup = pullup;
    io_in_conf.GPIO_Pin = mask;
    gpio_config (&io_in_conf);
}

static void tareaLuz(void* parametros){
    fsm_t* luzfsm = fsm_luz(&gpio_activated[BOTONLUZ], ALARMA);
    while(1){
        fsm_fire(luzfsm);
        vTaskDelay(PERIODOlUZ);
    }
}

static void tareaAlarma(void* parametros){
    fsm_t* alarmafsm = fsm_alarma(&codigoValido, &gpio_activated[BOTONCODIGO], ALARMA);
    while(1){
        fsm_fire(alarmafsm);
        vTaskDelay(PERIODOALARMA);
    }
}

static void tareaCodigo(void* parametros){
    fsm_t* codigofsm = fsm_codigo(&codigoValido, &gpio_activated[BOTONCODIGO]);

    if(xMutexCodeOK != NULL){
        if(xSemaphoreTake(xMutexCodeOK, portMAX_DELAY)){
            codigoValido = 0;
            xSemaphoreGive(xMutexCodeOK);
        }
    }

    while(1){
        fsm_fire(codigofsm);
        vTaskDelay(PERIODOCODIGO);
    }
}

static void controlDomotico(void* nada){
    gpio_setup (GPIO_Mode_Input, GPIO_PIN_INTR_DISABLE, GPIO_PullUp_EN,
                (1 << PRESENCIA));
    gpio_setup (GPIO_Mode_Input, GPIO_PIN_INTR_NEGEDGE, GPIO_PullUp_EN,
                (1 << BOTONCODIGO) | (1 << BOTONLUZ));
    gpio_setup (GPIO_Mode_Output, GPIO_PIN_INTR_DISABLE, GPIO_PullUp_DIS,
                (1 << ALARMA) | (1 << LUZ));
    gpio_intr_handler_register ((void *) io_intr_handler, NULL);
    ETS_GPIO_INTR_ENABLE();
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    xTaskHandle tarea;
    xTaskCreate (controlDomotico, "domoticaTarea", 2048, NULL, 1, &tarea);
}

