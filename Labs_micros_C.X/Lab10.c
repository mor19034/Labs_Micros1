/* 
 * File:   Lab10.c
 * Author: pablo moreno
 *
 * Created on 4 de mayo de 2021, 9:57
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements
// CONFIG1

#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = ON      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>// </editor-fold>
#include <stdint.h>
#include <stdio.h> //nos deja usar print 

//----------------------------------Variables-----------------------------------

//--------------------------------valores definidos-----------------------------
#define _XTAL_FREQ 8000000
//---------------------------------prototipos-----------------------------------
void setup (void);  //función para configuraciones
void putch (char dato); //función para recibir dato que se manda -caracter por
//caracter-
void cadena (void); //función para desplegar texto en pantalla
//-------------------------------interrupciones---------------------------------

//-----------------------------------main---------------------------------------
//------------------------------loop principal----------------------------------
void main(void) {
    setup();    //se configuro 
  
    while (1){ //loop infinito
        cadena(); //se llama función  
        }
    }
//---------------------------------funciones------------------------------------
//función de librería de stdio.h
void putch(char dato){ 
    while(TXIF == 0);
    TXREG = dato; //lo que se escribe se manda al pic para que lo procese 
    return; 
}
//Función para las cadenas
void cadena(void){
    __delay_ms(300); //delay en lo para desplegar los caraacteres
    printf("\r Escoge una opcion, chavo: \r");
    __delay_ms(300);
    printf("1. Desplegar cadena de caracteres: \r");
    __delay_ms(300);
    printf("2. Desplegar PORTA: \r");
    __delay_ms(300);
    printf("3. Desplegar PORTB: \r");
    
    while (RCIF == 0); //Se revisa el dato recibido
    // Opciones del menú
    if (RCREG == '1'){ //se revisa si se recibe el número 1
        __delay_ms(500);
        printf("\r Que tas esperando? es lo unico que hago...UwU \r");
    }
    else if (RCREG == '2'){ //segunda opcion del menú
        printf("\r Insertar caracter para desplegar en PORTA: \r");
        while (RCIF == 0); //mientras no se ha recibido nada, no se hace nada
        PORTA = RCREG; //cuando se recibe algo, se manda a PORTA
    }
    else if (RCREG == '3'){ //tercera opcion del menú
        printf("\r Insertar caracter para desplegar en PORTB: \r");
        while (RCIF == 0); //mientras no se ha recibido nada, no se hace nada
        PORTB = RCREG; //caundo se reciba el dato, se manda a PORTB
    }
    else{ //Si se ingresa una opción fuera de la lista, no sucede nada
        __delay_ms(500);
        printf("\r a mi se me hace que no se va a podeer...proba una opcion\r");
        NULL;
    }
    return;
}
//------------------------------configuraciones---------------------------------
void setup(void){
    //Estabalecer que puertos vamos a usar 
    ANSEL = 0b00000000;    
    ANSELH = 0;
    
    TRISA = 0b00000000;     //Primeros dos pines de puerto A como entrada 
    TRISB = 0b00000000;     //Puerto B como salida
   
    PORTA = 0X00;           //Limpiar puertos
    PORTB = 0x00;
    
    //configuracion de reloj oscilador
    OSCCONbits.SCS = 1;     //RELOJ INTERNO
    OSCCONbits.IRCF = 0b111;   //OSCILADOR DE 8MHz 111
    
    //Configuración del RX y TX
    TXSTAbits.SYNC = 0;     //modo sincrono desabilitado
    TXSTAbits.BRGH = 1;     //Bound rate High
    BAUDCTLbits.BRG16 = 1;  //16 bits para generar fire brake
    
    SPBRG = 207;    //SPBRG con 0.16% de error
    SPBRGH = 0;
    
    RCSTAbits.SPEN = 1; //Puertos seriales 
    RCSTAbits.RX9 = 0;  //No se van a recibir 9 bits
    RCSTAbits.CREN = 1; //habilitar recepción
    TXSTAbits.TXEN = 1; //habilitar transmisión
  
    //Configuracion de interrupciones
    INTCONbits.GIE = 1; //globales
    INTCONbits.PEIE = 1;
    //Interrupciones de transmisión recepción activadas
    PIE1bits.RCIE = 1;
    PIE1bits.TXIE = 1;
    //banderas de transmisión y recepción se bajan
    PIR1bits.TXIF = 0; 
    PIR1bits.RCIF = 0;   
}