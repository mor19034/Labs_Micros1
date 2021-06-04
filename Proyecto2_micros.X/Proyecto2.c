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
char dato;
char localidad;
char lectura1;
char lectura2;
char var1;
char var2;
char bandera;
//---------------------------------prototipos-----------------------------------
void setup (void);
void escribir (char dato, char localidad);
char leer (char localidad);
void putch (char valor); //función para recibir dato que se manda -caracter por
//caracter-
void terminal (void); //función para desplegar texto en pantalla
//-------------------------------interrupciones---------------------------------
void __interrupt()interrupcion(void){
  
//-----------------------------iterrupcion ADC    
    if(ADIF == 1){ //revisa si hay interrupcion en ADC
      if(ADCON0bits.CHS == 0){
          var1 = ADRESH;
          CCPR1L = (var1 >> 1)+125;
        }
        else if(ADCON0bits.CHS == 1) {
            var2 = ADRESH;
            CCPR2L = (var2 >> 1)+125;
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
   if (motores == 255){
       motores = 0;} //cuando en angulo llegue a su maximo  
   INTCONbits.T0IF = 0;
}
//------------------------interrupción Puerto B
if (RBIF == 1){ //revisar si se toca algún botón
    if (PORTBbits.RB6 == 0){ //revisar si se toca boton de escritura
        PORTEbits.RE0 = 1; //se enciede led de grabado
        
        escribir(var1, 0x17);
        escribir(var2, 0x18);
        
        __delay_ms(250); //
    }
    else if (PORTBbits.RB7 == 0){ //si presiono primer botón -reproducción-
        ADCON0bits.ADON = 0; 
        PORTEbits.RE1 = 1; //led que indica que se inicio la lectura
        lectura1 = leer (0x17);
        lectura2 = leer (0x18);
        
        CCPR1L = (lectura1 >> 1)+125;
        CCPR2L = (lectura2 >> 1)+125;
        
        __delay_ms(2000); //lo muestra por 2 segundos y regresa.
        ADCON0bits.ADON = 1;
    }
    else if (PORTBbits.RB5 == 0){ //esto sirve como seguridad para EEUSART
        //nos va a permitir para encenderla y apagarla, sino muere el codigo
        if (bandera == 0){
          bandera = 1;  
        }
        else {
            bandera = 0;
        }
    }
    else { //cuando se salga de grabar o leer se apagan los leds
      PORTEbits.RE0 = 0; //los leds siempre están apagados
      PORTEbits.RE1 = 0;
    }
    INTCONbits.RBIF = 0; //bajar la bandera 
}    
}

//-------------------------------loop principal---------------------------------
void main (void) {
    setup();
    ADCON0bits.GO = 1; //se empieza la conversión   
     
    while (1){ 
        if (bandera == 1){
            terminal(); 
        }
        //esto sirve par el cambio de canales
        else {
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
    return;
}
//-----------------------------------configuracion------------------------------
void setup (void){
    //Estabalecer que puertos vamos a usar 
    ANSEL = 0b00001111;     //RA0, RA1, RA3 y RA4 como entradas analógicas
    ANSELH= 0;
    
    TRISA = 0b00001111;     //Primeros cuatro pines de puerto A como entrada 
    TRISB = 0b11100000;     //Puerto B, ultimos pines como entrada
    TRISC = 0b00000000;     //Puerto C como salida
    TRISD = 0b00000000;     //Puerto D como salida   
    TRISE = 0b000;          //
    
    PORTA = 0X00;           //Limpiar puertos
    PORTB = 0X00;
    PORTC = 0X00;
    PORTD = 0X00;
    PORTE = 0x00;
    
    //configuracion de reloj
    OSCCONbits.SCS = 1;     //RELOJ INTERNO
    OSCCONbits.IRCF = 0b111;   //OSCILADOR DE 8MHz 111
    
    //CONFIGURACION DE PUERTO B
    OPTION_REGbits.nRBPU=0;  //Habilitar weak pull-up de puerto B
    WPUBbits.WPUB6 = 1;     //weak pull-ups de últimos dos pines en B
    WPUBbits.WPUB7 = 1;
    WPUBbits.WPUB5 = 1;
    IOCBbits.IOCB6 = 1;     //Habilitar interrupciones en puertos
    IOCBbits.IOCB7 = 1;
    IOCBbits.IOCB5 = 1;
    
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
    INTCONbits.RBIE = 1; 
    INTCONbits.RBIF = 1;
    INTCONbits.GIE = 1;
    
//    //Interrupciones de transmisión recepción activadas
//    PIE1bits.RCIE = 1;
//    PIE1bits.TXIE = 1;
//    //banderas de transmisión y recepción se bajan
//    PIR1bits.TXIF = 0; 
//    PIR1bits.RCIF = 0; 
    
    
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
    
    //Configurasión comunicación serial
    TXSTAbits.SYNC  = 0;    //Modo asincrono
    TXSTAbits.BRGH  = 1;    //Tranmision de datos a alta velocidad
    
    BAUDCTLbits.BRG16   = 1;
    
    SPBRG   = 207;  //Baud rate de 9600
    SPBRGH  = 0;
    
    RCSTAbits.SPEN  = 1;    //Se activan los puertos seriales
    RCSTAbits.RX9   = 0; 
    RCSTAbits.CREN  = 1;    //Se enciende la recepción continua
    
    TXSTAbits.TXEN  = 1;    //Se activa la transmición
    
}
//------------------------------------funciones---------------------------------
//----------------------------escribir en la EPPROM-----------------------------
void escribir (char dato, char localidad){
    EEADR = localidad;  
    EEDAT = dato;  
    
    INTCONbits.GIE = 0; //apagar las interrupciones globales
    
    EECON1bits.EEPGD = 0; //puntero para DATA memory
    EECON1bits.WREN = 1; //Se habilita escribir en la memoria
    
    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1; //se inicia el ciclo de escritura
    
    while(PIR2bits.EEIF == 0);
    PIR2bits.EEIF = 0;
    
    EECON1bits.WREN = 0; //seguro para evitar escritura 
}
//------------------------------leer EEPROM-------------------------------------
char leer (char localidad){
    EEADR = localidad; //esto dice que localidad se leerá
    EECON1bits.EEPGD = 0; //se apunta hacia la memoria 
    EECON1bits.RD = 1; //se habilita la lectura
    char dato = EEDAT; //lo que esta en la memoria se va a la variable
    return dato;
}
//--------------------------------comunicacion serial---------------------------
//función de librería de stdio.h
void putch(char valor){ 
    while(TXIF == 0);
    TXREG = valor; //lo que se escribe se manda al pic para que lo procese 
    return; 
}
//Función para las cadenas
void terminal(void){
    __delay_ms(300); //delay en lo para desplegar los caraacteres
    printf("\r Escoge una opcion, chavo: \r");
    __delay_ms(300);
    printf(" (1) Controlar motores: \r");
    __delay_ms(300);
    printf(" (2) ya no, ya no quiero hacer nada UnU: \r");
    
    while (RCIF == 0); //Se revisa el dato recibido
    // Opciones del menú
    
    if (RCREG == '1') { //
        __delay_ms(00);
        printf("\r\rQue motor desea controlar:");
        __delay_ms(100);
        printf("\r\r (1)Giro");
        __delay_ms(100);
        printf("\r\r (2)Modo del aleron");
        __delay_ms(100);
        printf("\r\r (3) Nitro, la velocidad pues");
        
        while (RCIF == 0);  //Esperamos a que el usuario ingrese un dato
        
        if (RCREG == '1') {
            __delay_ms(00);
            printf("\r\r pa donde queres girar?:");
            __delay_ms(100);
            printf("\r\r (a)Derecha");
            __delay_ms(100);
            printf("\r\r (b)Izquierda");
            __delay_ms(100);
            printf("\r\r (c)Centro");
            
            while (RCIF == 0);  //Esperamos a que el usuario ingrese un dato
            
            if (RCREG == 'a') {
                CCPR1L = (250 >> 1) + 125;
            }
            
            else if (RCREG == 'b') {
                CCPR1L = (0 >> 1) + 125;
            }
            
            else if (RCREG == 'c') {
                CCPR1L = (127 >> 1) + 125;
            }
            
            else {
                NULL;
            }   
        }
        
        if (RCREG == '2') {
            __delay_ms(00);
            printf("\r\r modo del aleron:");
            __delay_ms(100);
            printf("\r\r (a) carrera");
            __delay_ms(100);
            printf("\r\r (b) frando");
            __delay_ms(100);
            printf("\r\r (c) normal");
            
            while (RCIF == 0);  //Esperamos a que el usuario ingrese un dato
            
            if (RCREG == 'a') {
                CCPR2L = (250 >> 1) + 125;
            }
            
            else if (RCREG == 'b') {
                CCPR2L = (0 >> 1) + 125;
            }
            
            else if (RCREG == 'c') {
                CCPR2L = (127 >> 1) + 125;
            }
            
            else {
                NULL;
            }   
        }
        
        if (RCREG == '3') {
            __delay_ms(00);
            printf("\r\r Que velocidad quiere?:");
            __delay_ms(100);
            printf("\r\r (a) rapidos y furiosos");
            __delay_ms(100);
            printf("\r\r (b) jue madre, frenar");
            __delay_ms(100);
            printf("\r\r (c)meh, normal");
            
            while (RCIF == 0);  //Esperamos a que el usuario ingrese un dato
            
            if (RCREG == 'a') {
                valor_ADC = 255;
            }
            
            else if (RCREG == 'b') {
                valor_ADC = 0;
            }
            
            else if (RCREG == 'c') {
                valor_ADC = 127;
            }
            
            else {
                NULL;
            }   
        }
        
    }
    
    else if (RCREG == '2') {    //Si presionamos dos enviamos un caracter a PORTB
        __delay_ms(500);    //Preguntamos el caracter
        printf("\r\rAdios choripan\r");
        bandera = 0;
    } 
    
    else {  //Si el usuario presiona cualquier otro caracter no sucede nada
        NULL; 
    }
    return;
}
