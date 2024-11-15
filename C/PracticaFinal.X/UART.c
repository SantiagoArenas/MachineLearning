#include <xc.h>
#include "Pic32Ini.h"

#define RX 13 //RB13
#define TX 7 //RB7
#define TAM_COLA 100


typedef struct {
    int icabeza;
    int icola;
    char datos[TAM_COLA];
} cola_t;

static cola_t cola_tx, cola_rx;
static int divisor = 16;

void InicializarUART(int baudios) {
    cola_rx.icabeza = 0;
    cola_rx.icola = 0;
    cola_tx.icabeza = 0;
    cola_tx.icola = 0;
    ANSELB &= ~((1 << TX) | (1 << RX));
    TRISB |= 1 << RX;
    LATB |= 1 << TX;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    U1RXR = 3;
    RPB7R = 1;
    SYSKEY = 0x1CA11CA1;
    if(baudios > 38400){
        divisor = 4;
    }
    U1BRG = (5000000/(divisor*baudios)) - 1; 
    
    IPC8bits.U1IP = 3; 
    IPC8bits.U1IS = 1;
    IFS1bits.U1RXIF = 0;
    IFS1bits.U1TXIF = 0;
    IEC1bits.U1RXIE = 1;
    U1STAbits.URXISEL = 0; 
    U1STAbits.UTXISEL = 2;
    U1STAbits.URXEN = 1; 
    U1STAbits.UTXEN = 1;
    U1MODE = 0x8000;
}


__attribute__((vector(32), interrupt(IPL3SOFT), nomips16)) void InterrupcionUART1(void) {
    char c;
    if (IFS1bits.U1RXIF == 1) { // Ha interrumpido el receptor
        if ((cola_rx.icabeza + 1 == cola_rx.icola) ||
                (cola_rx.icabeza + 1 == TAM_COLA && cola_rx.icola == 0)) {
            // La cola est? llena
        } else {
            c = U1RXREG;
            cola_rx.datos[cola_rx.icabeza] = c; // Lee caracter de la UART
            cola_rx.icabeza++;
            if (cola_rx.icabeza == TAM_COLA) {
                cola_rx.icabeza = 0;
            }
        }
        IFS1bits.U1RXIF = 0; // Y para terminar se borra el flag
    }
    if (IFS1bits.U1TXIF == 1) { // Ha interrumpido el transmisor
        if (cola_tx.icola != cola_tx.icabeza) { // Hay datos nuevos
            U1TXREG = cola_tx.datos[cola_tx.icola];
            cola_tx.icola++;
            if (cola_tx.icola == TAM_COLA) {
                cola_tx.icola = 0;
            }
        } else { // Se ha vaciado la cola.
            IEC1bits.U1TXIE = 0; // Para evitar bucle sin fin
        }
        IFS1bits.U1TXIF = 0; // Y para terminar se borra el flag
    }
}


void putsUART(char s[]) {
    int i = 0;
    while (s[i] != '\0') {
        if (cola_tx.icabeza + 1 == cola_tx.icola || (cola_tx.icabeza + 1 == TAM_COLA && cola_tx.icola == 0)) {
            break;
        } else {
            cola_tx.datos[cola_tx.icabeza] = s[i];
            cola_tx.icabeza++;
            i++;
            if (cola_tx.icabeza == TAM_COLA) {
                cola_tx.icabeza = 0;
            }
        }
    }
    IEC1bits.U1TXIE = 1;
}

char getcUART(void) {
    char c;
    if (cola_rx.icabeza != cola_rx.icola) {
        c = cola_rx.datos[cola_rx.icola];
        cola_rx.icola++;
        if (cola_rx.icola == TAM_COLA) {
            cola_rx.icola = 0;
        }
    } else {
        c = '\0';
    }
    return c;
}
