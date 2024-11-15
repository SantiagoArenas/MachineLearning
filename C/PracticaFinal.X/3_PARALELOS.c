/* 
 * File:   3_PARALELOS.c
 * Author: santi
 *
 * Created on February 13, 2024, 12:57 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "Temporizador_2.h"
#include "Temporizador_3.h"

#define PIN_PULSADOR 5 // EL PULSADOR ESTÁ CONECTADO EN EL PIN RB5
#define PIN_LED_RELOJ_2 3 // EL LED CONTROLADO POR EL RELOJ ES RC3
#define PIN_LED_RELOJ_1 2 // EL LED A 1 SEGUNDO ES RC2
#define PIN_LED_PULSADOR 0 // EL LED A 1 SEGUNDO ES RC2

int main(void) 
{
    int pulsador, ticks, ticks_3;
    
    ANSELC = ~((1 << PIN_LED_RELOJ_1)|(1<< PIN_LED_PULSADOR)|(1<< PIN_LED_RELOJ_2));
    ANSELB = ~(1 << PIN_PULSADOR);

    TRISC = 0; //RC3 A RC0 SALIDAS, EL RESTO COMO ENTRADAS
    LATC = 0xF; //APAGA LOS LEDS DE RC0 A RC3

    TRISB = (1 << PIN_PULSADOR); //COMO INPUT SÓLO EL PIN DEL PULSADOR
    TRISA = 0;
    LATA = 0;
    
    InicializarTimer3();
    InicializarTimer2();
    
    INTCONbits.MVEC = 1; // Modo multivector
    asm("ei"); // Interr. habilitadas
    
    while(1)
    {   
        pulsador = (PORTB >> PIN_PULSADOR) & 1;

        if (pulsador == 0) {
            LATCCLR = 1 << PIN_LED_PULSADOR;
        } else {
            LATCSET = 1 << PIN_LED_PULSADOR;
        }
    }
    return 0;
}

