/* 
 * File:   Temporizador_2.c
 * Author: santi
 *
 * Created on February 13, 2024, 12:27 PM
 */

#include "Temporizador_2.h"
#include "Pic32Ini.h"
#include <xc.h>

#define PIN_LED_RELOJ 3
#define PIN_LED_GANAR 1

int var, juego_on = 0, contador = 0; // Variable global al driver, var y juego_on es para el 4
void InicializarTimer2(void) {
    InicializarReloj();
    T2CON = 0x0000;
    TMR2 = 0;
    PR2 = 19531; // Se configura el timer para que termine la cuenta en 1s, 
    //1s*5MHz*(1/256)=15931,25; me quito el 0,25.
    IPC2bits.T2IP = 2; // Se configura la prioridad de la interrupción a 2 
    IPC2bits.T2IS = 0; // Subprioridad 0
    IFS0bits.T2IF = 0; // Se borra el flag de interrupción por si estaba pendiente
    IEC0bits.T2IE = 1; // y por último se habilita su interrupción
    T2CON = 0x8070; // Se arranca el timer con prescalado 256, TCKPS = 7
}
__attribute__((vector(_TIMER_2_VECTOR), interrupt(IPL2SOFT), nomips16)) void InterrupcionTimer2(void) {
    // Se borra el flag de interrupción
    IFS0bits.T2IF = 0;
    LATCINV = 1 << PIN_LED_RELOJ;
    var = 0;
    if(juego_on == 1){
        contador++;
        if(contador>=5){
            juego_on = 0;
            contador = 0;
            LATCSET = 1 << PIN_LED_GANAR;
        }
    }
}
void enciendeLed (void){
    juego_on = 1;
    LATCCLR = 1 << PIN_LED_GANAR;
}

void incremVar(void){
    var++;
}

int getVar(void){
    return var;
}



