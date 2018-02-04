#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <string.h>

#include "../Settings.h"
#include "SYSTEM_Driver.h"
#include "CTMU_Driver.h"

/*******************************************************************************
 *          DEFINES
 ******************************************************************************/
#define RESULT_UF -1    /* Underflow                                          */
#define RESULT_OK  0    /* Ok                                                 */
#define RESULT_OF  1    /* Overflow                                           */


/*******************************************************************************
 *          LOCAL FUNCTIONS
 ******************************************************************************/
static void     ctmuConfigure(uint16_t mode, uint16_t range, int16_t trim, uint16_t ground);
static void     ctmuCalculateCapacitance(uint16_t counts, uint16_t delay, double I, double *C);
static int16_t  ctmuCapacitanceLogic(CData_t *capacitance);
static double   ctmuCountsToCurrent(uint16_t counts, uint16_t range);
static double   ctmuCountsToVoltage(uint16_t counts);
static uint16_t ctmuOptimalDelay(uint16_t range);
static void     ctmuSelectRange(uint16_t range);

// Current
static uint16_t ctmuCurrentMeasure(uint16_t mode, uint16_t range, int16_t trim);
static void     ctmuCurrentIterate(uint16_t range, uint16_t *counts, double *current);
// Capacitance
static uint16_t ctmuCapacitanceMeasure(uint16_t mode, uint16_t range, uint16_t delay, bool calibration);
static void     ctmuCapacitanceIterate(uint16_t range, uint16_t delay, double current, bool calibration, uint16_t *counts);


/*******************************************************************************
 *          LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * Configure the CTMU
 * @param mode Enable/Disable Time Generation Mode
 * @param range Current source range
 * @param trim Current source trim
 * @param ground Ground the current source or not
 */
void ctmuConfigure(uint16_t mode, uint16_t range, int16_t trim, uint16_t ground) {
    // Clear before start
    CTMUCON1 = 0x0000;
    CTMUCON2 = 0x0000;
    CTMUICON = 0x0000;
    
    AD1CON1 = 0x0000;
    AD1CON2 = 0x0000;
    AD1CON3 = 0x0000;
    
    CTMUCON1bits.CTMUEN = 0;    // Disable CTMU
    CTMUCON1bits.TGEN = mode;   // Enable/Disable Time Generation Mode
    CTMUCON1bits.EDGEN = 0;     // Edges are disabled
    CTMUCON1bits.IDISSEN = ground; // Current source grounded or not
    CTMUCON1bits.CTTRIG = 0;    // Trigger output disabled
    
    CTMUICONbits.IRNG = (range & 0x0003); //Set range
    CTMUICONbits.ITRIM = (trim & 0x003F); //Trim
}

/**
 * Calculate the capacitance by 
 * @param counts ADC measurement value
 * @param delay Delay
 * @param I Current
 * @param C Capacitance
 */
void ctmuCalculateCapacitance(uint16_t counts, uint16_t delay, double I, double *C) {
    double t = delay * 1e-6;
    double V = ctmuCountsToVoltage(counts);
    *C = (I * t) / V;
}

/**
 * Convert measured ADC value to current
 * @param range Selected current source range
 * @param counts
 * @return current in A
 */
double ctmuCountsToCurrent(uint16_t counts, uint16_t range) {
    double r = 1;
    switch (range) {
        default:
        case RANGE_0_55uA: r = RCAL_0_55uA; break;
        case RANGE_5_50uA: r = RCAL_5_50uA; break;
        case RANGE_55_0uA: r = RCAL_55_0uA; break;
        case RANGE_550_uA: r = RCAL_550uA; break;
    }
    return ctmuCountsToVoltage(counts) / r;
}

/**
 * Convert measured ADC value to voltage
 * @param counts
 * @return voltage in V
 */
double ctmuCountsToVoltage(uint16_t counts) {
    return (counts * ADREF) / ADSCALE;
}

/**
 * Calculate the optimal delay for a given current mode
 * @param range Selected current source range
 * @return Optimal delay in µs for selected range
 */
uint16_t ctmuOptimalDelay(uint16_t range) {
    uint16_t d = 1;
    switch (range) {
        default:
        case RANGE_0_55uA: 
            d = DELAY_0_55uA; 
            break;
        case RANGE_5_50uA: 
            d = DELAY_5_50uA; 
            break;
        case RANGE_55_0uA: 
            d = DELAY_55_0uA; 
            break;
        case RANGE_550_uA: 
            d = DELAY_550uA; 
            break;
    }
    return d;
}

/**
 * Select the MOSFET to set the resistor for calibrating current
 * @param range Selected current source range
 */
static void ctmuSelectRange(uint16_t range) {
    PORT_0_55uA = 0;
    PORT_5_50uA = 0;
    PORT_55_0uA = 0;
    PORT_550uA  = 0;
    
    switch (range) {
        case RANGE_0_55uA: 
            PORT_0_55uA = 1;
            PORT_5_50uA = 0;
            PORT_55_0uA = 0;
            PORT_550uA  = 0;
            break;
        case RANGE_5_50uA: 
            PORT_0_55uA = 0;
            PORT_5_50uA = 1;
            PORT_55_0uA = 0;
            PORT_550uA  = 0;
            break;
        case RANGE_55_0uA: 
            PORT_0_55uA = 0;
            PORT_5_50uA = 0;
            PORT_55_0uA = 1;
            PORT_550uA  = 0;
            break;
        case RANGE_550_uA: 
            PORT_0_55uA = 0;
            PORT_5_50uA = 0;
            PORT_55_0uA = 0;
            PORT_550uA  = 1;
            break;
    }
}

/**
 * Measures the internal current source
 * @param mode Enable/Disable Time Generation Mode
 * @param range Select the current source range, I_BASE, 10x I_BASE or 100x I_BASE
 * @param trim Trim value for the current source
 * @return ADC value measured 
 */
uint16_t ctmuCurrentMeasure(uint16_t mode, uint16_t range, int16_t trim) {
    // Step 1: Configure the CTMU
    ctmuConfigure(mode, range, trim, 0); // Current source is not grounded
    
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

/**
 * Iterates ITERATIONS times current measurements
 * @param range Current source range
 * @param counts ADC measurement value
 * @param current Current calculated from ADC measurement
 */
void ctmuCurrentIterate(uint16_t range, uint16_t *counts, double *current) {
    double cntsTot = 0;

    // Iterate
    uint16_t x = 0;
    for (x = 0; x < IIT_CALIBRATE; x++) {
        cntsTot += (double)(ctmuCurrentMeasure(CTMU_MODE_EDGE, range, 0));
    }

    // Results
    *counts = (cntsTot / IIT_CALIBRATE);
    *current = ctmuCountsToCurrent(*counts, range);
}


/**
 * Measures the internal capacitance
 * @param mode Enable/Disable Time Generation Mode
 * @param range Select the current source range, I_BASE, 10x I_BASE or 100x I_BASE
 * @param delay Delay before ADC starts sampling
 * @param calibration Is this a calibration or measurement (selects different AD port)
 * @return ADC value measured
 */
uint16_t ctmuCapacitanceMeasure(uint16_t mode, uint16_t range, uint16_t delay, bool calibration) {
    // Step 1: Configure the CTMU
    ctmuConfigure(mode, range, 0, 1); // Current source not grounded
    
    // Step 2 & 3: Configure GPIO and ADC
    if (calibration) {
        TRISAbits.TRISA1 = 1;       // AN1 is input for ADC 
        ANSELAbits.ANSA1 = 1;       // AN1 is analog
        AD1CHS0bits.CH0SA = 1;      // Channel 0 positive input is AN1
    } else {
        TRISBbits.TRISB0 = 1;       // AN2 is input for ADC 
        ANSELBbits.ANSB0 = 1;       // AN2 is analog
        AD1CHS0bits.CH0SA = 2;      // Channel 0 positive input is AN1
    }
    
    AD1CON1bits.ADON = 1;       // AD is on
    AD1CON1bits.AD12B = 0;      // 10-bit
    AD1CON1bits.FORM = 0b00;    // Integer output
    
    // Step 4-6: Enable the current source an start sampling
    CTMUCON1bits.CTMUEN = 1;    // Enable the CTMU
    CTMUCON2bits.EDG1STAT = 1;  // Enable current source
    AD1CON1bits.SAMP = 1;       // Manual sampling start
    
    // Step 7: Delay to discharge sample capacitance
    DelayMs(2);
    
    // Step 8: Disable discharge
    CTMUCON1bits.IDISSEN = 0;   // Discharge disabled
    
    // Step 9: Delay to charge sample cap
    DelayUs(delay);
    
    // Step 10: Convert the sample
    AD1CON1bits.SAMP = 0;       // Begin AD conversion
    while (AD1CON1bits.DONE == 0);
    
    // Step 11: Disable the CTMU
    CTMUCON2bits.EDG1STAT = 0;  // Disable current source
    IFS0bits.AD1IF = 0;         // Clear interrupt flag
    CTMUCON1bits.CTMUEN = 0;    // Disable CTMU
    
    // Clean up
    if (calibration) {
        TRISAbits.TRISA1 = 0;   // AN1 is output 
        ANSELAbits.ANSA1 = 0;   // AN1 is digital
    } else {
        TRISBbits.TRISB0 = 0;   // AN2 is output
        ANSELBbits.ANSB0 = 0;   // AN2 is digital
    }
    
    // Result
    return ADC1BUF0;
}

/**
 * Iterates ITERATIONS times capacitance measurements
 * @param range Current source range
 * @param delay Delay before ADC starts sampling
 * @param current Actual current
 * @param calibration Is this a calibration or not
 * @param counts ADC measurement value, should be around 70% of ADREF
 */
void ctmuCapacitanceIterate(uint16_t range, uint16_t delay, double current, bool calibration, uint16_t *counts) {
    double cntsTot = 0;
    
    // Iterate
    uint16_t x; 
    for (x = 0; x < IIT_CALIBRATE; x++) {
        cntsTot += (double)(ctmuCapacitanceMeasure(CTMU_MODE_EDGE, range, delay, calibration));
    }
    
    // Result is average
    *counts = cntsTot / IIT_CALIBRATE;
}

/**
 * Calculates the capacitance for a given current source range
 *  Step 1: Calculate actual current
 *  Step 2: Calculate system capacitance
 *  Step 3: Start iterating the input chanel until accurate result is found
 *  Step 4: Calculate the capacitance if accurate value is found
 * @param range Current source range
 * @param capacitance
 * @return OK, UF (underflow) or OF (overflow)
 */
int16_t ctmuCapacitanceLogic(CData_t *cData) {
    uint16_t range = cData->range;
    double current = 0;
    double searchedCap = 0;
    double systemCap = 0;
    int16_t result = 0;
    uint16_t delay = 0;
    uint16_t counts = 0;
    uint16_t idealCnt = ADSCALE * 0.7;
    uint16_t maxCnt = ADSCALE * 0.75;
    uint16_t minCnt = ADSCALE * 0.65;
    
    if (DEBUG) printf("*R=%d (I, D, C)\n", range);
    
    // Step 0: Select resistor
    ctmuSelectRange(range);
    
    // Step 1: Calculate actual current
    ctmuCurrentIterate(range, &counts, &current);
    cData->curAdcVal = counts;
    cData->current = current;
   
    // Step 2: Calculate system capacitance
    ctmuCapacitanceIterate(range, ctmuOptimalDelay(range), current, true, &counts);
    
    // If counts > 75% of ADSCALE, assume clipping and don't include system capacitance
    if (counts < maxCnt) {
        ctmuCalculateCapacitance(counts, delay, current, &systemCap);
        cData->Cx = systemCap;
    }
    
    // Step 3: Start iterating the input chanel until accurate result is found
    counts = 0;
    delay = 300;
    uint16_t iterations = 0;
    while ((iterations < IIT_MAX) && (delay >= DELAY_MIN && delay <= DELAY_MAX) && (counts < minCnt || counts > maxCnt)) {
        ctmuCapacitanceIterate(range, delay, current, false, &counts);
        
        if (DEBUG) printf("%d,%d,%d\n", iterations, delay, counts);
        
        // Calculated guess for correct delay
        delay = (((double)delay / (double)counts) * (double)idealCnt);
        iterations++;
        
        LED2 = !LED2;
    }
    
    // Step 4: Calculate the capacitance if accurate value is found
    if ((iterations < IIT_MAX) && (delay >= DELAY_MIN && delay <= DELAY_MAX)) {
        
        ctmuCalculateCapacitance(counts, delay, current, &searchedCap);
        cData->Cx = searchedCap;
        cData->capAdcVal = counts;
        cData->delay = delay;
        cData->iterations = iterations;
        result = RESULT_OK;
        
    } else {
        if (counts < minCnt || delay >= DELAY_MAX) {
            result = RESULT_UF; // Underflow
        }
        if (counts > maxCnt || delay <= DELAY_MIN) {
            result = RESULT_OF; // Overflow
        }
    }
    
    return result;
}


/*******************************************************************************
 *          DRIVER FUNCTIONS
 ******************************************************************************/

uint16_t ranges[4] = {RANGE_0_55uA, RANGE_5_50uA, RANGE_55_0uA, RANGE_550_uA};
void capacitanceMeasure(CData_t *cData) {
    uint16_t r;
    for (r = 0; r < 4; r++) {
        cData->range = ranges[r];
        if (ctmuCapacitanceLogic(cData) == RESULT_OK) {
            return; // Ready!
        }
    }
    
    cData->Cx = -1;
}



