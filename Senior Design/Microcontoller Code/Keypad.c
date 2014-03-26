/*----------------------------------------------------------------------------
 * Name:   Keypad.c
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

#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include <RTL.h>                                  // RTX kernel functions & defines
#include "keypad.h"
#include "STM32_Init.h"               /* STM32 Initialization                */

/***************************************************/
/*					GLOBALS		 			       */
/**************************8************************/
//GPIO_InitTypeDef* GPIO_InitStruct;	 				/*Define GPIO structure to initialize*/
unsigned short Key_value;
  unsigned char key1=0;
  unsigned char key2=0;
  unsigned char key3=0;
/****************Key_Init ***************************/
/* Initializes the switch input for GPIO2 inputs
 * Inputs: None 
 * Outputs: Key which was pressed in KeyPad	*/
 /**************************************************/

//********* KeyScan ****************
// Scans the switch inputs
// Inputs: None 
// Outputs: Key which was pressed in KeyPad

unsigned short keyScan(void){
  

  Key_value = 0;
 
  key2 = GPIOB->IDR;
    if((key2 & KEY_ALL1) == KEY_4){  //read
      Key_value = 4;
	}else if((key2 & KEY_ALL1) == KEY_5){  //read
      Key_value =  5;
    }
	else if((key2 & KEY_ALL1) == KEY_6){  //read
      Key_value =  6;
    } 
	else{
			key1 = GPIOA->IDR; 
			if((key1 & KEY_ALL2) == KEY_8){
						Key_value =8;
			}
	 		else if((key1 & KEY_ALL2) == KEY_2){  //read
      					Key_value = 2;
			}else if((key1 & KEY_ALL2) == KEY_3){   //read
      					Key_value =  3;
   			}else if((key1 & KEY_ALL2) == KEY_7){  //read
      					Key_value = 7;
    		}else if((key1 & KEY_ALL2) == KEY_9){  //read
     	 				Key_value =  9;
		 	}
			else if((key1 & KEY_ALL2) == KEY_HASH){  //read
		    	  		Key_value =  11;
			}
			else{
			     key3 = GPIOC->IDR;
				if((key3 & KEY_ALL3) == KEY_3){
				 		Key_value =3;
				}
				else if((key3 & KEY_ALL3) == KEY_0){
						Key_value =0;
				}
				else if((key3 & KEY_ALL3) == KEY_1){
						Key_value =1;
				} 
				else if((key3 & KEY_ALL3) == KEY_STAR){  //read
		      			Key_value = 10;    
		    	} 
				else{
						Key_value =100;
				}
			  }

	   }
	
	 return Key_value;
	
     }
  
/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/  
/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/  
