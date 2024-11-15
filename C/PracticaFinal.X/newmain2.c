/* 
 * File:   newmain_sel.c
 * Author: santi
 *
 * Created on April 17, 2024, 1:02 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xc.h>
#include "Pic32Ini.h"
#include "InitTimer2.h"
#include "UART.h"

/*
 * 
 */
#define MUX_IN 7 // RA7
#define MUX_OUT 10 // RA10
#define LED_ROJO 0 // CAMBIAR?
// sel estarÔøΩ en RC4 a RC7, RA9 y RB8
#define SEL 6 //RC6 a RC7
#define SEL5 9 //RA9 
#define SEL6 8 //RB8 
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

void levantarPieza(uint8_t columna, uint8_t fila);
void colocarPieza(uint8_t columna, uint8_t fila);
void inicializar_matrices(void);

static uint8_t tablero[COLUMNAS][FILAS]; // 4 columnas con 4 filas binario
static pieza tableroPartida[COLUMNAS][FILAS]; // tablero con la estructura de pieza
static pieza piezaEnMano; // pieza que se ha levantado
static uint8_t pulsador_ant[COLUMNAS][FILAS], pulsador_act[COLUMNAS][FILAS]; // pulsadores en cada casillas (binario: 0->no hay pieza, 1->hay pieza	)

void actualizaSel(uint8_t sel);
static uint8_t sel = 0; // 4 bits de seleccion, los 2 menos significativos para columnas, los 2 m√°s significativos para filas

int main(void) {

    ANSELA = ~((1 << MUX_IN)|(1<<0)|(1<<9));// penÔøΩltimo bit de sel
    ANSELB = ~((0x3 << 2)|(1<<8));// ÔøΩltimo bit de sel
    ANSELC = ~((1 << LED_ROJO)|(0x3 << SEL));

    LATA = 0X3|(1<<9); // MUX_OUT siempre a 1, ver esquema electrÔøΩnico
    LATB = (1<<8)|(1<<3);
    LATC = ~((1 << LED_ROJO)|(1<<6)|(1<<7)); // Inicialmente encendido

    TRISA = (1 << MUX_IN)|(1<<0)|(1<<9);
    TRISB = (0x3 << 2)|(1<<8);
    TRISC = 0;

    inicializar_matrices();
    //sel = 10;
    while(1){
        
        actualizaSel(sel);
        Lectura(sel, 1);
        sel++; // incrementamos la selecciÔøΩn para leer otro boton en la siguiente iteraciÔøΩn
        if (sel == 16) { // 64 para el tablero completo
            LATCINV = 1<<0;
            sel = 0;
        }
        
    }




    return 0;
}

void actualizaSel(uint8_t sel) {
    uint8_t sel_fila, sel_columna;
    sel_fila = (sel >> 0) & 0x3;
    sel_columna = (sel >> 2) & 0x3;

    /*sel5 = (sel >> 4) & 0x1;
    sel6 = (sel >> 5) & 0x1;*/

    LATC &= ~(0x3 << 6);
    asm("nop");
    LATC |= (sel_fila << 6);

    TRISA |= (1<<0)|(1<<9);
    asm("nop");
    TRISB |= (0x3 << 2)|(1<<8);
    asm("nop");
    /*LATA &=~(0x3 <<0);
    asm("nop");
    LATB &=~(0x3<<2);
    asm("nop");*/
    
    if (sel_columna == 0) {
        TRISA &= ~(1 << 0);
        asm("nop");
        //LATA |= (1 << 0);
        asm("nop");
    } else if (sel_columna == 0x1) {
        asm("nop");
        TRISA &= ~(1 << 9);
        asm("nop");
        //LATA |= (1 << 1);
        asm("nop");
    } else if (sel_columna == 0x2) {
        TRISB &= ~(1 << 8);
        asm("nop");
        //LATB |= (1 << 8);
        asm("nop");
    } else if (sel_columna == 0x3) {
        TRISB &= ~(1 << 3);
        asm("nop");
        //LATB |= (1 << 3);
        asm("nop");
    }
}

void Lectura(uint8_t sel, uint8_t estado_mov) {
    uint8_t fila = (sel >> 0) & 0x3; //     uint8_t fila = (sel >> 3) & 0x7;
    uint8_t columna = (sel >> 2) & 0x3; //     uint8_t columna = (sel >> 0) & 0x7;
      
    pulsador_act[columna][fila] = (PORTA >> MUX_IN) & 0x1;
    if (pulsador_ant[columna][fila] > pulsador_act[columna][fila]) { // Se ha levantado una pieza
        piezaLevantada = 1;
        piezaColocada = 0;
        if(estado_mov == 0) { // Si capturamos la pieza no nos interesa guardar la pieza que comemos
            levantarPieza(columna, fila);
        } else {
            tableroPartida[columna][fila].nombre = VACIO;
            tableroPartida[columna][fila].color = 2;
        }
        pulsador_ant[columna][fila] = pulsador_act[columna][fila];

        
    } else if (pulsador_ant[columna][fila] < pulsador_act[columna][fila]) { // Se ha colocado una pieza
        piezaColocada = 1;
        piezaLevantada = 0;
        if(estado_mov) { // si se est· inicializando
            colocarPieza(columna, fila);
        }
        pulsador_ant[columna][fila] = pulsador_act[columna][fila];

    }  
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

void inicializar_matrices() {
    // Recorrer todas las filas y columnas
    for (int i = 0; i < COLUMNAS; i++) {
        for (int j = 0; j < FILAS; j++) {
            // Asignar cero a cada elemento de ambas matrices
            pulsador_ant[i][j] = 0;
            pulsador_act[i][j] = 0;
        }
    }
}