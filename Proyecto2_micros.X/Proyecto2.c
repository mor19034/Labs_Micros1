/* 
 * File:   Proyecto2.c
 * Author: pablo
 *
 * Created on 18 de mayo de 2021, 10:18
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <pic16f887.h>// </editor-fold>
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

//------------------------valores definidos y variables-------------------------
#define _XTAL_FREQ 8000000
int N_timer0;
int motores;
int valor_ADC;
//---------------------------------prototipos-----------------------------------
void setup (void);
//-------------------------------interrupciones---------------------------------
void __interrupt()interrupcion(void){
  
//-----------------------------iterrupcion ADC    
    if(ADIF == 1){ //revisa si hay interrupcion en ADC
      if(ADCON0bits.CHS == 0){
            CCPR1L = (ADRESH>>1)+125;
        }
        else if(ADCON0bits.CHS == 1) {
            CCPR2L = (ADRESH>>1)+125;
        }
        else if (ADCON0bits.CHS == 2){
           valor_ADC = ADRESH;
        }
        PIR1bits.ADIF = 0;
    }
    
//------------------------interrupción TMR0
if (INTCONbits.T0IF == 1){ 
    motores++;
   if (motores >= valor_ADC){
       RD0 = 0;
       RD1 = 0;
   }
   else{
       RD0 = 1;
       RD1 = 1; 
   }
   if (motores == 255){motores = 0;} //cuando en angulo llegue a su maximo  
   INTCONbits.T0IF = 0;
}
    return;
}

//-------------------------------loop principal---------------------------------
void main (void) {
    setup();
    ADCON0bits.GO = 1; //se empieza la conversión   
     
    while (1){ 
        //esto sirve par el cambio de canales
        if (ADCON0bits.GO == 0){    //revisar si se empieza conversion
            if (ADCON0bits.CHS == 0){ //si esta en canal 0, cambiamos a 1
                ADCON0bits.CHS = 1;
            }
            else if (ADCON0bits.CHS == 1){
                ADCON0bits.CHS = 2;
            }
            else if (ADCON0bits.CHS == 2){
                ADCON0bits.CHS = 0;
            }
            __delay_us(100);    //delay para hacer cambio de canal
            ADCON0bits.GO = 1;  //se comienza otra conversión
        }
    }
}

//-----------------------------------configuracion------------------------------
void setup (void){
    //Estabalecer que puertos vamos a usar 
    ANSEL = 0b00001111;     //RA0, RA1, RA3 y RA4 como entradas analógicas
    ANSELH= 0;
    
    TRISA = 0b00001111;     //Primeros cuatro pines de puerto A como entrada 
    TRISB = 0b00000000;     //Puerto B como salida
    TRISC = 0b00000000;     //Puerto C como salida
    TRISD = 0b00000000;     //Puerto D como salida    
    
    PORTA = 0X00;           //Limpiar puertos
    PORTB = 0X00;
    PORTC = 0X00;
    PORTD = 0X00;
    
    //configuracion de reloj
    OSCCONbits.SCS = 1;     //RELOJ INTERNO
    OSCCONbits.IRCF = 0b111;   //OSCILADOR DE 8MHz 111
    
    
    //Configuracion ADC
    ADCON1bits.ADFM = 0; //Leer bits más significativos; justificado izquierda
    ADCON1bits.VCFG0 = 0; //Voltajes de referencia VSS y VDD
    ADCON1bits.VCFG1 = 0;
    
    ADCON0bits.ADCS = 0b10;   //reloj de conversión FOSC/32
    ADCON0bits.CHS = 0;     //se empieza conversion en canal 0
    ADCON0bits.ADON = 1;    //se enciende el ADC
    __delay_us(100);
    
    //configuración de PWM
    TRISCbits.TRISC1 = 1;   //Salidas de la señal PWM
    TRISCbits.TRISC2 = 1;   
    
    PR2 = 250;  //darle el valor necesario para que mi PWM sea de 2ms
    CCP1CONbits.P1M = 0;    //configuracion modo PWm
    CCP1CONbits.CCP1M = 0b1100; //Primer PWM
    CCP2CONbits.CCP2M = 0b1100; //Segundo PWM
    
    CCPR1L = 0x0f;  //ciclo de trabajo normal
    CCP1CONbits.DC1B = 0;
   
    //Configuracion de interrupciones
    PIR1bits.ADIF = 0; //se baja la bandera del ADC
    PIE1bits.ADIE = 1;  //convertidor ADC interrupciones habilitadas    
    INTCONbits.PEIE = 1;  //activar interrupciones perifericas
    INTCONbits.T0IE = 1;  //habilitar interrupcion de timer0
    INTCONbits.T0IF = 0; //limpiar bandera de tmr0
    INTCONbits.GIE = 1;
    
    
    //Configuración TMR2
    PIR1bits.TMR2IF = 0;    //bajar bandera de timer2
    T2CONbits.T2CKPS = 0b11; //preescalar 1:16 revisar que preescalar usar
    T2CONbits.TMR2ON = 1;
    
    while(PIR1bits.TMR2IF == 0); 
    PIR1bits.TMR2IF = 0;
    
    //SALIDA DE PWM
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;   //salida de PWM
    
    //Configuración TMR0
    OPTION_REGbits.T0CS = 0; //FOSC/4
    OPTION_REGbits.PSA = 0; //incrementar en flanco positivo de reloj
    OPTION_REGbits.PS = 0b011; //preescalar de 16
    TMR0 = 246; 
}