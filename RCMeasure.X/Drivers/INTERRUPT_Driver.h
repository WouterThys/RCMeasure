/* 
 * File:   interrupts.h
 * Author: wouter
 *
 * Created on 3 maart 2015, 13:06
 */

#ifndef INT_DRIVER_H
#define	INT_DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Initializes the interrupts to the default settings.
 */
void D_INT_Init(void);
/**
 * Enable interrupts.
 */
void D_INT_EnableInterrupts(bool enable);

/**
 * Enables interrupts for I2C messaging.
 * @param enable
 */
void D_INT_EnableI2CInterrupts(bool enable);


#ifdef	__cplusplus
}
#endif

#endif	