#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xc.h>
#include "Pic32Ini.h"
#include "InitTimer2.h"
#include "UART.h"

#define MUX_IN 7 // RA7
#define MUX_OUT 10 // RA10
#define LED_ROJO 0 // CAMBIAR?
// sel estarÔøΩ en RC4 a RC7, RA9 y RB14
#define SEL 6 //RC6 a RC7
#define SEL5 9 //RA9 
#define SEL6 14 //RB14 
#define RX 13 //RB13
#define TX 7 //RB7

#define FILAS 4
#define COLUMNAS 4
#define PEON   'P'
#define ALFIL 'A'
#define CABALLO 'C'
#define TORRE   'T'
#define DAMA  'D'
#define REY   'R'
#define VACIO 7

#define A 'a'
#define B 'b'
#define C 'c'
#define D 'd'

typedef struct pieza {
    char nombre; //que pieza es
    char color; //blanca = 1, negra = 0
    char columna;
    char fila;
    // char movida;     //si ha sido movida = 0
} pieza;

uint8_t piezaLevantada; //flag que indica que la pieza de esa iteraciÔøΩn ha sido levantada
uint8_t piezaColocada; //flag que indica que la pieza de esa iteraciÔøΩn ha sido colocada
uint8_t inicializado = 0; // flag que dice si estÔøΩn las fichas colocadas en la posiciÔøΩn de inicio

void Lectura(uint8_t sel, uint8_t estado_mov);
void RepresentarTab(void);
void levantarPieza(uint8_t columna, uint8_t fila);
void colocarPieza(uint8_t columna, uint8_t fila);
void InicializacionTablero();
void actualizaSel(uint8_t sel);
char getColumna(pieza p) ;

static uint8_t tablero[COLUMNAS][FILAS]; // 4 columnas con 4 filas binario
static pieza tableroPartida[COLUMNAS][FILAS]; // tablero con la estructura de pieza
static uint8_t sel = 0; // 4 bits de seleccion, los 2 menos significativos para columnas, los 2 m√°s significativos para filas
static pieza piezaEnMano; // pieza que se ha levantado
static uint8_t pulsador_ant[COLUMNAS][FILAS], pulsador_act[COLUMNAS][FILAS]; // pulsadores en cada casillas (binario: 0->no hay pieza, 1->hay pieza	)

int main(void) {
    ANSELA = ~((1 << MUX_IN)|(0x3 << 0));// penÔøΩltimo bit de sel
    ANSELB = ~(0x3 << 2);// ÔøΩltimo bit de sel
    ANSELC = ~((1 << LED_ROJO)|(0x3 << SEL));

    LATA = 0; // MUX_OUT siempre a 1, ver esquema electrÔøΩnico
    LATB = 0;
    LATC = ~(1 << LED_ROJO); // Inicialmente encendido

    TRISA = (1 << MUX_IN)|(0x3 << 0);
    TRISB = (0x3 << 2);
    TRISC = 0;

    enum {
        Inicializacion, Espera, Movimiento, Captura, Transmision
    } estado;
    estado = Inicializacion;
    char respuesta[100];
    uint8_t estado_mov = 0; // a 0 tiene comportamiento normal, a 1 estÔøΩ en estado Movimiento
    
    //Representar tablero
    RepresentarTab();
    InicializarUART(9600);
    
    actualizaSel(sel);
    
    InitTimer2(3000); // Interrup cada  3 segundos
    INTCONbits.MVEC = 1; // Modo multivector
    asm("ei"); // Interr. habilitadas
    
    while (1) {
        
        // Lectura del tablero
        Lectura(sel, estado_mov);

        // TransiciÔøΩn de estados ( funciÔøΩn delta )
        switch (estado) {
            case Inicializacion:
                if (inicializado == 1) { // Se han colocado las piezas en la posiciÔøΩn inicial
                    estado = Espera;
                    paraTimer2(); // Paramos la interrupciÛn de inicializacÌon
                }
                break;
            case Espera:
                if (piezaLevantada == 1) { // El usuario ha levantado una pieza
                    piezaLevantada = 0; // Borro el flag
                    putsUART("Pieza Levantada");
                    estado = Movimiento;
                }
                break;
            case Movimiento:
                if (piezaColocada == 1) { // El usuario ha recolocado la pieza
                    piezaColocada = 0; // Borro el flag
                    sprintf(respuesta, "Movimiento: %c%c%c\n", piezaEnMano.nombre, getColumna(piezaEnMano), piezaEnMano.fila); // Ejemplo Ce4
                    putsUART(respuesta);
                    estado = Transmision;
                } else if (piezaLevantada == 1) { // El usuario ha levantada otra pieza
                    piezaLevantada = 0; // Borro el flag
                    putsUART("Otra pieza levantada");
                    estado = Captura;
                }
                break;
            case Captura:
                if (piezaColocada == 1) {
                    sprintf(respuesta, "Movimiento: %c x %c%c\n", piezaEnMano.nombre,  getColumna(piezaEnMano), piezaEnMano.fila); // Ejemplo: Kxg4
                    putsUART(respuesta);
                    estado = Transmision;
                }
            case Transmision:
                estado = Espera;
                break;
        }
        
        
        // FunciÔøΩn Lambda
        if (estado == Inicializacion) {
            InicializacionTablero(tablero);
            estado_mov = 1;
        } else if (estado == Espera) {
            estado_mov = 0;
        } else if (estado == Movimiento) {
            estado_mov = 1;
        } else if (estado == Captura) {
            estado_mov = 0;
        } else if (estado == Transmision) {
            estado_mov = 0;
        }

        // Se actualiza el bit de salida
        if (estado == Inicializacion) {
            LATCCLR = (1 << LED_ROJO); // Para indicar que no se ha empezado la partida
        } else {
            LATCSET = (1 << LED_ROJO);
        }

        sel++; // incrementamos la selecciÔøΩn para leer otro boton en la siguiente iteraciÔøΩn
        actualizaSel(sel);
        if (sel == 16) { // 64 para el tablero completo
            sel = 0;
        }
    }
    return 0;
}

void InicializacionTablero() {
    // tableroConPiezas representa la posicion inicial en binario
    uint8_t tableroConPiezas[COLUMNAS][FILAS];
    // Dos primeras y dos ÔøΩltimas filas con 1, y el resto con 0
    for (int c = 0; c < 4; c++) {
        for (int f = 0; f < 4; f++) {
            // if (f < 2 || f >= 6) {
                tableroConPiezas[c][f] = 1;
            /* }else {
                tableroConPiezas[c][f] = 0;
            }*/
        }
    }

    if (tablero == tableroConPiezas) {
        inicializado = 1;
        memcpy(pulsador_ant, tableroConPiezas, sizeof(tablero));
    } else {
        inicializado = 0;
    }
}
// Funci√≥n que coloca los valores iniciales para las piezas en sus casillas
void RepresentarTab() {
    for (int c = 0; c < 4; c++) {
        for (int f = 0; f < 4; f++) {
            pieza p;
            p.columna = c;
            p.fila = f;

            if (f == 1 || f == 2) {
                p.nombre = PEON;
                p.color = (f == 1) ? 1 : 0; // 1 para blanco, 0 para negro
            } else if (f == 0 || f == 2) {
                p.color = (f == 0) ? 1 : 0; // 1 para blanco, 0 para negro
                if (c == 0 || c == 3) {
                    p.nombre = TORRE;
                } else if (c == 2) {
                    p.nombre = DAMA;
                } else if (c == 1) {
                    p.nombre = REY;
                }
            } else {
                p.nombre = VACIO; // Casilla vac√≠a
                p.color = 2; // Color no aplicable
            }

            tableroPartida[c][f] = p;
        }
    }
    
    /*
    for (int c = 0; c < 8; c++) {
        for (int f = 0; f < 8; f++) {
            pieza p;
            p.columna = c;
            p.fila = f;

            if (f == 1 || f == 6) {
                p.nombre = PEON;
                p.color = (f == 1) ? 1 : 0; // 1 para blanco, 0 para negro
            } else if (f == 0 || f == 7) {
                p.color = (f == 0) ? 1 : 0; // 1 para blanco, 0 para negro
                if (c == 0 || c == 7) {
                    p.nombre = TORRE;
                } else if (c == 1 || c == 6) {
                    p.nombre = CABALLO;
                } else if (c == 2 || c == 5) {
                    p.nombre = ALFIL;
                } else if (c == 3) {
                    p.nombre = DAMA;
                } else if (c == 4) {
                    p.nombre = REY;
                }
            } else {
                p.nombre = VACIO; // Casilla vac√≠a
                p.color = 2; // Color no aplicable
            }

            tableroPartida[c][f] = p;
        }
    }
    */
}

void Lectura(uint8_t sel, uint8_t estado_mov) {
    uint8_t fila = (sel >> 2) & 0x3; //     uint8_t fila = (sel >> 3) & 0x7;
    uint8_t columna = (sel >> 0) & 0x3; //     uint8_t columna = (sel >> 0) & 0x7;
      
    pulsador_act[columna][fila] = (PORTA >> MUX_IN) & 0x1;
    if (pulsador_ant[columna][fila] < pulsador_act[columna][fila]) { // Se ha levantado una pieza
        piezaLevantada = 1;
        piezaColocada = 0;
        if(estado_mov == 0) { // Si capturamos la pieza no nos interesa guardar la pieza que comemos
            levantarPieza(columna, fila);
        } else {
            tableroPartida[columna][fila].nombre = VACIO;
            tableroPartida[columna][fila].color = 2;
        }
        
    } else if (pulsador_ant[columna][fila] > pulsador_act[columna][fila]) { // Se ha colocado una pieza
        piezaColocada = 1;
        piezaLevantada = 0;
        if(estado_mov) { // si se est· inicializando
            colocarPieza(columna, fila);
        }
    }  
    pulsador_ant[columna][fila] = pulsador_act[columna][fila];
}

void levantarPieza(uint8_t columna, uint8_t fila) {
    pieza p = tableroPartida[columna][fila];
    piezaEnMano = p;
    tableroPartida[columna][fila].nombre = VACIO;
    tableroPartida[columna][fila].color = 2;
}

void colocarPieza(uint8_t columna, uint8_t fila) {
    tableroPartida[columna][fila] = piezaEnMano;
}

// actualiza los registros de salida para los multiplexores
void actualizaSel(uint8_t sel) {
    uint8_t sel_fila, set_columna, sel_A, sel_B; 
    sel_fila = (sel >> 0) & 0x3;
    set_columna = (sel >> 2) & 0x3;

    /*sel5 = (sel >> 4) & 0x1;
    sel6 = (sel >> 5) & 0x1;*/

    LATC &= ~(0x3 << 6);
    asm("nop");
    LATC |= (sel_fila << 6);
    
    TRISA |= (0x3 << 0);
    TRISB |= (0x3 << 2);
    
    if(sel==0){
        sel_A = 0x1;
        TRISA &= (1<<0);
        sel_B = 0;
        LATA |= (1<<0);
    } else if (sel==0x1){
        sel_A = 0x2;
        TRISA &= ~(1<<1);
        sel_B = 0;
        LATA |= (1<<1);
    }  else if (sel==0x2){
        sel_A = 0;
        TRISB &= ~(1<<2);
        sel_B = 0x1;
        LATB |= (1<<2);
    } else if (sel==0x3){
        sel_A = 0;
        TRISB &= (1<<3);
        sel_B = 0x2;
        LATB |= (1<<3);
    }
    
    /*LATA &= ~(0x3 << 0);
    asm("nop");
    LATA |= (sel_A << 0);
    
    LATB &= ~(0x3 << 2);
    asm("nop");
    LATB |= (sel_B << 2);*/
    // Con el tablero reducido a 4x4, solo necesitaremos 4 bits del sel
    /*LATA &= ~(0x1 << SEL5);
    asm("nop");
    LATA |= (sel5 << SEL5);
    
    LATB &= ~(0x1 << SEL6);
    asm("nop");
    LATB |= (sel6 << SEL6);*/
}

char getColumna(pieza p) {
    char col = 7; // en este caso serÔøΩa un error obtener un 7
    if (p.columna == 0){
        col = A;
    } else if (p.columna == 1){
        col = B;
    } else if (p.columna == 2){
        col = C;
    } else if (p.columna == 3) {
        col = D;
    }   
    return col;
}
