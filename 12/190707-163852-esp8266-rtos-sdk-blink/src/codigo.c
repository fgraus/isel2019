#include <string.h>
#include <stdio.h>
#include "esp_common.h"
#include "gpio.h"
#include "freertos/task.h"
#include "fsm.h"

static int codigo_secreto[] = { 7, 8, 9};
static int codigo[10];
static int actualPulsacion = 0;
static portTickType tiempo = 0;

static int *codigoValido = NULL;
static volatile int* boton;
static int empezo = 0;

static int codigoCorrecto (fsm_t* fsm){
    return actualPulsacion == (sizeof(codigo_secreto) / sizeof(codigo_secreto[0]));
}

static int timeout(fsm_t* fsm){
    return !codigoCorrecto(fsm) && empezo && (xTaskGetTickCount() > tiempo);
}

static int timeoutCorrecto(fsm_t* fsm){
    return (timeout(fsm) && (codigo[actualPulsacion] == codigo_secreto[actualPulsacion]));
}

static int timeoutNoCorrecto(fsm_t* fsm){
    return (timeout(fsm) && (codigo[actualPulsacion] != codigo_secreto[actualPulsacion]));
}

static int pulsacion(fsm_t* fsm){
    return !codigoCorrecto && !timeout(fsm) && *boton;
}

static void codigoAceptado(fsm_t* fsm){
    *codigoValido = 1;
    reset(fsm);
    puts("CODIGO CORRECTO");
}

static void siguienteDigito (fsm_t* fsm){
    codigo[++actualPulsacion] = 0;
    empezo = 0;
}

static void reset(fsm_t* fsm){
    for(int i = 0; i<actualPulsacion;i++){
        codigo[i] = 0;
    }
    actualPulsacion = 0;
    empezo = 0;
}

static void incrementarCodigoActual(fsm_t* fsm){
    codigo[actualPulsacion]++;
    tiempo = xTaskGetTickCount() + (1000/ portTICK_RATE_MS);
    empezo = 1;
    *boton = 0;
}

fsm_t* fsm_codigo (int* validop, volatile int* botonp){
    static fsm_trans_t codigo_tt[] = {
        {0, codigoCorrecto, 0, codigoAceptado},
        {0, timeoutCorrecto, 0, siguienteDigito},
        {0, timeoutNoCorrecto, 0, reset},
        {0, pulsacion, 0, incrementarCodigoActual},
        {-1, NULL, -1, NULL}
    };
    codigoValido = validop;
    boton = botonp;
    return fsm_new(codigo_tt);
}

