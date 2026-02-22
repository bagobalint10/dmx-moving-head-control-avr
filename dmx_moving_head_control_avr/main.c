/*
 * robotlampa_final.c
 *
 * Created: 2025. 10. 09. 14:43:11
 * Author : bagob
 */ 

#include <avr/io.h>

#include "control_board.h"
#include "timer.h"
#include "dmx_usart.h"
#include "motor_drive.h"

static int dmx_pos;

static void tmp_timer(void);

int main(void)
{
	timers_init();
	motor_1_init();

    while (1) 
    {	 	
	    motor_1_main(dmx_pos); 
		tmp_timer();	
    }
}

static void tmp_timer(void)
{
   static uint32_t tmp_current_time = 0;
   static uint32_t tmp_time_0 = 0;
   static uint16_t tmp_interval_0 = 10000;	//10mp

   tmp_current_time = millis();

   if ((uint32_t)(tmp_current_time - tmp_time_0) >= tmp_interval_0)
   {
	   tmp_time_0 = tmp_current_time;
	   
	   if(dmx_pos) dmx_pos = 0;				// dmx pozíció változtatása
	   else  dmx_pos = 10000;
   }
}
