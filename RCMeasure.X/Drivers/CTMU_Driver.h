#ifndef CTMU_DRIVER_H
#define	CTMU_DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct CData {
    double   Cx;         // Searched capacitance
    double   Ci;         // Internal capacitance
    uint16_t range;      // Range for which the capacitance was found
    double   current;    // Actual current for range
    uint16_t curAdcVal;  // ADC value used to calculate actual current
    uint16_t delay;      // Delay for which the capacitance was found
    uint16_t capAdcVal;  // ADC value used to calculate capacitance
    uint16_t iterations; // Iterations needed to find delay
} CData_t;

typedef struct RData {
    double   Rx;         // Searched resistance
    uint16_t range;      // Range for which the resistance was found
    double   current;    // Actual current for range
    uint16_t curAdcVal;  // ADC value used to calculate actual current
    double   Rrange[4];  // Resistance for each range
} RData_t;

/**
 * Calculate the capacitance and store data in cData
 * @param cData Struct containing the data to find C
 */
void capacitanceMeasure(CData_t *cData);

/**
 * Calculate the resistance and store data in rData
 * @param rData Struct containing the data to find R
 */
void resistanceMeasure(RData_t *rData);


#ifdef	__cplusplus
}
#endif

#endif	/* CTMU_DRIVER_H */

