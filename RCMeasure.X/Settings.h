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


#define DEBUG       1


/**
 * Interrupt priorities
 */

#define CN_IP       2     
#define T2_IP       1  
#define U1RX_IP     3
    
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
#define UART1_BAUD      57600
#define UART1_ID        2   

#define UART1_RX_Dir    TRISBbits.TRISB6    /* RX Pin on RP38-RB6             */
#define UART1_TX_Dir    TRISBbits.TRISB7    /* TX Pin on RP39-RB7             */

#define UART1_RX_Map    0b0100110           /* I/O RP38                       */
#define UART1_TX_Map    0b000001


/**
 * CTMU
 */
#define CTMU_MODE_EDGE  0

#define RANGE_0_55uA    1       /* Base current = 0.55 µA                     */
#define RANGE_5_50uA    2       /* 10x base current = 5.5µA                   */
#define RANGE_55_0uA    3       /* 100x base current = 55µA                   */
#define RANGE_550_uA    0       /* 1000x base current = 550µA                 */

#define RCAL_0_55uA     3.9e6   /* R 3.9M for 0.55µA source                   */
#define RCAL_5_50uA     390e3   /* R 390k for 5.50µA source                   */
#define RCAL_55_0uA     39e3    /* R  39k for 55µA source                     */
#define RCAL_550uA      3.9e3   /* R 3.9k for 550µA source                    */

/**
 * Delay: calculated with C V = I t. 
 *  - C = guess for internal system C (around 40pF)
 *  - V = 70% of ADREF
 *  - I = ideal value for range
 */

#define DELAY_0_55uA    167    /* Delay 167µs for 0.55µA source               */
#define DELAY_5_50uA    17     /* Delay  17µs for 5.50µA source               */
#define DELAY_55_0uA    1      /* Delay   1µ for 55µA source                  */
#define DELAY_550uA     1      /* Delay   1µ for 550µA source                 */

#define PORT_0_55uA     LATBbits.LATB13
#define PORT_5_50uA     LATBbits.LATB12
#define PORT_55_0uA     LATBbits.LATB11
#define PORT_550uA      LATBbits.LATB10

#define ADSCALE         1023    /* 10-bit ADC, 1023                           */
#define ADREF           3.3     /* Vref is 3.3V                               */

#define IIT_CALIBRATE   10      /* Number of iterations  for calibration      */
#define IIT_MAX         500     /* Max number of iterations when measuring C  */
#define DELAY_STEP      20      /* Steps per delay iteration                  */      
#define DELAY_MAX       5e3     /* Max delay for a selected source range      */
#define DELAY_MIN       20      /* Min delay for a selected source range      */
    

#endif	/* XC_HEADER_TEMPLATE_H */

