#if defined(__XC16__)
#include <xc.h>
#elif defined(__C30__)
#if defined(__dsPIC33E__)
#include <p33Exxxx.h>
#elif defined(__dsPIC33F__)
#include <p33Fxxxx.h>
#endif
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <string.h>

#include "Settings.h"

#include "Drivers/INTERRUPT_Driver.h"
#include "Drivers/SYSTEM_Driver.h"
#include "Drivers/UART_Driver.h"

/*******************************************************************************
 *          DEFINES
 ******************************************************************************/
#define CTMU_MODE_EDGE  0
#define RANGE_6_5uA     2       // 10x base current of 0.65 µA
#define RCAL            429e3   // R is 429k
#define ADSCALE         1023    // 10-bit ADC
#define ADREF           3.3     // Vref is 3.3V

#define ITERATIONS      10      // Number of iterations


/*******************************************************************************
 *          MACRO FUNCTIONS
 ******************************************************************************/

/*******************************************************************************
 *          VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *          LOCAL FUNCTIONS
 ******************************************************************************/
static void initialize();
static uint16_t ctmuCurrentCalibrate(uint16_t mode, uint16_t range, int16_t trim);

void initialize() {
    D_INT_EnableInterrupts(false);

    // Initialize system
    D_SYS_InitPll();
    D_SYS_InitOscillator();
    D_SYS_InitPorts();

    // Interrupts
    D_INT_Init();
    D_INT_EnableInterrupts(true);
}

uint16_t ctmuCurrentCalibrate(uint16_t mode, uint16_t range, int16_t trim) {
    
    // Clear before start
    CTMUCON1 = 0x0000;
    CTMUCON2 = 0x0000;
    CTMUICON = 0x0000;
    
    AD1CON1 = 0x0000;
    AD1CON2 = 0x0000;
    AD1CON3 = 0x0000;
    
    // Step 1: Configure the CTMU
    CTMUCON1bits.CTMUEN = 0;    // Disable CTMU
    CTMUCON1bits.TGEN = mode;   // Enable/Disable Time Generation Mode
    CTMUCON1bits.EDGEN = 0;     // Edges are disabled
    CTMUCON1bits.IDISSEN = 0;   // Current source is not grounded
    CTMUCON1bits.CTTRIG = 0;    // Trigger output disabled
    
    CTMUICONbits.IRNG = (range & 3); //Set range
    
    // Step 2: Configure GPIO
    TRISAbits.TRISA0 = 1;       // AN0 is input for ADC 
    ANSELAbits.ANSA0 = 1;       // AN0 is analog
    
    // Step 3: Configure ADC
    AD1CHS0bits.CH0SA = 0;      // Channel 0 positive input is AN0
    
    AD1CON1bits.ADON = 1;       // AD is on
    AD1CON1bits.AD12B = 0;      // 10-bit
    AD1CON1bits.FORM = 0b00;    // Integer output
    
    // Step 4-6: Enable the current source an start sampling
    CTMUCON1bits.CTMUEN = 1;    // Enable the CTMU
    CTMUCON2bits.EDG1STAT = 1;  // Enable current source
    AD1CON1bits.SAMP = 1;       // Manual sampling start
    
    // Step 7: Wait for sample
    DelayMs(3);
    
    // Step 8: Convert the sample
    AD1CON1bits.SAMP = 0;       // Begin AD conversion
    while (AD1CON1bits.DONE == 0);
    
    // Step 9: Disable the CTMU
    CTMUCON2bits.EDG1STAT = 0;  // Disable current source
    IFS0bits.AD1IF = 0;         // Clear interrupt flag
    CTMUCON1bits.CTMUEN = 0;    // Disable CTMU
    
    
    // Clean up
    TRISAbits.TRISA0 = 0;       // AN0 is output 
    ANSELAbits.ANSA0 = 0;       // AN0 is digital
    
    // Result
    return ADC1BUF0;
}

/*******************************************************************************
 *          MAIN PROGRAM
 ******************************************************************************/

int main(void) {

    initialize();
    
    // Initialize UART
    D_UART_Init(UART_MODULE_1, UART1_BAUD);
    D_UART_Enable(UART_MODULE_1, true);
   
    DelayMs(1000);
    
    float cntsAvg = 0;
    float vCal = 0;
    float cntsTot = 0;
    float ctmuISrc = 0;
    float result = 0;
    
    uint16_t x = 0;
    for (x = 0; x < ITERATIONS; x++) {
        result = (float) (ctmuCurrentCalibrate(CTMU_MODE_EDGE, RANGE_6_5uA, 0));
        cntsTot += (float) result;
    }
    
    // Average
    cntsAvg = (cntsTot / ITERATIONS);
    vCal = (cntsAvg / ADSCALE * ADREF);
    ctmuISrc = vCal / RCAL; // Current in µA
    
    // TODO: iterate to calibrate

    while (1) {

        LED1 = !LED1;
        DelayMs(500);

    }
    return 0;
}