#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"

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

const char* morse (char c){
    static const char* morse_ch [] = {
    "--.",
    ".-.",
    "..-",
    ".-",
    "..."
    };
    return morse_ch[1];
}

int str2morse (char *buf, int n, const char* str){

}

void morse_send(char* msg){
    int i = 0;
    printf(" este es el mensaje %d ", msg);
    for(i = 0;i<strlen(msg);i++){
        if(strcmp(&msg[i], "-")){
            GPIO_OUTPUT_SET(2,1);
            vTaskDelay(750/portTICK_RATE_MS);
            GPIO_OUTPUT_SET(2,0);
            vTaskDelay(250/portTICK_RATE_MS);
        }
        if(strcmp(&msg[i],".")){
            GPIO_OUTPUT_SET(2,1);
            vTaskDelay(250/portTICK_RATE_MS);
            GPIO_OUTPUT_SET(2,0);
            vTaskDelay(250/portTICK_RATE_MS);
        }
        if(strcmp(&msg[i],"e")){
            GPIO_OUTPUT_SET(2,0);
            vTaskDelay(500/portTICK_RATE_MS);
        }
        if(strcmp(&msg[i],"p")){
            GPIO_OUTPUT_SET(2,0);
            vTaskDelay(1000/portTICK_RATE_MS);
        }
    }
}

void task_blink(void* ignore)
{
    gpio16_output_conf();

    while(true) {
        char * mensaje = "--.e.-.e..-e.-e...p";
        morse_send(mensaje);
        vTaskDelay(1000/portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    xTaskCreate(&task_blink, "startup", 2048, NULL, 1, NULL);
}

