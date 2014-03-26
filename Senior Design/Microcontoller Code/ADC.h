/*----------------------------------------------------------------------------
 * Name:    ADC.H
 * Purpose: Implementation of ADC ports 1 -3
 * Edited by Upahar Sood
 * Date: 11/1/2010 1 AM
 * Notes:
 *---------------------------------------------------------------------------*/

#ifndef _ADC_H_
#define _ADC_H_

 typedef enum {PC0 = 0, PC1 = 1, PC2 = 2, PC3 = 3, PC4 = 4, PC5 = 5, PC6 = 6} ADC_Channel;
 /*------------------------------------------------------------------------------
 * Initialises the Analog/Digital converter based on ADC_Channel
 * @PA: contains the channel to be initialized
 *------------------------------------------------------------------------------*/
extern void ADC_SoftwareInit (void);
/*----------------------------------------------------------------------------
 * Retrieves the value now at AD_Channel 
 * @PA: contains the channel to be initialized
 * Comments: returns 0 if not initialized
 *----------------------------------------------------------------------------*/
extern unsigned long ADC_getValue (int chan);
#endif
