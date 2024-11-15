#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include "Pic32Ini.h"
#include "UART.h"

#define POS_PREESCALADO 4
#define ON 15

uint8_t pieza_levantada = 0; // flag de si hay una pieza levantada + informacion de que pieza era
void InitTimer2(uint16_t retardo_ms) {
    int max_num = 65535;
    int multiplicador = retardo_ms * 5000;
    int vector[] = {1, 2, 4, 8, 16, 32, 64, 256};

    for (int i = 0; i < sizeof(vector) / sizeof(vector[0]); i++) {
        int PRpos = multiplicador / vector[i] - 1;
        if (PRpos <= max_num) {
            T2CON = 0; // Timer 2 off
            TMR2 = 0;  // Cuenta a 0
            IFS0bits.T4IF = 0; // Se borra el flag
            PR2 = PRpos; // PR2 se establece con el valor calculado
            IPC2bits.T2IP = 2;
            IPC2bits.T2IS = 0;
            IFS0bits.T2IF = 0;
            IEC0bits.T2IE = 1;
            T2CON = 0x8000 | (i << POS_PREESCALADO); // Timer 2 on, preescalado definido por 'i', timer en modo activo
            
            /*INTCONbits.MVEC = 1;
            asm("ei");*/
        }
    }
}

__attribute__((vector(8), interrupt(IPL2SOFT), nomips16)) void InterrupcionT2(void) {
    IFS0bits.T2IF = 0;
    char *respuesta = "Establece la posición inicial como aparece por pantalla\n";
    putsUART(respuesta);
}

void paraTimer2(void){
    T2CON &= ~(1<<ON);
}

