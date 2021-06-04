/*
 * File:   Proyecto2.c
 * Author: Josue Salazar
 *
 * Created on 25 de mayo de 2021, 12:16 PM
 */


#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 8000000 

///////Prototipos////////
void confi(void);
void ISR (void);
void esc_EEP (char data, char direc);
char leer_EEP (char direc);
//////VARIABLES/////
int motor;
int ciclo;
char data;
char direc;
char lec1;
char lec2;
char lec3;




/////Interrupcion///////////

void __interrupt() ISR(void){
    if (PIR1bits.ADIF){
        if(ADCON0bits.CHS == 0){
            CCPR1L = (ADRESH>>1)+127;
            lec1 =(ADRESH>>1)+127;
            
        }
        else if(ADCON0bits.CHS == 1) {
            CCPR2L = (ADRESH>>1)+127;
            lec2 = (ADRESH>>1)+127; 
        }
        else if (ADCON0bits.CHS == 2){
           motor = (ADRESH);
          
          
        }
        PIR1bits.ADIF = 0;
    }
    if (RBIF ==1)
    {
        if (PORTBbits.RB1 == 0){
            ADCON0bits.ADON = 0;
            PORTBbits.RB6 = 1;
            lec1 = leer_EEP (0x17);
            lec2 = leer_EEP (0x18);
            
            
            CCPR1L = (lec1 >> 1)+127;
            CCPR2L = (lec2 >> 1)+127;
            __delay_ms(1000);
            ADCON0bits.ADON = 1;
            
            
        }
        else if (PORTBbits.RB0 == 0){
            PORTBbits.RB7 = 1;
            esc_EEP (lec1, 0x17);
            esc_EEP (lec2, 0x18);
   
            __delay_ms(1000);
        
        }
        else {
            PORTBbits.RB6 = 0;
            PORTBbits.RB7 = 0;
            
        
        }
        INTCONbits.RBIF = 0;
    }
   ////INT TMR0/////
    if (INTCONbits.T0IF ==1){
        ciclo++;
        if (ciclo > motor) {
            RD2 = 0;
            RD3 = 0;
            
        }
        else {
            RD2 = 1;
            RD3 =1;
          
        }
       
        if (ciclo == 255)
        {
            ciclo = 0;
        }
        INTCONbits.T0IF = 0;
    }
    return;
}
void esc_EEP (char data, char direc){
    EEADR = direc;
    EEDAT = data; 
    
    INTCONbits.GIE = 0;
    
    EECON1bits.EEPGD = 0;
    EECON1bits.WREN = 1;
    
    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;
    
    while(PIR2bits.EEIF == 0);
    PIR2bits.EEIF = 0;
    
    
    EECON1bits.WREN = 0;
    
    

}

char leer_EEP (char direc){
    EEADR = direc;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1;
    char data = EEDATA;
    return data; 
   
}

void main(void) {
    confi();
    ADCON0bits.GO = 1;
    
    while(1)
    {
    
        if(ADCON0bits.GO == 0){
            if (ADCON0bits.CHS == 0){
                ADCON0bits.CHS = 1;
            }
            else if (ADCON0bits.CHS == 1){
                ADCON0bits.CHS = 2;
            }
            else if (ADCON0bits.CHS == 2) {
                ADCON0bits.CHS = 0;
            }
            __delay_us(200);
            ADCON0bits.GO = 1;
        
        }
    }
    
    return;
}

void confi(void){
  ANSEL = 0b00000111;
  ANSELH = 0X00;
  //Aqui configuramos entradas y salidas
  TRISA = 0b00000111;
  TRISD = 0X00;
  TRISB = 0b00000111;
  
  WPUB = 0X03;
  IOCB = 0x03;    
  
  PORTB = 0X00;
  PORTA = 0X00;
  PORTD = 0X00;
  // colocamos nuestro oscilador interno en 8Mhz
  OSCCONbits.IRCF2 = 1;
  OSCCONbits.IRCF1 = 1;
  OSCCONbits.IRCF0 = 1; 
  OSCCONbits.SCS = 1;

  //Configuracion del ADC
  ADCON1bits.ADFM = 0;
  ADCON1bits.VCFG0 = 0;
  ADCON1bits.VCFG1 = 0;
  
  ADCON0bits.ADCS = 0b10;
  ADCON0bits.CHS = 0;
  __delay_us(200);
  ADCON0bits.ADON = 1;
  __delay_us(200);
 
  
  
   // cinfuguracion de PWM
  TRISCbits.TRISC2 = 1;
  TRISCbits.TRISC1 = 1;
  PR2 = 250;
  CCP1CONbits.P1M = 0;
  CCP2CONbits.CCP2M = 0b1100;
  CCP1CONbits.CCP1M = 0b1100;
  
  CCPR1L = 0X0F;
  CCPR2L = 0X0F;
  CCP1CONbits.DC1B = 0;
  CCP2CONbits.DC2B0 = 0;
  CCP2CONbits.DC2B1 = 0;
  
 // CONFIGURACION TMR0 
  OPTION_REGbits.T0CS =0;
  OPTION_REGbits.PSA =0;
  OPTION_REGbits.PS = 0b011;
  TMR0 = 246;
  //Configuracion del TMR2
  
  PIR1bits.TMR2IF = 0;
  T2CONbits.T2CKPS = 0b11;
  T2CONbits.TMR2ON = 1;
  
  while(PIR1bits.TMR2IF == 0);
  PIR1bits.TMR2IF = 0;
  TRISCbits.TRISC2 = 0;
  TRISCbits.TRISC1 = 0;
  
  //Configuracion de interrupciones
  PIR1bits.ADIF = 0;
  PIE1bits.ADIE = 1;
  INTCONbits.PEIE = 1;
  INTCONbits.GIE = 1;
  INTCONbits.T0IE = 1;
  INTCONbits.T0IF = 0;
  INTCONbits.RBIE = 1;
  INTCONbits.RBIF = 0;
  
  
  
  // Configuracion de Pull 
  OPTION_REGbits.nRBPU = 0;
  WPUBbits.WPUB0 = 1;
  WPUBbits.WPUB1 = 1;
  IOCBbits.IOCB0 = 1;
  IOCBbits.IOCB1 = 1;
  
  return;
}