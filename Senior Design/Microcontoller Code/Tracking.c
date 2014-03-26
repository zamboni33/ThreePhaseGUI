/*----------------------------------------------------------------------------
 * Name:    Tracking.c
 * Purpose: Functions for the USART TX and RX Serial Routines
 *                on MCBSTR9 evaluation board
 *
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stm32f10x_lib.h>            /* STM32F10x Library Definitions       */
#include "STM32_Init.h"               /* STM32 Initialization                */
#include <RTL.h>                      /* RTX kernel functions & defines      */
#include "Tracking.h"

#define PHOTO_TOLERANCE 0.02
#define POWER_TOLERANCE	0.05
#define MICRO_SEC		0.000001
#define NUM_SPOKE		4
#define SEC_PER_MIN		60


static long float power_old = 0;
static int frequency = MIN_FREQUENCY;
static long float voltage = 0;
static long float current = 0;
static int RPM = 0;

char vpoints[16];
char fpoints[16];
char vlipoints[16];
char lpoints[16];

void set_VFTable(char vinput[16], char finput[16]){

int i;
for(i=0;i<16;i++){
vpoints[i] = vinput[i];
fpoints[i] = finput[i];
}
frequency = fpoints[0];
}
void set_VLITable(char vinput[16], char linput[16]){
int i;
for(i=0;i<16;i++){
vlipoints[i] = vinput[i];
lpoints[i] = linput[i];
}

}
void voltage_calc(unsigned short equation) {
//use the equation to calculate the voltage


}

void voltage_calctable() {
//use the equation to calculate the voltage
   int i;
   for(i=0;i<16;i++){
   if(RPM<fpoints[0]){
   voltage = vpoints[0] + (vpoints[1]-vpoints[0])/(fpoints[1]-fpoints[0])*( RPM - fpoints[0]);
   return;
   }
   else if(RPM<fpoints[i]){
   voltage = vpoints[i] + (vpoints[i-1]-vpoints[i])/(fpoints[i-1]-fpoints[i])*( RPM - fpoints[i]);
   return;
   }
   else if(RPM>fpoints[15]){
   voltage = vpoints[15] + (vpoints[15]-vpoints[14])/(fpoints[15]-fpoints[14])*( RPM - fpoints[15]);
   return;
   }
   }

}

int frequency_calctable(long float voltage_photo) {
//use the equation to calculate the frequency
   int i, new_frequency;
   for(i=0;i<16;i++){
   if(voltage_photo<vpoints[0]){
   new_frequency = fpoints[0] + (fpoints[1]-fpoints[0])/(vpoints[1]-vpoints[0])*( voltage_photo - vpoints[0]);
   return new_frequency;
   }
   else if(voltage_photo<vpoints[i]){
   new_frequency = fpoints[i] + (fpoints[i-1]-fpoints[i])/(vpoints[i-1]-vpoints[i])*( voltage_photo - vpoints[i]);
   return new_frequency;
   }
   else if(voltage_photo>vpoints[15]){
   return fpoints[15];
   }
   }
   return 0;
}
long float voltage_calc_light_intensity(unsigned short photo_value) {
//use the equation to calculate the voltage of the solar  panels
   int i;
   long float voltage_photo;
   for(i=0;i<16;i++){
   if(photo_value<lpoints[0]){
   voltage_photo = vlipoints[0] + (vlipoints[1]-vlipoints[0])/(lpoints[1]-lpoints[0])*( RPM - lpoints[0]);
   return voltage_photo;
   }
   else if(photo_value<lpoints[i]){
   voltage_photo = vlipoints[i] + (vlipoints[i-1]-vlipoints[i])/(lpoints[i-1]-lpoints[i])*( RPM - lpoints[i]);
   return voltage_photo;
   }
   else if(photo_value>lpoints[15]){
   voltage_photo = vlipoints[15] + (vlipoints[15]-vlipoints[14])/(lpoints[15]-lpoints[14])*( RPM - lpoints[15]);
   return voltage_photo;
   }
   }
   return 0;
}

void RPM_calc(unsigned short current_frequency){
//Tachometer speed calculation
 RPM = (int)(SEC_PER_MIN/(current_frequency*MICRO_SEC*NUM_SPOKE));
}

void current_calc(){
	current = 0; // Need to add code after developing Current Sensor Circuit
}

void photo_update(unsigned short AD_photo_new, unsigned short AD_photo_old){
	  int new_frequency;
	  long float voltage_photo;
	  if(AD_photo_old*(1+PHOTO_TOLERANCE)>AD_photo_new){
		voltage_photo = voltage_calc_light_intensity(AD_photo_new);	//Determines new voltage input
	  	new_frequency = frequency_calctable(voltage_photo);
	  	while(frequency != new_frequency){
	  			 frequency--;
				 TIM1->ARR = frequency;
				 os_dly_wait(50);
	  	}
	  }
	  else if(AD_photo_old*(1-PHOTO_TOLERANCE)<AD_photo_new){
		  voltage_photo = voltage_calc_light_intensity(AD_photo_new);	//Determines new voltage input
		  new_frequency = frequency_calctable(voltage_photo);
		  while(frequency != new_frequency){
		  		 frequency++;
				 TIM1->ARR = frequency;
				 os_dly_wait(50);
		  }
	  }


}



void power_calculate(unsigned short current_frequency){
	long float power;
	RPM_calc(current_frequency);
	voltage_calctable();
	power = (voltage) * (current) * (RPM / 60);
	if( power == power_old){}
	else if( power > power_old){
		frequency++;

	}
	else if( power < power_old){
		frequency--;
	}
	TIM1->ARR = frequency;
}
/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

