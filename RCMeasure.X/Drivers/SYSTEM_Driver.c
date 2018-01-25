#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "SYSTEM_Driver.h"
/*******************************************************************************
 *          DEFINES
 ******************************************************************************/

/*******************************************************************************
 *          MACRO FUNCTIONS
 ******************************************************************************/

/*******************************************************************************
 *          VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *          DRIVER FUNCTIONS
 ******************************************************************************/
void D_SYS_InitOscillator(void) {
    /* Disable Watch Dog Timer */
    RCONbits.SWDTEN = 0;

    OSCCONbits.NOSC = 0b001; // Fast RC Oscillator (FRC) with divide-by-N and PLL (FRCPLL)
    OSCCONbits.CLKLOCK = 0; //bit 7 CLKLOCK: Clock Lock Enable bit
    OSCCONbits.IOLOCK = 0; //bit 6 IOLOCK: I/O Lock Enable bit
    OSCCONbits.OSWEN = 1; //bit 0 OSWEN: Oscillator Switch Enable bit
    
    while (OSCCONbits.COSC != 0b001);

    /* Reference clock on OSC2*/
    REFOCONbits.ROON = 1;   // Enable clock on output
    REFOCONbits.ROSEL = 0;  // System clock is used as the reference clock
    REFOCONbits.RODIV = 0b0000; // Reference clock not divided
}

void D_SYS_InitPll(void) {
    CLKDIVbits.ROI = 0; //bit 15 ROI: Recover on Interrupt bit
    CLKDIVbits.DOZE0 = 0;
    CLKDIVbits.DOZE1 = 0;
    CLKDIVbits.DOZE2 = 0; //bit 14-12 DOZE<2:0>: Processor Clock Reduction Select bits
    CLKDIVbits.DOZEN = 0; //bit 11 DOZEN: Doze Mode Enable bit
    CLKDIVbits.FRCDIV0 = 0;
    CLKDIVbits.FRCDIV1 = 0;
    CLKDIVbits.FRCDIV2 = 0; //bit 10-8 FRCDIV<2:0>: Internal Fast RC Oscillator Post-scaler bits
    CLKDIVbits.PLLPOST0 = 0;
    CLKDIVbits.PLLPOST1 = 0; //bit 7-6 PLLPOST<1:0>: PLL VCO Output Divider Select bits (also denoted as ?N2?, PLL post-scaler)
    CLKDIVbits.PLLPRE0 = 0;
    CLKDIVbits.PLLPRE1 = 0;
    CLKDIVbits.PLLPRE2 = 0;
    CLKDIVbits.PLLPRE3 = 0;
    CLKDIVbits.PLLPRE4 = 0;
    PLLFBDbits.PLLDIV = 74;
}

void D_SYS_InitPorts(void) {
    TRISA = 0x0000;     // All outputs
    ANSELA = 0x0000;    // All digital
    PORTA = 0x0000;     // All '0'

    TRISB = 0x0000;
    ANSELB = 0x0000;
    PORTB = 0x0000;
}
