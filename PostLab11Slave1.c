/*
 * File:   PostLab11Slave1.c
 * Author: jorge
 *
 * Created on 13 de mayo de 2022, 11:49 PM
 */
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>             // int8_t, unit8_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _XTAL_FREQ 1000000

#define IN_MIN 0                
#define IN_MAX 255              // Valores de entrada a Potenciometro
#define OUT_MIN 62
#define OUT_MAX 125             // Valores para el Servomotor

unsigned short CCPR = 0;
uint8_t val_temporal = 0;           // Variable temporal
int PWMTIMER = 0;

void setup(void);
unsigned short map(uint8_t val, uint8_t in_min, uint8_t in_max, 
            unsigned short out_min, unsigned short out_max);

void __interrupt() isr (void){
    if(PIR1bits.SSPIF){                 // Interrupcion si el esclavo recibió la información
        val_temporal = SSPBUF;          // Cargar el valor del maestro a variable temporAL
        CCPR = map(val_temporal, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);   // Valor de ancho de pulso
        CCPR1L = (uint8_t)(CCPR>>2);    // 8 bits mas significativos en CPR1L
        CCP1CONbits.DC1B = CCPR & 0b11; // 2 bits menos significativos en DC1B
        PIR1bits.SSPIF = 0;             // Limpiar la bandera de SPI
    }
    return;
}

void setup(void){
    ANSEL = 0;                         
    ANSELH = 0;                 // I/O digitales
    
    TRISA = 0b00100000;         // RA5 como entradas
    TRISC = 0b00111000;         // SD1 entrada, SCK y SD0 como salida
    PORTCbits.RC5 = 0;
    TRISD = 0;                  // PORTD como salida
    PORTA = 0;                  // Se limpia PORTA
    PORTC = 0;                  // Se limpia PORTC
    // Configuración del oscilador
    OSCCONbits.IRCF = 0b0100;   // 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    // Configuración de interrupciones
    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupciones de periféricos
    
    // Configuración de Esclavo
    // Configuración de SPI
    SSPCONbits.SSPM = 0b0100;   // SPI Maestro, Reloj -> FOSC/4  (250 KBIS/s)
    SSPCONbits.CKP = 0;         // Reloj inactivo
    SSPCONbits.SSPEN = 1;       // Habilitar pines de SPI
    SSPSTATbits.CKE = 1;        // Dato enviado cada flanco de subida
    SSPSTATbits.SMP = 0;        // Dato al final del pulso de reloj

        
    PIR1bits.SSPIF = 0;         // Limpiar bandera de SPI
    PIE1bits.SSPIE = 1;         // Habilitar interrupcion de SPI
    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupciones de periféricos
    
    // Configuración PWM
    TRISCbits.TRISC2 = 1;           // Deshabilitar salida de CCP1 (Se pone como entrada)
    PR2 = 255;                      // Periodo de 16 ms
    
    // Configuracion CCP
    CCP1CON = 0;                    // Apagar CCP1
    CCP1CONbits.P1M = 0;            // Modo sigle output
    CCP1CONbits.CCP1M = 0b1100;     // PWM
    
    CCPR1L = 61>>2;
    CCP1CONbits.DC1B = 61 & 0b11;
    
    PIR1bits.TMR2IF = 0;            // Limpiar bandera de TMR2
    T2CONbits.T2CKPS = 0b11;        // Prescaler 1:16
    T2CONbits.TMR2ON = 1;           // Encender TMR2
    while (!PIR1bits.TMR2IF);       // Esperar un ciclo del TMR2
    PIR1bits.TMR2IF = 0;
    
    TRISCbits.TRISC2 = 0;           // Habilitar salida de PWM

}

void main(void) {
    setup();
    while(1){        
        // Envio y recepcion de datos en maestro
    }
    return;
}

unsigned short map(uint8_t x, uint8_t x0, uint8_t x1, unsigned short y0, unsigned short y1){
    return (unsigned short)(y0+((float)(y1-y0)/(x1-x0))*(x-x0));
}