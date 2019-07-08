#include <string.h>
#include <stdio.h>
#include "esp_common.h"
#include "gpio.h"
#include "freertos/task.h"
#include "fsm.h"

static int *codigo_valido = NULL;
static int presencia = 1;                    
static int alarma = 2;

static int codigoCorrecto(fsm_t* fsm){
    return *codigo_valido;
}

static int pressenc(fsm_t* fsm){
    return !GPIO_INPUT_GET(presencia);
}

static void armarAlarma(fsm_t* fsm){
    GPIO_OUTPUT_SET (alarma, 0);
    *codigo_valido = 0;
    puts("ALARMA ARMADA");
}

static void intruso(fsm_t* fsm){
    GPIO_OUTPUT_SET (alarma, 1);
    puts ("INTRUSO PILLADO");
}

static void desarmarAlarma(fsm_t* fsm){
    GPIO_OUTPUT_SET(alarma, 0);
    *codigo_valido = 0;
    puts("ALARMA DESARMADA");
}

fsm_t* fsm_alarma (int* valido, int presenciap, int alarmap){
    static fsm_trans_t alarma_tt[] = {
        {0, codigoCorrecto, 1, armarAlarma},
        {1, codigoCorrecto, 0, desarmarAlarma},
        {1, pressenc, 1, intruso},
        {-1, NULL, -1, NULL},
    };
    codigo_valido = valido;
    presencia = presenciap;
    alarma = alarmap;
    return fsm_new(alarma_tt);

}

