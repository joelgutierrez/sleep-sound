/********************************************
*
*  Name: Joel Gutierrez
*  Section: 4pm - Thursday
*  Assignment: Project
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"


void writenibble(unsigned char);

/*
  init_lcd - Configure the I/O ports and send the initialization commands
*/
void init_lcd()
{
	//Set the DDR register bits for ports B and D
    DDRD |= (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4);	
    DDRB |= (1 << 0 | 1 << 1);

    _delay_ms(15);              // Delay at least 15ms

    writenibble(0x30);          // Use writenibble to send 0011
    _delay_ms(5);               // Delay at least 4msec

    writenibble(0x30);          // Use writenibble to send 0011
    _delay_us(120);             // Delay at least 100usec

    writenibble(0x30);          // Use writenibble to send 0011, no delay needed

    writenibble(0x20);          // Use writenibble to send 0010
    _delay_ms(2);               // Delay at least 2ms
    
    writecommand(0x28);         // Function Set: 4-bit interface, 2 lines

    writecommand(0x0f);         // Display and cursor on
}

/*
  moveto - Move the cursor to the row (1 to 2) and column (1 to 16) specified
*/
void moveto(unsigned char row, unsigned char col)
{
    if(row == 0)
    {
    	
    	 writecommand(0x80 + ((0x00)+col));
    }
    else
    {
    	writecommand(0x80 + ((0x40)+col));
    }

}

/*
  stringout - Write the string pointed to by "str" at the current position
*/
void stringout(char *str)
{
	int i=0;
    while(str[i] != '\0')
    {
    	writedata(str[i]);
    	i++;
    }
    i++;
}

/*
  writecommand - Send the 8-bit byte "cmd" to the LCD command register
*/
void writecommand(unsigned char cmd)
{
	PORTB &= ~(1 << PB0);
	writenibble(cmd);
	cmd = cmd << 4;
	writenibble(cmd);
	_delay_ms(2);
}

/*
  writedata - Send the 8-bit byte "dat" to the LCD data register
*/
void writedata(unsigned char dat)
{
	PORTB |= (1 << PB0);
	writenibble(dat);
	dat = dat << 4;    
	writenibble(dat);
	_delay_ms(2);
	
}

/*
  writenibble - Send four bits of the byte "lcdbits" to the LCD
*/
void writenibble(unsigned char lcdbits)
{
	PORTD &= 0x0F;
	PORTD |= lcdbits & (0xF0);
	
	//PORTB &= ~(1 << PB1);		//0-1-0 state
	PORTB |= (1 << PB1);		//230ns
	PORTB |= (1 << PB1);
	PORTB &= ~(1 << PB1);
}


