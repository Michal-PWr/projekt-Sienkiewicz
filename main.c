/*----------------------------------------------------------------------------
 Copyright:
 Author:         Radig Ulrich
 Remarks:
 known Problems: none
 Version:        16.12.2009
 Description:
------------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>

#include "Wyswietlacz.h"

#define USTAW_PD0 PORTD |= 1<<PD0
#define SKASUJ_PD0 PORTD &= ~(1<<PD0)


/////////////////////////////////////////



volatile char znak = 'a';
volatile uint8_t isTransmission = 0;
volatile uint8_t address = 0, command = 0;
volatile uint8_t signals[32];
volatile uint8_t transmissionFinished = 0;
void wyslij_0()
{
	USTAW_PD0;
	_delay_us(562);
	SKASUJ_PD0;
	_delay_us(562);
}
void wyslij_1()
{
	USTAW_PD0;
	_delay_us(562);
	SKASUJ_PD0;
	_delay_ms(1);
	_delay_us(687);
}
void wyslij(uint8_t addressToSend, uint8_t commandToSend)
{
	USTAW_PD0;
	_delay_ms(9);
	SKASUJ_PD0;
	_delay_ms(4);
	_delay_us(500);
	for (uint8_t i=0; i<8; i++)
	{
		if ((addressToSend >> i) & 0x01)
			wyslij_1();
		else
			wyslij_0();
	}
	for (uint8_t i=0; i<8; i++)
	{
		if ((addressToSend >> i) & 0x01)
			wyslij_0();
		else
			wyslij_1();
	}
	for (uint8_t i=0; i<8; i++)
	{
		if ((commandToSend >> i) & 0x01)
			wyslij_1();
		else
			wyslij_0();
	}
	for (uint8_t i=0; i<8; i++)
	{
		if ((commandToSend >> i) & 0x01)
			wyslij_0();
		else
			wyslij_1();
	}
	wyslij_0(); // kod stopu
}
int main(void)
{
	DDRC|=( (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) |  (1<<PC4) | (1<<PC5)) ;
	PORTC = 0x1f;
	i2c_init();
	int adres=0;
	for (int i=0; i<0xFF ; i++)
		if (i2c_start(i)==0)
		{
			adres=i;
			i=0xFF;
		}
    LCD_Initialize(adres);
    LCD_WriteText("ABC");
	DDRB |= (1<<0);
	PORTB |= (1<<0);
	DDRD |= 0x03;
	DDRD &= ~(1 << 2);
	PORTD |= (1 << 2);
	PORTD &= ~( 1<<PD2 | 1<<PD3 );
	MCUCR = (1<<ISC00 | 1<<ISC01);
	GICR |= (1 << INT0);

	//for(unsigned long b = 0;b<100000;b++){;asm("nop");};
	TCCR1B &= 0xF8;
	TCCR1B |= (1 << CS11);//preskaler 8
	TCNT1 = 0;
	TIMSK |= (1 << TOIE1);
	sei();

	while(1)
	{
		wyslij(5, 0);
		if (transmissionFinished == 1)
		{
			transmissionFinished = 0;
			_LCD_Write(signals[1] / 100 +'0', 1);
			_LCD_Write((signals[1] % 100) / 10 +'0', 1);
			_LCD_Write(signals[1] % 10 +'0', 1);
		}
		_delay_ms(2000);
		LCD_Clear();
	}

}

ISR (INT0_vect)
{
	int temp = TCNT1;
	TCNT1 = 0;
	static uint8_t index = 0;
	if (isTransmission == 0)
	{
		index = 0;
		isTransmission = 1;
	}
	else
	{
		if (temp < 200)
		{
			signals[index / 8] &= ~(1 << index % 8);
			index++;
		}
		else if (temp < 500)
		{
			signals[index / 8] |= (1 << index % 8);
			index++;
		}

	}
	if (index == 33)
	{
		index = 0;
		transmissionFinished = 1;
		isTransmission = 0;
	}

}

ISR(TIMER1_OVF_vect)
{
	;//LCD_Clear();
}

