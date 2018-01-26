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
#define RANGE_550uA     3       // 100x base current of 0.55 µA
#define RCAL            10e3    // R is 10k
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
static void ctmuCurrentIterate(float *results);

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
    
    CTMUICONbits.IRNG = (range & 0x0003); //Set range
    CTMUICONbits.ITRIM = (trim & 0x003F); //Trim
    
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
    DelayMs(2);
    
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

static void ctmuCurrentIterate(float *results) {
    float cntsAvg = 0;
    float cntsTot = 0;
    float ctmuISrc = 0;
    float result = 0;
    int16_t cal;
    uint16_t cnt = 0;
    for (cal = -30; cal <= 30; cal++) {
        result = 0;
        cntsAvg = 0;
        ctmuISrc = 0;
        cntsTot = 0;

        uint16_t x = 0;
        for (x = 0; x < ITERATIONS; x++) {
            result = (ctmuCurrentCalibrate(CTMU_MODE_EDGE, RANGE_550uA, cal));
            cntsTot += result;
        }

        // Average
        cntsAvg = (cntsTot / ITERATIONS);
        
        *(results + cnt) = cntsAvg;
        cnt++;
        DelayMs(10);
        //vCal = (cntsAvg / ADSCALE * ADREF);
        //ctmuISrc = (vCal / RCAL) * 1000000; // Current in µA
    }
}

/*******************************************************************************
 *          MAIN PROGRAM
 ******************************************************************************/

int main(void) {

    initialize();
    
    // Initialize UART
    D_UART_Init(UART_MODULE_1, UART1_BAUD);
    D_UART_Enable(UART_MODULE_1, true);
   
    DelayMs(100);
    
    float results[60];
    float *result;
    
    result = results;
   
    ctmuCurrentIterate(result);    
    
    uint16_t cnt = 0;
    for (cnt = 0; cnt < 60; cnt++) {
        printf("%d: %f\n", cnt, *(result+cnt));
        DelayMs(1);
    }
        
    
    
    while (1) {
        LED1 = !LED1;
        DelayMs(1000); 
    }
    return 0;
}


/***
 *  Current calibration test results
 *  --------------------------------
 * 
 * RCAL = 10k
 * ADC = 10-bit
 * CTMU = 55µA (base range x100)
 *
 * RESULT: best value for trim = -1 => 172 counts ~ 55.48µA
   
 
#define CTMU_MODE_EDGE  0
#define RANGE_550uA     3       // 100x base current of 0.55 µA
#define RCAL            10e3    // R is 10k
#define ADSCALE         1023    // 10-bit ADC
#define ADREF           3.3     // Vref is 3.3V

#define ITERATIONS      10      // Number of iterations
 
0: 69.400002
1: 71.000000
2: 71.000000
3: 79.000000
4: 96.000000
5: 96.000000
6: 96.000000
7: 96.000000
8: 96.599998
9: 99.900002
10: 103.000000
11: 112.000000
12: 112.000000
13: 114.400002
14: 119.300003
15: 121.699997
16: 125.800003
17: 127.000000
18: 132.800003
19: 135.100006
20: 140.000000
21: 143.000000
22: 146.600006
23: 150.300003
24: 153.199997
25: 157.899994
26: 159.000000
27: 159.000000
28: 167.000000
29: 172.000000
30: 176.000000
31: 178.899994
32: 192.000000
33: 192.000000
34: 192.000000
35: 192.199997
36: 195.000000
37: 199.000000
38: 199.000000
39: 207.000000
40: 215.500000
41: 224.000000
42: 224.000000
43: 224.000000
44: 224.100006
45: 227.100006
46: 231.000000
47: 240.000000
48: 240.000000
49: 242.500000
50: 246.500000
51: 249.500000
52: 253.199997
53: 255.000000
54: 255.000000
55: 263.299988
56: 268.000000
57: 270.899994
58: 274.200012
59: 278.899994
 */


