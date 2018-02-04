/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef UTILS_H
#define	UTILS_H

#include <xc.h> // include processor files - each processor file is guarded.  

bool stringEquals(const char * str1, const char * str2);

uint16_t stringToInt(const char * str);

/**
 * Find the minimum value in an array
 * @param values: the array
 * @param length: length of the array
 * @return index of smallest element
 */
uint16_t minimumValue(float *values, uint16_t length);

#endif	/* XC_HEADER_TEMPLATE_H */

