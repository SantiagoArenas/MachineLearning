/* 
 * File:   newmain.c
 * Author: santi
 *
 * Created on February 13, 2024, 12:13 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#define PIN_PULSADOR 5 // EL PULSADOR ESTÁ CONECTADO EN EL PIN RB5

int main(void) 
{
    int pulsador;
    //ANSELC = ~(1<<PIN_LED_RELOJ);
    ANSELB = ~(1<<PIN_PULSADOR);
    
    TRISC = 0xF; //RC3 A RC0 SALIDAS, EL RESTO COMO ENTRADAS
    LATC = ~0xF; //APAGA LOS LEDS DE RC0 A RC3
    
    TRISB = ~(1<<PIN_PULSADOR); //COMO INPUT SÓLO EL PIN DEL PULSADOR
    TRISA = 0;
    LATA = 0;
    
    
    while(1)
    {
        pulsador = (PORTB>>PIN_PULSADOR) & 1;
        
        if(pulsador == 0)
        {
            LATC &= ~1; 
        }
        else
        {
            LATC |= 1; 
        }
    }
    return 0;
}

