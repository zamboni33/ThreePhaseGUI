/*----------------------------------------------------------------------------
 * Name:    main.c
 * Purpose: Main File of the Solar Powered motor drive code
 *
 *----------------------------------------------------------------------------
 * Edited by Upahar Sood from the RTX_TRAFFIC PROJECT
 *
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <stdio.h>                    /* standard I/O .h-file                */
#include <ctype.h>                    /* character functions                 */
#include <string.h>                   /* string and memory functions         */
#include <RTL.h>                      /* RTX kernel functions & defines      */
#include <stm32f10x_lib.h>            /* STM32F10x Library Definitions       */
#include "STM32_Init.h"               /* STM32 Initialization                */
#include "LCD.h"                      /* LCD functions & defines             */
#include "KeyPad.h"                   /* Keypad functions & defines       */
#include "Tracking.h"          		  /*Tracking functions & defines      */
#include "ADC.h"
#include "PWM.h"
/***************************************************/
/*					Serial Menu		 			   */
/**************************8************************/
const char menu[] = 
   "\n"
   "+*******************PWM MOTOR CONTROLLER *********************+\n"
   "| This is the serial testing interface for SPMD.              |\n"
   "| You can enter motor characteristics, display measurements   |\n"
   "|                                                             |\n"
   "+ command -+      syntax -----+ function ---------------------+\n"
   "| Use LCD/Keypad  | L			| No longer use serial input  |\n"
   "| Display         | V           | View characteristics        |\n"
   "| Enter V/F point | A           | Enter Voltage/Frequency data|\n"
   "| Enter Il/V      | B           | Enter solar panel data      |\n"
   "| Enter Equation  | E           | Voltage/Frequency equation  |\n"
   "| Clear V/F data  | C           | Clear Voltage/frequency data|\n"
   "| Clear Il/V data | D           | Clear solar panel data      |\n"
   "+----------+-------------+------------------------------------+\n";

/*********************************
	External Functions required **
**********************************/
extern void getline (char *, int);
extern void serial_init (void);     
extern int GetKey (void);            /* external function:  input character */         
/***************************************************/
/*					Task IDS		 			   */
/**************************8************************/
OS_TID t_command;                     /* assigned task id of task: command   */
OS_TID t_getesc;					/* assigned task id of task: get_esc   */
OS_TID t_lcd;                         /* assigned task id of task: lcd       */
OS_TID t_tach;                        /*assigned task id of tachometer      */  
OS_TID t_photo;					      /*assigned task id of photo sensor  */
OS_TID t_KeyPad;					 /*assigned task id of keypad  */
OS_TID t_motor;                     /* assigned task id of task: keyread   */
OS_TID t_tracker;                     /* assigned task id of task: keyread   */
OS_TID t_tach;
OS_TID t_lcd2;                
/***************************************************/
/*					GLOBALS		 			   */
/**************************8************************/
volatile unsigned short LCD_menu=0;
short frequency;
char PWM_state = 0;
short AD_tach =330;                     /* Last AD value read in interrupt  */
short AD_photo;
short AD_photo_old;
short tach = 0;
char cmdline[16];                     /* storage for command input line      */
char vinput[16];
char finput[16];
char linput[16];
char lvinput[16];
BIT escape;                        /* flag: mark ESCAPE character entered */
int key=100;
int current_freq =0;
int current_volt =0;
int X2 = 0;
int X = 0;
int constant = 0;
int power = 0;									 												   
int intensity = 0;
short s3value = 0;
//short AD_last;

#define ESC  0x1B                     /* ESCAPE character code               */
#define MAX_CURRENT 1000
#define S3             0x2000                     // PC13: S3
#define S2             0x0001                     // PA0 : S2
#define UNBOUNCE_CNT        5                     // unbounce the keys
static U64 cmd_stack[800/8];          /* A bigger stack for command task     */


/***************************************************/
/*					Task prototypes		 			   */
/**************************8************************/
__task void init (void);              
__task void get_escape (void);
__task void command (void);
__task void lcd (void);
__task void lcd2 (void);
__task void photo (void);
__task void power_calc(void);
__task void motor (void);
__task void tachometer(void);

int S2Pressed (void) {
  static int S2KeyCount = 0, S2KeyPressed = 0;

  if (S2KeyPressed) {
    if (!((GPIOA->IDR & S2) == 0 )) {             // Check if S2 is not pressed
      if (S2KeyCount < UNBOUNCE_CNT) S2KeyCount++;
      else {
        S2KeyPressed = 0;
        S2KeyCount = 0;    
      }
    }
  }
  else {
    if (((GPIOA->IDR & S2) == 0 ))  {             // Check if S2 is pressed
      if (S2KeyCount < UNBOUNCE_CNT) S2KeyCount++;
      else {
        S2KeyPressed = 1;
        S2KeyCount = 0;
		return (1);
      }
    }
  }
  return (0);
}

/*----------------------------------------------------------------------------
  S3Pressed
  check if S3 is pressed (unbounced).
 *----------------------------------------------------------------------------*/
int S3Pressed (void) {
  static int S3KeyPressed = 0;

  if (S3KeyPressed) {
    if (!((GPIOC->IDR & S3) == 0 )) {             // Check if S3 is not pressed
        S3KeyPressed = 0;
        
      
    }
  }
  else {
    if (((GPIOC->IDR & S3) == 0 ))  {             // Check if S3 is pressed
      
        S3KeyPressed = 1;
        
		return (1);
      
    }
  }
  return (0);
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/

int main (void) {                     /* program execution starts here       */

	/* init and start with task 'INIT'     */
   stm32_Init ();                      /* STM32 setup (must be called first   */
   os_sys_init (init);   
 
   
        
}



/*----------------------------------------------------------------------------
 *        Task 1 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

    PWM_init();    
	t_motor = os_tsk_create (motor,0);
	t_lcd     = os_tsk_create (lcd, 0);    /* start task lcd                   */
    os_tsk_delete_self ();                 /* stop init task (no longer needed)*/
}


/*----------------------------------------------------------------------------
 *        Task 6 'Tachometer': Read the tachometer value input
 *---------------------------------------------------------------------------*/
__task void tachometer(void){
   
   
//	os_itv_set (100); /* set wait interval:  5 second        */     
	while (1) { 			  /* Loop forever                     */
    AD_tach = ADC_getValue(1);                /* Read AD_last value               */
	AD_tach /=10;
	os_dly_wait (100);                   /* wait interval (already set to 5s )  */	
    }	
      
}
/*----------------------------------------------------------------------------
 *        Task 7 'Photo Sensor': Read the current value input
 *---------------------------------------------------------------------------*/
__task void photo (void) {
 	    
	 while (1) { 
	 AD_photo = ADC_getValue(0);                 /* Read AD_last value               */
	 AD_photo /= 10;                     /* Scale to AD_Value to 0 - 78      */
	 photo_update(AD_photo,AD_photo_old);
	 AD_photo_old = AD_photo;
	 os_dly_wait (100);                   /* wait interval (already set to 5s )  */	
	}
	

}

/*----------------------------------------------------------------------------
 *        Task 5 'tracker': Calculates the power
 *---------------------------------------------------------------------------*/

__task void tracker(void){
	
	power_calculate(current_volt);  //power calculation from the voltage input

}

/*----------------------------------------------------------------------------
 *        Task 7 'get_escape': check if ESC (escape character) was entered
 *---------------------------------------------------------------------------*/
__task void get_escape (void) {
  while (1)  {                                /* endless loop                */
    if (GetKey () == ESC) {                   /* If ESC entered, set flag    */ 
      escape = __TRUE;                        /* 'escape', set event flag of */
      os_evt_set (0x0002, t_command);         /* task 'command' and          */
    }
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'command': command processor
 *---------------------------------------------------------------------------*/
__task void command (void) {
  U32 i;

  printf (menu);                              /* display command menu        */
  while (1) {                                 /* endless loop                */
    printf ("\nCommand: ");                   /* display prompt              */
    getline (cmdline, sizeof (cmdline));      /* get command line input      */

    for (i = 0; cmdline[i] != 0; i++) {       /* convert to uppercase        */
      cmdline[i] = (char) toupper(cmdline[i]);
    }

    for (i = 0; cmdline[i] == ' '; i++);      /* skip blanks                 */

    switch (cmdline[i]) {                     /* proceed to command function */
      case 'D':                               /* Display Time Command        */
        printf ("The current frequency output value is %d Hz\n",current_freq);
		getline (cmdline, sizeof (cmdline));      /* get command line input      */
		break;

      case 'F':                               /* Set Time Command            */
         printf ("Enter a low frequency output value in Hz\n");
         getline (cmdline, sizeof (cmdline));      /* get command line input      */
        break;

      case 'C':                               /* Set End Time Command        */
       	printf ("Enter a current output value in mA\n");
		getline (cmdline, sizeof (cmdline));      /* get command line input      */
        break;

      case 'V':                               /* Set Start Time Command      */
        printf ("Enter a voltage output value in V\n");
		getline (cmdline, sizeof (cmdline));      /* get command line input      */
        break;

      default:                                /* Error Handling              */
        printf (menu);                        /* display command menu        */
        break;
    }   
  }
}

__task void lcd2 (void) {

	lcd_init (); 
	lcd_clear ();
 	for(;;) {

	 lcd_print ("Tachospeed: ");
	 set_cursor(0,1);
	 Fixed_Fix2Str(AD_tach);
     lcd_print (" RPM");
	 os_dly_wait(100);
	
	 lcd_clear ();
	 lcd_print("Intensity: ");
	 set_cursor(0,1);
	 Fixed_Fix2Str(AD_photo);
	 lcd_print (" V");
	 os_dly_wait(100); 
	 lcd_clear ();

	} 
 
} 

/*----------------------------------------------------------------------------
 *        Task 2 'lcd': LCD Control task
 *---------------------------------------------------------------------------*/
__task void lcd (void) {
  
   unsigned short index1=0;
   unsigned short index2=0;
   unsigned short i;
   lcd_init ();                           /* Initialize LCD display module    */
   for (;;) { 
  	int decctr =0;
	while(key==100){
    key=keyScan();
	lcd_clear ();
    lcd_print ("Welcome to the");
	set_cursor(0,1);
	lcd_print ("SPMD system  ");          
    os_dly_wait (200);
	lcd_clear ();
    lcd_print ("Press WK/TA to ");
	set_cursor(0,1);
	lcd_print ("Start/Stop motor ");          
    os_dly_wait (200); 
	lcd_clear ();
    lcd_print ("Select an ");
	set_cursor(0,1);
	lcd_print ("option");          
    os_dly_wait (200);
	lcd_clear ();
    lcd_print ("1-View system");
	set_cursor(0,1);
	lcd_print("  Data");
	os_dly_wait(200);
		lcd_clear();
	lcd_print("2-Add V/F point");
    os_dly_wait (200);
	lcd_clear ();
    lcd_print ("3-Enter equation");
	os_dly_wait(200);
	lcd_clear();
	lcd_print("4-Enter Solar");
    set_cursor(0,1);
	lcd_print("  Data");
	os_dly_wait (200);
	lcd_clear ();
	lcd_print ("5-Clear Motor");
	set_cursor(0,1);
	lcd_print("  Data");
	os_dly_wait (200);
	lcd_clear();
    lcd_print ("6-Clear Solar");
	set_cursor(0,1);
	lcd_print("  Data");
    os_dly_wait (200);

	}									  
	
	switch(key){
	case 1: 
    /*Printing system Data*/
		while(key!=11){
		key = keyScan();
		if(key==11) break;
		lcd_clear ();
    	lcd_print ("Frequency :");
		set_cursor(0,1);
		LCD_outDec (current_freq);
		key = keyScan();
		if(key==11) break;
		os_dly_wait (200);
		lcd_clear ();
    	lcd_print ("Power :");
		set_cursor(0,1);
		LCD_outDec (power);
		key = keyScan();
		if(key==11) break;
		os_dly_wait (200);   
		lcd_clear ();
    	lcd_print ("Solar Intensity:");
		set_cursor(0,1);
		LCD_outDec (AD_photo);             
    	os_dly_wait (200);
		key = keyScan();
		if(key==11) break;
		}
		key=100;
	    break;
	
	case 2: 
	 /*Getting V/F Data from the user*/
	lcd_clear ();
    lcd_print ("Enter # to exit");
	os_dly_wait(200);
	while(key!=11) { 
    
	/*Enter Voltage Point*/
			lcd_clear ();
			key=0;

    		lcd_print ("Enter V point: ");          
    	    set_cursor(0,1);
			decctr=0;

			os_dly_wait (5);
			key = keyScan();
			if(key==11) break;
			while((key==100)|| (key ==10)){key=keyScan();}

			while(key!=10){
				
				if((key!=100)&&(key!=10)&&(key!=11)){
					LCD_putchar (key+48);
					if(decctr==0){
					   vinput[index1] = key;
					}else{
						 vinput[index1] = (vinput[index1]*10)+ key;
					   }
					    
					decctr++;
				}
				os_dly_wait (5);
				key=keyScan();
				if(key==11) break;
				
			}
			if(key==11) { key =100; break;}
			os_dly_wait (200);
			
	/*Enter Frequency Point*/
			key=100;
			lcd_clear ();
    		lcd_print ("Enter F point ");          
    		set_cursor(0,1);
			decctr=0;
			while((key==100)|| (key ==10)){key=keyScan();}

			while(key!=10){
				 
				if((key!=100)&&(key!=10)&&(key!=11)){
					LCD_putchar (key+48);
					if(decctr==0){
					   finput[index1] = key;

					}else{
						 finput[index1] = (finput[index1]*10)+ key;
					   }
					    
					decctr++;
					if(key==11) key =100;
				}
				os_dly_wait (5);
				key=keyScan();
				if(key==11) key =100;
		     }
			 os_dly_wait (200);
			if(index1!=15){
			index1++;
			}
			else{
			index1=0;
			}
		  }
	key=100;
	break;

	case 3:
	/*Getting Equation from the user*/
	   	decctr=0;
		key =0;
		lcd_clear ();
    	lcd_print ("Enter the ");
		set_cursor(0,1);
		lcd_print("coefficients");
    	os_dly_wait (200);
		lcd_clear ();
    	lcd_print ("X^2: ");
		set_cursor(0,1);
		while(key!=10){
				os_dly_wait (200);
				key=keyScan();
				if((key!=100)&&(key!=10)){
					LCD_putchar (key+48);
					if(decctr==0){
					   X2 = key;

					}else{
						 X2 = (X2*10)+ key;
					}
					    
					decctr++;
				}
		     }
		
		os_dly_wait (200);
		decctr=0;
		key=0;
		lcd_clear();
		lcd_print ("X: ");
		set_cursor(0,1);
		while(key!=10){
				os_dly_wait (200);
				key=keyScan();
				if((key!=100)&&(key!=10)){
					LCD_putchar (key+48);
					if(decctr==0){
					   X = key;

					}else{
						 X = (X*10)+ key;
					}
					    
					decctr++;
				}
		     }
		
		os_dly_wait (200);
		decctr=0;
		key=0;
		lcd_clear();
		lcd_print ("Constant ");
		set_cursor(0,1);
		while(key!=10){
				os_dly_wait (200);
				key=keyScan();
				if((key!=100)&&(key!=10)){
					LCD_putchar (key+48);
					if(decctr==0){
					   constant = key;
                    }
					else{
						 constant = (constant*10)+ key;
					}
					    
					decctr++;
				}
		     }
		
		os_dly_wait (200);
	    key=100;
		break;

	case 4:
			 /*Getting Solar Panel input data*/
	lcd_clear ();
	key=0;
    lcd_print ("Enter # to exit");
	while(key!=11) { 
    		key=100;
	/*Enter Light intensity Point*/
			lcd_clear ();
    		lcd_print ("Enter Light     "
			          "intensity point:"); 
			os_dly_wait (200);
	        lcd_clear ();
    		lcd_print ("in W/m");          
    	    set_cursor(0,1);
			decctr=0;
			while((key==100)|| (key ==10)){key=keyScan();}
			while(key!=11){
				
				if((key!=100)&&(key!=10)&&(key!=11)){
					LCD_putchar (key+48);
					if(decctr==0){
					   linput[index2] = key;
					}else{
						 linput[index2] = (linput[index2]*10)+ key;
					   }
					    
					decctr++;
				}
				os_dly_wait (200);
				key=keyScan();
				//if(key ==11) break;
			}
			os_dly_wait (200);
			//if(key==11) break;
	/*Enter Voltage output Point*/
			
			key=100;
			lcd_clear ();
    		lcd_print ("Enter V point ");          
    		set_cursor(0,1);
			decctr=0;
			while((key==100)|| (key ==10)){key=keyScan();}
			while(key!=11){
				
				if((key!=100)&&(key!=10)&&(key!=11)){
					LCD_putchar (key+48);
					if(decctr==0){
					   lvinput[index2] = key;

					}else{
						 lvinput[index2] = (lvinput[index2]*10)+ key;
					}
					    
					decctr++;
				}
				os_dly_wait (200);
				key=keyScan();
				//os_dly_wait(2000);
		     }
			if(index2!=15){
			index2++;
			}
			else{
			index2=0;
			}
		  }
	key=100;
	break;
	
	case 5:
	
	lcd_clear ();
    lcd_print ("V/F table");
	set_cursor(0,1);
	lcd_print ("cleared");
	os_dly_wait (200);
	for(i=0;i<16;i++){
		vinput[i]=0;
		finput[i]=0;
	}
	key=100;
	break;

	case 6:
    lcd_clear ();
    lcd_print ("LI/V table");
	set_cursor(0,1);
	lcd_print ("cleared");
	os_dly_wait (200);
	
	for(i=0;i<16;i++){
		lvinput[i]=0;
		linput[i]=0;
	}
	key=100;
	break;
	
	default: 
	key=100;
	break;
}
}
}
 /*----------------------------------------------------------------------------
 *        Task 3 'Keyread': Reads input to start PWM generation, current sensing and power calculation
 *---------------------------------------------------------------------------*/
__task void motor (void) {

   while (1) {                                     // Loop forever
   if(S3Pressed() && s3value == 0){
   	s3value = 1;
	PWM_start();                             // enable timer
	ADC_SoftwareInit();
	os_tsk_delete(t_lcd);
	t_lcd2     = os_tsk_create (lcd2, 0);    /* start task lcd                   */
    t_tach = os_tsk_create (tachometer, 0);   
   	t_photo = os_tsk_create (photo, 0); 
	}
	else if(S3Pressed() && s3value==1){
	s3value = 0;
    PWM_stop();	                         // enable timer
	os_tsk_delete(t_lcd2);
	os_tsk_delete(t_tach);
	os_tsk_delete(t_photo);
	t_lcd = os_tsk_create(lcd,0);
	}
   }

   // end while
}



/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
