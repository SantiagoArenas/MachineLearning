/* 
 * File:   Temporizador_3.c
 * Author: santi
 *
 * Created on February 13, 2024, 1:06 PM
 */

#include "Temporizador_3.h"
#include "Pic32Ini.h"
#include <xc.h>
#define PIN_LED_RELOJ_2 3 // EL LED CONTROLADO POR EL RELOJ ES RC3
#define PIN_LED_RELOJ_1 2 // EL LED A 1 SEGUNDO ES RC2
static uint32_t ticks_3 = 0; // Variable global al driver
void InicializarTimer3(void) {
    InicializarReloj();
    T3CON = 0x0000;
    TMR3 = 0;
    PR3 = 39062; // Se configura el timer para que termine la cuenta en 0,5s, 
    //0,5s*5MHz*(1/64)=39062,5; me quito el 0,5.
    IPC3bits.T3IP = 4; // Se configura la prioridad de la interrupción a 4
    IPC3bits.T3IS = 0; // Subprioridad 0
    IFS0bits.T3IF = 0; // Se borra el flag de interrupción por si estaba pendiente
    IEC0bits.T3IE = 1; // y por último se habilita su interrupción
    T3CON = 0x8060; // Se arranca el timer con prescalado 64, TCKPS = 6
}
__attribute__((vector(_TIMER_3_VECTOR), interrupt(IPL4SOFT), nomips16)) void InterrupcionTimer3(void) {
    // Se borra el flag de interrupción
    IFS0bits.T3IF = 0;
    LATCINV = 1 << PIN_LED_RELOJ_1;
    }



