/* 
 * File:   Lab09.c
 * Author: pablo moreno
 *
 * Created on 26 de abril de 2021, 23:06
 */
#include <stdio.h>
#include <stdlib.h>

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
//---------------------------dvalores definidos-----------------------------
#define _XTAL_FREQ 8000000

//---------------------------------prototipos-----------------------------------
void setup (void);
//-------------------------------interrupciones---------------------------------
void __interrupt()interrupcion(void){
  
//-----------------------------iterrupcion ADC----------------------------------    
    if(ADIF == 1){ //revisa si hay interrupcion en ADC
      if(ADCON0bits.CHS == 0){ //revisar el primer canal
          CCPR2L = (ADRESH>>1)+125; //  
      }  
      else {
          CCPR1L = (ADRESH>>1)+125;
      }

      PIR1bits.ADIF = 0; //bajar bandera de interrupcion de ADC
    }
    return;
}

//------------------------------loop principal----------------------------------
void main(void) {
    setup();
  
    while (1){ 
        //esto sirve par el cambio de canales
        if (ADCON0bits.GO == 0){    //revisamos si ya se incia la conversión
            if (ADCON0bits.CHS == 0){ //se revisa si el canal es 1, sino, cambia
                ADCON0bits.CHS = 1;
            }
            else {
                ADCON0bits.CHS = 0; //si GO esta en 1 
            }
            __delay_us(100);    //delay para hacer cambio de canal
            ADCON0bits.GO = 1;  //se activan la conversion de ADC
        }
    }
    return;
}
//------------------------------configuraciones---------------------------------
void setup(void){
    //Estabalecer que puertos vamos a usar 
    ANSEL = 0b00000011;     //RA0 y RA1 como entradas analógicas
    ANSELH= 0;
    
    TRISA = 0b00000011;     //Primeros dos pines de puerto A como entrada 
    TRISC = 0b00000000;     //Puerto C como salida
   
    PORTA = 0X00;           //Limpiar puertos
    PORTC = 0X00;

    
    //configuracion de reloj
    OSCCONbits.SCS = 1;     //RELOJ INTERNO
    OSCCONbits.IRCF0 = 1;   //OSCILADOR DE 8MHz 111
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    
    //Configuracion de interrupciones
    INTCONbits.GIE = 1;
    PIR1bits.ADIF = 0;
    INTCONbits.PEIE = 1;  //activar interrupciones de ADC
    PIE1bits.ADIE = 1;  
    
    //Configuracion ADC
    ADCON1bits.ADFM = 0; //Leer bits menos significativos; justificado izquierda
    ADCON1bits.VCFG0 = 0; //Voltajes de referencia VSS y VDD
    ADCON1bits.VCFG1 = 0;
    
    ADCON0bits.ADCS = 0b10;   //reloj de conversión FOSC/32
    ADCON0bits.CHS = 0;     //Canal 0
    ADCON0bits.ADON = 1;    //se enciende el ADC
    __delay_us(50);
    
    //configuración de PWM
    TRISCbits.TRISC1 = 1;   //Salidas de la señal PWM
    TRISCbits.TRISC2 = 1;   
    
    PR2 = 250;  //darle el valor necesario para que mi PWM sea de 2ms
    CCP1CONbits.P1M = 0;    //configuracion modo PWm
    CCP1CONbits.CCP1M = 0b1100; //Primer PWM
    CCP2CONbits.CCP2M = 0b1100; //Segundo PWM
    
    CCPR1L = 0x0f;  //ciclo de trabajo normal
    CCP1CONbits.DC1B = 0;
    
    //Configuración TMR2
    PIR1bits.TMR2IF = 0;    //bajar bandera de timer2
    T2CONbits.T2CKPS = 0b11; //preescalar 1:16 revisar que preescalar usar
    T2CONbits.TMR2ON = 1;
    
    while(PIR1bits.TMR2IF == 0); 
    PIR1bits.TMR2IF = 0;
    
    //SALIDA DE PWM
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;   //salida de PWM
    
    return;
}