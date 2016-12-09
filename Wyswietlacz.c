#include "Wyswietlacz.h"


/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void)
{
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */

  TWSR = 0;                         /* no prescaler */
  TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */

}/* i2c_init */


/*************************************************************************
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
unsigned char i2c_start(unsigned char address)
{
    uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

}/* i2c_start */


/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready

 Input:   address and transfer direction of I2C device
--------------------------------------------------------------------------
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void i2c_stop(void)
{
    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

}/* i2c_stop */


/*************************************************************************
  Send one byte to I2C device

  Input:    byte to be transfered
  Return:   0 write successful
            1 write failed
*************************************************************************/
unsigned char i2c_write( unsigned char data )
{
    uint8_t   twst;

	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

}/* i2c_write */


/*************************************************************************
 Read one byte from the I2C device, request more data from device

 Return:  byte read from I2C device
*************************************************************************/



void _LCD_Write (unsigned char dataToWrite, unsigned char RS)
{
	unsigned char temp =dataToWrite & 0xF0;
	if (RS==1)
		temp |= 13;
	else
		temp |= 12;
	i2c_write(temp);
	i2c_write(temp&0xFB);
	temp = dataToWrite << 4;
	if (RS==1)
		temp |= 13;
	else
		temp |= 12;
	i2c_write(temp);
	i2c_write(temp&0xFB);
	_delay_ms(2);
}
void LCD_Clear(void)
{
	_LCD_Write(HD44780_CLEAR,0);
	_delay_ms(2);
}
void LCD_Initialize(int address)
{
	i2c_start(address);
	i2c_write(8);
	for( int i=0; i<3; i++)
	{
		i2c_write(52);
		i2c_write(48);
		_delay_ms(5);
	}
	i2c_write(44);
	i2c_write(40);
	_delay_ms(1);
	_LCD_Write(HD44780_FUNCTION_SET | HD44780_FONT5x7 | HD44780_TWO_LINE | HD44780_4_BIT,0);
	_LCD_Write(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_OFF,0);
	LCD_Clear();
	_LCD_Write(HD44780_ENTRY_MODE | HD44780_EM_SHIFT_CURSOR | HD44780_EM_INCREMENT,0);// inkrementaja adresu i przesuwanie kursora
	_LCD_Write(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK,0); // w³¹cz LCD, bez kursora i mrugania
	LCD_Clear();
}
void LCD_WriteText(char * text)
{
while(*text)
  _LCD_Write(*text++,1);
}

void LCD_GoTo(unsigned char x, unsigned char y)
{
_LCD_Write(HD44780_DDRAM_SET | (x + (0x40 * y)),0);
}
