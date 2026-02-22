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
	while(EECR & (1<<EEPE));	// waiting for last write 

	cli();

	EEAR = uiAddress;			// data + adress 
	EEDR = ucData;
	 
	EECR |= (1<<EEMPE);			// write data --> eeprom
	EECR |= (1<<EEPE);
		 
	sei();
 }

 uint8_t eeprom_read_byte(unsigned int uiAddress)
 {
	 while(EECR & (1<<EEPE));	// wait for last read
	 
	 EEAR = uiAddress;			// set adress
	 EECR |= (1<<EERE);			// read data ,  eeprom   --> EEDR
	 
	 return EEDR;	
 }



