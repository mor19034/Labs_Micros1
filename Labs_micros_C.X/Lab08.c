/* 
 * File:   Lab08.c
 * Author: pablo
 *
 * Created on 20 de abril de 2021, 11:17
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
//-------------------------------dvalores definidos-----------------------------
#define N_timer0 236 //VALOR N para que timer 0 tarde 5ms 
#define _XTAL_FREQ 4000000
//----------------------------------tabla---------------------------------------
//los numeros incrementan de 0 hasta 9 
char tabla [10] = {0B00111111, 0B00000110, 0B01011011, 0B01001111, 0B01100110,
0B01101101, 0B01111101, 0B00000111, 0B01111111, 0B01100111};

//----------------------------------variables-----------------------------------
int flags; 
int unidades;
int decenas;
int centenas;
int var;
int residuo;
//---------------------------------prototipos-----------------------------------
void setup (void);
int division(void);
//-------------------------------interrupciones---------------------------------
void __interrupt()interrupcion(void){
//----------------------------interrupcion timer0-------------------------------
    if(T0IF == 1){  //revisa si hay overflow en timer0
        PORTA = 0X00;
           
        if(flags==0b00000000){  //encendido de primer display para unidades
          PORTBbits.RB2 = 0;
          PORTBbits.RB0 = 1;
          PORTD = (tabla[unidades]); 
          flags=0b00000001;
        }
        
        else if(flags==0b00000001){ //encendido de segundo display para decenas
           PORTBbits.RB0 = 0;
           PORTBbits.RB1 = 1;
           PORTD = (tabla[decenas]);
           flags = 0b00000010;
       }
        
        else if(flags==0b00000010){ //encendido de tercer display para centenas
           PORTBbits.RB1 = 0;
           PORTBbits.RB2 = 1;
           PORTD = (tabla[centenas]);
           flags = 0b00000000;
       }
    TMR0 = N_timer0;    //cargar de nuevo valor de timer0
    INTCONbits.T0IF=0;  //bajar bandera de interrupcion de timer 0
    }    
  
//-----------------------------iterrupcion ADC----------------------------------    
    if(ADIF == 1){ //revisa si hay interrupcion en ADC
      if(ADCON0bits.CHS == 0){ //revisar el primer canal
          PORTC = ADRESH;      
      }  
      else {    //sino, 
          var = ADRESH;             
      }
      PIR1bits.ADIF = 0; //bajar bandera de interrupcion de ADC
    }
    return;
}

//------------------------------loop principal----------------------------------
void main(void) {
    setup();
    ADCON0bits.GO = 1;  //es la señal de iniciar la conversión
    while (1){ 
        division();   //Se llama a la division
        
        if (ADCON0bits.GO == 0){    //revisamos si ya se incia la conversión
            if (ADCON0bits.CHS == 0){ //se revisa si el canal 
                ADCON0bits.CHS = 1;
            }
            else {
                ADCON0bits.CHS = 0; //si GO esta en 1 
            }
            __delay_us(200);
            ADCON0bits.GO = 1;
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
    TRISB = 0b00000000;     //Puerto B como salida
    TRISC = 0b00000000;     //Puerto C como salida
    TRISD = 0b00000000;     //Puerto D como salida    
    
    PORTA = 0X00;           //Limpiar puertos
    PORTB = 0X00;
    PORTC = 0X00;
    PORTD = 0X00;
    
    //configuracion de reloj
   
    OSCCONbits.SCS = 1;     //RELOJ INTERNO
    OSCCONbits.IRCF0 = 0;   //OSCILADOR DE 4MHz 011
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    
    //Configuracion timer_0
    OPTION_REGbits.T0CS= 0; //interrupcion cada FOSC/4
    OPTION_REGbits.PSA = 0; //preescalar a timer0
    OPTION_REGbits.PS0 = 1; //preescalar de 256
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS2 = 1;
    TMR0 = N_timer0;   
    INTCONbits.T0IF=0;   //bajar bandera de interrupcion
    
    //Configuracion ADC
    ADCON1bits.ADFM = 0;
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    
    ADCON0bits.ADCS0 = 1;   //FOSC/8
    ADCON0bits.CHS = 0;     //Canal 0
    __delay_us(200);
    ADCON0bits.ADON = 1;
    __delay_us(2000);
    
    //Configuracion de interrupciones
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;  //activar interrupciones de ADC
    INTCONbits.T0IE = 1;  //activar interrupción de timer0 
    PIE1bits.ADIE = 1;  
    
    INTCONbits.T0IF = 0;  //bajar bandera de interrupcion de timer 0
    PIR1bits.ADIF = 0;
    //------------------variable para multiplexear-valorinicial-----------------
    flags =0b00000000;
    return;
}

int division (void){
    centenas = var/100;  //division para encontrar cantida de centenas
    residuo = var%100;   //el residuo que quedo arriba cuando se dividio  
    decenas = residuo/10; //lo que quedo de residuo pasa a entero 
    unidades = residuo%10; //lo que quedo de decenas lo pasa a unidades
}
