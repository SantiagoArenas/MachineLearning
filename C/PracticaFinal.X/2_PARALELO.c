/* 
 * File:   2_PARALELO.c
 * Author: santi
 *
 * Created on February 13, 2024, 12:22 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "Temporizador_2.h"

#define PIN_PULSADOR 5 
#define PIN_LED_RELOJ 3 
#define PIN_LED_PULSADOR 0 


int main(void) {
    int pulsador, ticks_act, ticks_ant, ticks, par;

    ANSELC = ~((1 << PIN_LED_RELOJ)|(1<< PIN_LED_PULSADOR));
    ANSELB = ~(1 << PIN_PULSADOR);

    TRISC = 0; //RC3 A RC0 SALIDAS, EL RESTO COMO ENTRADAS
    LATC = 0xF; //APAGA LOS LEDS DE RC0 A RC3

    TRISB = (1 << PIN_PULSADOR); //INPUT PULSADOR
    TRISA = 0;
    LATA = 0;

    InicializarTimer2();

    
    INTCONbits.MVEC = 1; // Modo multivector
    asm("ei"); // Interr. habilitadas


    while (1) {
        pulsador = (PORTB >> PIN_PULSADOR) & 1;

        if (pulsador == 0) {
            LATCCLR = 1 << PIN_LED_PULSADOR;
        } else {
            LATCSET = 1 << PIN_LED_PULSADOR;
        }
    }
    return 0;
}

