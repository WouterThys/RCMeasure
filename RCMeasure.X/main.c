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
#include <math.h>

#include "Settings.h"

#include "Drivers/INTERRUPT_Driver.h"
#include "Drivers/SYSTEM_Driver.h"
#include "Drivers/UART_Driver.h"
#include "Drivers/CTMU_Driver.h"
#include "utils.h"

/*******************************************************************************
 *          DEFINES
 ******************************************************************************/



/*******************************************************************************
 *          MACRO FUNCTIONS
 ******************************************************************************/

/*******************************************************************************
 *          VARIABLES
 ******************************************************************************/
CData_t capacitanceData; // Data struct for capacitance


/*******************************************************************************
 *          LOCAL FUNCTIONS
 ******************************************************************************/
static void initialize();
static void printCapData(CData_t *cData);

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

void printCapData(CData_t* cData) {
    printf(" -> Cx = %eF\n", cData->Cx);
    printf("   -Ci = %eF\n", cData->Ci);
    printf("   -Range = %d\n", cData->range);
    printf("   -Current = %eA\n", cData->current);
    printf("   -Cur ADC = %d\n", cData->curAdcVal);
    printf("   -Delay = %d\n", cData->delay);
    printf("   -Cap ADC = %d\n", cData->capAdcVal);
    printf("   -Iterations = %d\n", cData->iterations);
    printf("\n\n\n");
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
    
    printf("\n *** Start *** \n");
    
    TRISBbits.TRISB5 = 1; // Input
    uint16_t ledCnt = 0;
    while (1) {
        if (PORTBbits.RB5 == 1) {
            LED1 = 0;
            // Clear all data
            capacitanceData.Ci = -1;
            capacitanceData.Cx = -1;
            capacitanceData.range = 0;
            capacitanceData.current = -1;
            capacitanceData.curAdcVal = 0;
            capacitanceData.delay = 0;
            capacitanceData.capAdcVal = 0;
            capacitanceData.iterations = 0;
            
            // Let the magic happen
            printf("\n** Start C measurement...");
            capacitanceMeasure(&capacitanceData);
            printCapData(&capacitanceData);
        }
        
        // Led and de-bounce 
        ledCnt++;
        if (ledCnt > 20) {
            ledCnt = 0;
            LED1 = !LED1;
        }
        DelayMs(20); 
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


//static void ctmuCurrentIterate(float *results);
//static void ctmuCurrentBestTrim(float *results,  int16_t *bestTrim, float *current);
//static void ctmuCurrentIterate(float *results) {
//    float cntsAvg = 0;
//    float cntsTot = 0;
//    float ctmuISrc = 0;
//    float result = 0;
//    int16_t cal;
//    uint16_t cnt = 0;
//    for (cal = -(TRIMRANGE/2); cal < (TRIMRANGE/2); cal++) {
//        result = 0;
//        cntsAvg = 0;
//        ctmuISrc = 0;
//        cntsTot = 0;
//
//        uint16_t x = 0;
//        for (x = 0; x < ITERATIONS; x++) {
//            result = (ctmuCurrentCalibrate(CTMU_MODE_EDGE, RANGE_0_55uA, cal));
//            cntsTot += result;
//        }
//
//        // Average
//        cntsAvg = (cntsTot / ITERATIONS);
//        
//        *(results + cnt) = cntsAvg;
//        cnt++;
//        DelayMs(1);
//    }
//}
//
//void ctmuCurrentBestTrim(float *results, int16_t *bestTrim, float *current) {
//    float expected = (RCAL * BASE_CURRENT * ADSCALE) / ADREF;
//    
//    float dif[TRIMRANGE];
//    
//    // Find differences from expected counts
//    uint16_t cnt;
//    for (cnt = 0; cnt < TRIMRANGE; cnt++) {
//        float d = (*(results + cnt) - expected);
//        if (d < 0) {
//            dif[cnt] = -1 * d;
//        } else {
//            dif[cnt] = d;
//        }
//    }
//    
//    // Index of minimum difference
//    uint16_t bestNdx = minimumValue(dif, TRIMRANGE);
//    
//    // Results
//    *bestTrim = bestNdx - (TRIMRANGE / 2);
//    *current = (results[bestNdx] * ADREF) / (RCAL * ADSCALE);
//}


