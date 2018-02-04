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



/**
 * Calculate the capacitance and store data in cData
 * @param cData Struct containing the data to find C
 */
void capacitanceMeasure(CData_t *cData);


#ifdef	__cplusplus
}
#endif

#endif	/* CTMU_DRIVER_H */

