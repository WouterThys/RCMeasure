/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SETTINGS_H
#define	SETTINGS_H

#include <xc.h> // include processor files - each processor file is guarded.  

/**
 * Interrupt priorities
 */

#define CN_IP       2     
#define T2_IP       1  
    
/**
 *  PIC
 */
 
#define LED1            PORTBbits.RB15
#define LED1_Dir        TRISBbits.TRISB15
#define LED2            PORTBbits.RB14
#define LED2_Dir        TRISBbits.TRISB14
    
/**
 * UART
 */      
#define UART1_BAUD      9600
#define UART1_ID        2   

#define UART1_RX_Dir    TRISBbits.TRISB6    /* RX Pin on RP38-RB6             */
#define UART1_TX_Dir    TRISBbits.TRISB7    /* TX Pin on RP39-RB7             */

#define UART1_RX_Map    0b0100110           /* I/O RP38                       */
#define UART1_TX_Map    0b000001

    

#endif	/* XC_HEADER_TEMPLATE_H */

