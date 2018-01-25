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

#endif	/* XC_HEADER_TEMPLATE_H */

