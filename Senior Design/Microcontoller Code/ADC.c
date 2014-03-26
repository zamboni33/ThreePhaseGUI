/*----------------------------------------------------------------------------
 * Name:    ADC.c
 * Purpose: Drivers for ADC
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/
 /*----------------------------------------------------------------------------
 * Edited by Upahar Sood
 * Date: 10/28/2010
 * Changes: Implementation of two logically separate displays on a single
 *          two-line LCD
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include "STM32_Init.h"                           // STM32 Initialization
#include "ADC.h"

#define ADC_TEMP_ZERO        1891     // Zero deg C reading for onboard temp sensor

// ADC CR1 options
#define ADC_SCAN        0x00000100     // Scan mode enabled

// ADC CR2 options
#define ADC_ON          0x00000001     // Turn ADC on
#define ADC_RSTCAL      0x00000008     // Reset calibration data
#define ADC_CAL         0x00000004     // Calibration bit
#define ADC_CONT        0x00000002     // Continuous sampling mode
#define ADC_ALIGN_LEFT  0x00000800     // Left align the data
#define ADC_ALIGN_RIGHT 0x00000000     // Default alignment is right-aligned
#define ADC_START       0x00400000     // SWSTART bit, start converstion of regular channels
#define ADC_TSVREFE     0x00800000     // TSVREFE bit controls the temperature sensor power

// ADC status flags
#define ADC_CONV_DONE   0x00000002     // Conversion done flag
#define ADC_WD_FLAG     0x00000001     // Analog watchdog flag

// Macros
#define ADC1_CONVERTING  (!(ADC1->SR & ADC_CONV_DONE))

unsigned long current;
/******************************Public Functions******************/
/*------------------------------------------------------------------------------
 * Initialises the Analog/Digital converter based on ADC_Channel
 * @PA: contains the channel to be initialized
 * Comments: Add PA2, PA3 support
 *			is stm32_init() needed?
 *------------------------------------------------------------------------------*/

void ADC_SoftwareInit()
{
/******************************************************************************/
/*               Initialises the Analog/Digital converter                     */
/*               PA1 (ADC Channel1) is used as analog input                   */
/*               use DMA Channel1 for ADC1 (see DMA request mapping)          */
/******************************************************************************/
 int count;
  RCC->APB2ENR |= (1<<9);                        // Enable periperal clock for ADC1
  ADC1->CR2 &= ~ADC_ON;                          // Turn off the ADC
  count = 50;
  while(--count);                                    // Must delay 2 ADC clocks
  ADC1->CR2 |= ADC_ON;                           // Turn on the ADC
  ADC1->CR2 |= ADC_TSVREFE;                      // Turn the temperature sensor on
  ADC1->SMPR1 = 0x00280000;                      // set sample time channel 16  (55.5 cycles)
  ADC1->SMPR2 = 0x00000B6D;                      // set sample time channel 0-3 (55.5 cycles)
  count = 50;
  while(--count);                                    // Must delay before conversion starts
  ADC1->CR2 |= ADC_RSTCAL;                       // Reset calibration registers
  ADC1->CR2 |= ADC_CAL;                          // Calibrate the ADC
  while(ADC1->CR2 & ADC_CAL);                    // Wait for the calibration to complete
}		


/*----------------------------------------------------------------------------
 * Retrieves the value now at AD_Channel 
 * @PA: contains the channel to be initialized
 * Comments: returns 0 if not initialized
 *----------------------------------------------------------------------------*/
unsigned long ADC_getValue(int PA)
{
 ADC1->SMPR1 = 0x00140000;                      // set sample time channel 16  (55.5 cycles)
  ADC1->SMPR2 = 0x00000B6D;                      // set sample time channel 0-3 (55.5 cycles)
  ADC1->CR1 &= ~ADC_SCAN;                        // Disable scan mode
  ADC1->SQR1 = 0x00000000;                       // Only one conversion, so
  ADC1->SQR2 = 0x00000000;                       //  clear all other scan settings
  ADC1->SQR3 = (unsigned int) PA;
  ADC1->CR2 |= ADC_ON;                           // Start the conversion
  while(ADC1_CONVERTING);                        // Wait for conversion to complete
  return ADC1->DR;                          // Return the result of the conversion
 }
