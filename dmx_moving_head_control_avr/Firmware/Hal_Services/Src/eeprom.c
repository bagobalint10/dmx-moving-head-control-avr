/*
 * eeprom.c
 *
 * Created: 2025. 10. 12. 12:08:14
 *  Author: bagob
 */ 

 #include "eepromh.h"

 #include <avr/interrupt.h>

 void eeprom_write_byte(unsigned int uiAddress, uint8_t ucData)
 {
	while(EECR & (1<<EEPE));	// elõzõ írás kivárása 

	cli();

	EEAR = uiAddress;			// adat + cím megadása 
	EEDR = ucData;
	 
	EECR |= (1<<EEMPE);			// adat beírása --> eeprom
	EECR |= (1<<EEPE);
		 
	sei();
 }

 uint8_t eeprom_read_byte(unsigned int uiAddress)
 {
	 while(EECR & (1<<EEPE));	// elõzõ írás várása
	 
	 EEAR = uiAddress;			// cím megadása
	 EECR |= (1<<EERE);			// adat kiolvasása ,  eeprom   --> EEDR
	 
	 return EEDR;	
 }



