/********************************************
 *  Name: Joel Gutierrez
 *  Date: December 2016
 *  Description: Closes a door if left open 
 *  between time A and time B
 ********************************************/
 
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

void init_lcd(void);
void init_adc(void);
void init_timer1(unsigned short max_count);
void close_door(void);
void moveto(unsigned char, unsigned char);
void stringout(char *);
void writecommand(unsigned char);
void writedata(unsigned char);
void writenibble(unsigned char);

#define SMALL_TIME	6	//needs to hands b/w ie: 6am and 10pm
#define BIG_TIME	10
#define AM_PM	   'A'
 
volatile int clockset_state = 0;	//0 = clock hasn't been set; 1 = clock has been set;
volatile int door_state = 0;		//0 = door closed or state of irrelevance
 									//1 = open door during time frame
 									//2 = 10min door open period expired
volatile int adc_value = 0;
volatile int row = 0;
volatile int column = 8;
volatile char clock[10];
volatile unsigned int time_hr = 1;
volatile unsigned int time_min_tens = 0;
volatile unsigned int time_min_ones = 0;
volatile unsigned char ampm = 'A';
volatile unsigned int secs = 0;
volatile unsigned char open_ones = 0;
volatile unsigned char open_tens = 0;
volatile unsigned char open_min = 0; 
//volatile unsigned int close_time = 0;

 
int main(void)
{
 	
	init_lcd();
	init_adc();
	writecommand(1);
	init_timer1(15625);
	DDRB |= (1 << PB3 | 1 << PB4);	//output for motor (d11-green wire) and LED (d12-white)
	
	char row0[] = "TIME =  1:00AM";	//row0[8,10,11,12] change
 	char row1[] = "MIN:SEC = 0:00";	//row1[10,12,13] change
 	
 	//prints rows
 	moveto(0,0);
	stringout(row0);
	moveto(1,0);
	stringout(row1);
	moveto(row, column);
 	
 	//set clock time
 	//interrupt way
	ADCSRA |= (1 << ADIE);	//ADC interrupt enabled 
	sei();					//enable global interrupts
 	ADCSRA |= (1 << ADSC); 	//start 1st ADC conversion
 	while(clockset_state == 0){}
 	moveto(0, 14);

 	//cli();
 	//PORTB |= (1 << PB3); 	//turn on motor
 	//_delay_ms(1000);
	//PORTB &= ~(1 << PB3); 	//turn off motor
 	
 	PORTC |= (1 << 5);	//enables A0 pull-up resistor on interrupter (ylw)
	ADMUX |= (1 << MUX2 | 1 << MUX0);	//port c pin 5 used for input
	//check for door status	
 	while(1)
 	{
 		if(door_state == 0)
 		{
 			PORTB &= ~(1 << PB4);	//turn off LED
 		}
 		else if(door_state == 1)	//*****change blinking LED scheme to 2 for loops? so blink can stop midway if door closes
 		{
 			//blink LED
 			PORTB |= (1 << PB4);
 			_delay_ms(1000);
 			PORTB &= ~(1 << PB4);
 			_delay_ms(1000);
 		}
 		else if(door_state == 2)
 		{
 			close_door();
 		}
 	}
 
	return 0;
}

/****************************************************************************************/
//closes door while still monitoring for current door_state
void close_door()
{
	int i = 0;
	for(i = 0; i < 25; i++)	//delay+#of intervals = 5000ms = 5sec
	{
		PORTB |= (1 << PB4);		//LED on while closing
		if(door_state == 2)
		{
			PORTB |= (1 << PB3); 	//turn on motor
			_delay_ms(200);
		}	
		else
		{
			PORTB &= ~(1 << PB3); 	//turn off motor
		}
	}
	
	PORTB &= ~(1 << PB3 | 1 << PB4); 	//turn off motor and LED
	door_state = 0;
}

/****************************************************************************************/

//Initializes the ADC
void init_adc()
{
	ADMUX |= (1 << REFS0 | 1 << ADLAR);				
	ADCSRA |= (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0 | 1 << ADEN);
}

//ADC interrupt
ISR(ADC_vect)
{
	adc_value = ADCH;
	if(clockset_state == 0)
	{	
		_delay_ms(175);
		if(adc_value > 230)
		{}
		else if(adc_value > 180)	//button "select" pressed? if yes,...
		{
			clockset_state = 1;
		}
		else if(adc_value > 130)	//button "left" pressed? if yes,...
		{
			if(column == 8){}
			else if(column == 10){column = 8;}
			else
			{
				column--;
			}
	 		moveto(row, column);
		}
		else if(adc_value > 80)		//button "down" pressed? if yes,...
		{
			if(column == 8)
			{
				if(time_hr > 1){time_hr--;}
				else{time_hr = 12;}
			}	
			else if(column == 10)
			{
				if(time_min_tens > 0){time_min_tens--;}
				else{time_min_tens = 5;}
			}
			else if(column == 11)
			{
				if(time_min_ones > 0){time_min_ones--;}
				else{time_min_ones = 9;}
			}
			else if(column == 12)
			{
				if(ampm == 'A'){ampm = 'P';}
				else{ampm = 'A';}
			}
			snprintf(clock, 7, "%2d:%d%d%c", time_hr, time_min_tens, time_min_ones, ampm);
			moveto(0,7);
			stringout(clock);
			moveto(row, column);
		}
		else if(adc_value > 30)		//button "up" pressed? if yes,...
		{
			if(column == 8)
			{
				if(time_hr < 12){time_hr++;}
				else{time_hr = 1;}
			}	
			else if(column == 10)
			{
				if(time_min_tens < 5){time_min_tens++;}
				else{time_min_tens = 0;}
			}
			else if(column == 11)
			{
				if(time_min_ones < 9){time_min_ones++;}
				else{time_min_ones = 0;}
			}
			else if(column == 12)
			{
				if(ampm == 'A'){ampm = 'P';}
				else{ampm = 'A';}
			}
			snprintf(clock, 7, "%2d:%d%d%c", time_hr, time_min_tens, time_min_ones, ampm);
			moveto(0,7);
			stringout(clock);
			moveto(row, column);
		}
		else						//button "right" pressed
		{
			if(column == 12){}
			else if(column == 8){column = 10;}
			else 
			{
				column++;
			}
	 		moveto(row, column);
		}
	}
	else //clock is already set	-- checking door open/close changes
	{
		if(door_state != 2)
		{
			if(time_hr >= SMALL_TIME && 
			time_hr <= BIG_TIME && ampm == AM_PM)
			{
				if(adc_value > 130) 		//door resistance = closed		
				{
					door_state = 0;
				}
				else	//door resistance = open	
				{
					door_state = 1;
				}
			}
			else{door_state = 0;}
		}
	}
	ADCSRA |= (1 << ADSC); 	//start next ADC conversion
}

/****************************************************************************************/

//sets ClearTimer_onCompare mode & the prescalar(1024)
//enable timer interrupt
//load max count; 0 to 15,625 in 1sec in 16MHz clock; counter modulus
//enables timer interrupt
void init_timer1(unsigned short max_count)
{
	TCCR1B |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = max_count;
	TCCR1B |= (1 << CS12 | 1 << CS10);
}

//Timer interrupt
ISR(TIMER1_COMPA_vect)
{
	if(clockset_state == 0){}
	else
	{
		secs++;	
		if(secs%60 == 0)
		{
			secs = 0;
			time_min_ones++;
			if(time_min_ones%10 == 0)
			{
				time_min_ones = 0;
				if(time_min_tens == 5)
				{
					time_min_tens = 0;
					time_hr++;
					if(time_hr%12 == 0)
					{
						if(ampm == 'A'){ampm = 'P';}
						else{ampm = 'A';}
					}
					else if(time_hr == 13) 
					{
						time_hr = 1;
					}
				}
				else
				{
					time_min_tens++;
				}
			}
			snprintf(clock, 7, "%2d:%d%d%c", time_hr, time_min_tens, time_min_ones, ampm);
			moveto(0,7);
			stringout(clock);
			moveto(0, 14);
		}
		
		
		if(door_state == 0)
		{
			open_ones = 0;
			open_tens = 0;
			open_min = 0;
			moveto(1, 10);
			writedata(0x30 + open_min);
			moveto(1, 12);
			writedata(0x30 + open_tens);
			moveto(1, 13);
			writedata(0x30 + open_ones);
			moveto(1, 14);
		}
		else if(door_state == 1)
		{
			open_ones++;
				
			if(open_ones%10 == 0)
			{
				open_ones = 0;
				open_tens++;
				if(open_tens%6 == 0)
				{
					open_tens = 0;
					if(open_min == 9)
					{
						door_state = 2;
						open_ones = 0;
						open_tens = 0;
						open_min = 0;
					}
					else
					{
						open_min++;
					}
				}	
			}
			moveto(1, 10);
			writedata(0x30 + open_min);
			moveto(1, 12);
			writedata(0x30 + open_tens);
			moveto(1, 13);
			writedata(0x30 + open_ones);
			moveto(1, 14);
		}
		else if(door_state == 2)
		{
			//close_time++;
			
			open_ones = 0;
			open_tens = 0;
			open_min = 0;
			moveto(1, 10);
			writedata(0x30 + open_min);
			moveto(1, 12);
			writedata(0x30 + open_tens);
			moveto(1, 13);
			writedata(0x30 + open_ones);
			moveto(1, 14);
		}
	}
}