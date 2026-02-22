/*
 * control_board.c
 *
 * Created: 2025. 10. 10. 16:52:46
 *  Author: bagob
 */ 

 #include "control_board.h"

 #include "buttons.h"
 #include "timer.h"
 #include "lcd_driver.h"
 #include "eepromh.h"
 #include "port_config.h"
 #include "relay.h"

 uint8_t *dmx_adress_pointer;
 uint8_t dmx_array[512];

 static int dmx_adress = 0x01;

 // button variables

 static uint8_t bt_up = 1;
 static uint8_t bt_down = 1;
 static uint8_t bt_enter = 1;
 static uint8_t bt_mode = 1;

 // lcd variables

 static uint8_t lcd_buffer[4] = "abcd";
 static uint8_t lcd_dot_buffer[4] = {0,0,0,0};
 static uint8_t lcd_enable = 1;

 static int menu_n = 1;				
 static uint8_t sub_menu_f = 0;
 static int sub_menu_n = 0;

 static uint8_t dmx_menu_blink = 0;

 static uint8_t lamp_cold_f = 0; 
 static uint8_t lamp_count = 0;

 static void push_string(void)
 {
	static const char *menu_string[3]=	{	"rst ",
											"a   ",
											"lamp" };
			
	static const char *sub_menu_string[3]=	{	"  on",
												" off",
												"    "};

	const char *menu_pointer;	// for string_push()

	if(sub_menu_f) menu_pointer = sub_menu_string[sub_menu_n];  
	else menu_pointer = menu_string[menu_n];	
	if(menu_n == 1) menu_pointer = menu_string[1];		
	if((menu_n == 1) && dmx_menu_blink) menu_pointer = sub_menu_string[2];	

	for (int i = 0; i<4; i++)
	{
		lcd_buffer[i] = *(menu_pointer+i);
	}

	if(menu_n == 1)	
	{
		char s_adress[3];

		s_adress[2] = (dmx_adress%10)+48;	
		s_adress[1] = (dmx_adress/10%10)+48;
		s_adress[0] = (dmx_adress/100%10)+48;
				
		for (int i = 1; i<4; i++)				// str_copy 1-3
		{
			lcd_buffer[i] = s_adress[i-1];
		}
	}	 
 }

 static void menu(void)
 {
	static uint32_t current_time = 0;
	current_time = millis();
	static uint32_t prev_time_long = 0;
	static uint16_t interval_long = 1000;		// long button press time
	
	static uint32_t prev_time_counter = 0;
	static uint16_t interval_counter = 50;  	// fast counting time

	static uint32_t prev_time_blink = 0;
	static uint16_t interval_blink = 350;  		// letter "A" blink time

	static uint32_t prev_time_save = 0;
	static uint16_t interval_save = 100;  		// save blink time

	static uint32_t prev_time_heat_blink = 0;
	static uint16_t interval_heat_blink = 100;  // heat dots animation

	static uint32_t prev_time_lamp_cold = 0;
	static uint32_t interval_lamp_cold = 300000;// cool down time - 5 min  5*60*1000


	static uint8_t heat_dots = 0x02;
	static uint8_t heat_dots_dir = 0x01; 

	// rising edge detection
	static uint8_t bt_up_tmp = 1;
	static uint8_t bt_down_tmp = 1;
	static uint8_t bt_enter_tmp = 1;
	static uint8_t bt_mode_tmp = 1;

	uint8_t enter_f = 0; 
	uint8_t mode_f = 0;
	uint8_t up_f = 0;
	uint8_t down_f = 0;

	uint8_t up_long_f = 0;
	uint8_t down_long_f = 0;
	uint8_t enter_long_f = 0;

	static uint8_t save_f = 0;
	static uint8_t save_counter = 0;
	static uint8_t save_once = 0;
	static uint8_t clear_once = 0;
	static uint8_t lamp_on_f = 0;
	
	

	//---------> edgedetection

	if(bt_up^bt_up_tmp)		
	{
		up_f = !bt_up;		
		bt_up_tmp = bt_up;
	}
	if(bt_down^bt_down_tmp)	 
	{
		down_f = !bt_down;	
		bt_down_tmp = bt_down;
	}
	if(bt_enter^bt_enter_tmp)	 
	{
		enter_f = !bt_enter;		 
		bt_enter_tmp = bt_enter;
	}
	if(bt_mode^bt_mode_tmp)	 
	{
		mode_f = !bt_mode;	 
		bt_mode_tmp = bt_mode;
	}
	
	// long button pressed: enter, up, down

	if((bt_enter+bt_down+bt_up) == 2)	// only one button has been pressed
	{ 
		if (!bt_enter)		 
		{
			if(enter_f) prev_time_long = current_time;	
			if ((uint32_t)(current_time - prev_time_long)>= interval_long) enter_long_f = 1; 
		}
		else if (!bt_up)	
		{
			if(up_f) prev_time_long = current_time;	
			if ((uint32_t)(current_time - prev_time_long)>= interval_long) up_long_f = 1; 
		}
		else if (!bt_down)	
		{
			if(down_f) prev_time_long = current_time;	
			if ((uint32_t)(current_time - prev_time_long)>= interval_long) down_long_f = 1; 
		}
	}
	else
	{
		enter_long_f = 0;	 
		up_long_f = 0;
		down_long_f = 0;
	} 
	
	// menu

	switch(menu_n)
	{
	case 0:	
				// reset();
	
				break;

	case 1:		// DMX ADRESS
				if(sub_menu_f)  				// SUBMENU 
				{
					if(mode_f) sub_menu_f = 0;
					if(up_f) dmx_adress++;
					if(down_f) dmx_adress--;

					if ((uint32_t)(current_time - prev_time_blink)>= interval_blink)
					{
						prev_time_blink = current_time;
						dmx_menu_blink ^= 0x01;
					}

					if (up_long_f)
					{
						if ((uint32_t)(current_time - prev_time_counter)>= interval_counter)
						{
							prev_time_counter = current_time;
							dmx_adress++;
						}
					}
					if (down_long_f)
					{
						if ((uint32_t)(current_time - prev_time_counter)>= interval_counter)
						{
							prev_time_counter = current_time;
							dmx_adress--;
						}
					}	    
				}
				else		
				{

					if(enter_f) sub_menu_f = 1;	
					if(up_f) menu_n--;	
					if(down_f) menu_n++;

					dmx_menu_blink = 0;
					
				} 
				break;

	case 2:		// LAMP
	
				if(sub_menu_f)  				
				{					
					if(mode_f) sub_menu_f = 0;	
					if(up_f) sub_menu_n--;	
					if(down_f) sub_menu_n++;
				}
				else	
				{				
					if(enter_f) sub_menu_f = 1;	
					if(up_f) menu_n--;
					if(down_f) menu_n++;
				}
				break;
	default: break;
	}

	// MENU OVERFLOW GUARD
	if(menu_n < 1) menu_n = 1;
	if(menu_n > 2) menu_n = 2;

	if(sub_menu_n < 0) sub_menu_n = 0;
	if(sub_menu_n > 1) sub_menu_n = 1;

	// ADRESS OVERFLOW GUARD
	if(dmx_adress < 1) dmx_adress = 512;
	if(dmx_adress > 512) dmx_adress = 1;

	push_string();
								 
	// save enter

	if (sub_menu_f && enter_long_f)
	{ 
		save_f = 1;
	}

	if(save_f && !save_once)
	{
		save_once = 1;

		if(menu_n == 2)
		{
			if(sub_menu_n == 0) lamp_on_f = 1;		
			else if (sub_menu_n == 1) lamp_on_f = 0;

		}else if(menu_n == 1)
		{
			eeprom_write_byte(EEPR_ADR_DMX_ARD_0, (uint8_t)dmx_adress);	
			eeprom_write_byte(EEPR_ADR_DMX_ADR_1, (uint8_t)(dmx_adress >> 8));

			dmx_adress_pointer = (dmx_array+(dmx_adress-1)+2);
		}
	}

	if (save_f && (save_counter < 10))	
	{
		if ((uint32_t)(current_time - prev_time_save) >= interval_save)
		{
			prev_time_save = current_time;
			lcd_enable ^= 0x01;
			save_counter++;
		}
	}else lcd_enable = 1;

	if (bt_enter)
	{
		save_counter = 0; 
		save_f = 0;
		save_once = 0;
	}

	// heat animation

	if(lamp_on_f && lamp_count)
	{
		if ((uint32_t)(current_time - prev_time_heat_blink)>= interval_heat_blink)
		{
			prev_time_heat_blink = current_time;
			if(heat_dots==0x01 || heat_dots == 0x08) heat_dots_dir ^= 0x01;
			if(heat_dots_dir) heat_dots = (heat_dots<<1);
			else heat_dots = (heat_dots>>1);	
		}

		for (int i = 0; i<4;i++)
		{
			lcd_dot_buffer[i] = (heat_dots&(0x01<<i))&&0x01;
		}

		clear_once = 1;

	}else if(clear_once)   //clear all flag; 
	{
		clear_once = 0;
		for (int i = 0; i<4;i++)
		{
			lcd_dot_buffer[i] = 0x00;
		}
	}

	/// relay time measure

	if (lamp_on_f && lamp_cold_f)
	{
		lamp_cold_f = 0;	
		relay_set();		
		eeprom_write_byte(EEPR_ADR_LAMP, lamp_cold_f);

		for (int i = 0; i<4;i++)
		{
			lcd_dot_buffer[i] = 0x01;
		}

	}else if (!lamp_on_f && !lamp_cold_f && !lamp_count)
	{ 
		relay_reset();

		prev_time_lamp_cold = current_time;
		lamp_count = 1;
		for (int i = 0; i<4;i++)
		{
			lcd_dot_buffer[i] = 0x00;
		}
	}

	if (lamp_count) 
	{
		if ((uint32_t)(current_time - prev_time_lamp_cold)>= interval_lamp_cold)
		{

			lamp_cold_f = 1;
			eeprom_write_byte(EEPR_ADR_LAMP, lamp_cold_f);
			lamp_count = 0;			
		}
	}
 }

 void control_board_init(void)
 {
	buttons_init(10); //debounce time in ms
	set_buttons_variables(&bt_up, &bt_down, &bt_enter, &bt_mode);
	lcd_init(4);	// 4x4 = 16ms refresh time 
	relay_init();

	lamp_cold_f = eeprom_read_byte(EEPR_ADR_LAMP);
	dmx_adress = eeprom_read_byte(EEPR_ADR_DMX_ARD_0);
	dmx_adress |= (eeprom_read_byte(EEPR_ADR_DMX_ADR_1)<<8);

	dmx_adress_pointer = (dmx_array+(dmx_adress-1)+2);
	dmx_adress_pointer = dmx_array; 


 }

 void control_board_main(void)
 {
	button_read(); 
	menu();	 
	lcd_write_buffer(lcd_buffer, lcd_dot_buffer, lcd_enable);	
 }
   


