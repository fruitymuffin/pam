/*
 * led-controller.cpp
 *
 * Created: 16/04/2024 7:16:16 PM
 * Author : jmcna
 */ 

// Clock params
#define F_CPU 24000000

// USART 2 params
#define USART_BAUD_RATE(BAUD_RATE) ((float)(4000000 * 64 / (16 * (float)BAUD_RATE)) + 0.5)
#define MAX_INPUT_LEN 36

// SPI params

// General params
#define CAMERA_TRIGGER_PULSE_WIDTH 100 // us

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void CLOCK_init()
{
	// Set peripheral clock = fclk / 6 = 4MHz
	CLKCTRL.MCLKCTRLB |= CLKCTRL_PDIV_6X_gc | CLKCTRL_PEN_bm;
	
	// Set main clock to internal oscillator @ 24MHz with auto tune enabled
	CLKCTRL.OSCHFCTRLA |= CLKCTRL_FRQSEL_24M_gc | CLKCTRL_AUTOTUNE_bm;
}

// Initialize USART2 in their default pin locations
void USART2_init(uint16_t baud)
{
	PORTF.DIR |= PIN0_bm;
	PORTF.DIR &= ~PIN1_bm;
	
	// Set the baud rate
	USART2.BAUD = uint16_t(USART_BAUD_RATE(baud));
	
	// Enable transmitter and receiver
	USART2.CTRLB |= USART_TXEN_bm | USART_RXEN_bm;
	
	// Set asynchronous, no parity, 8bit data, 1 stop bit
	// This is the default configuration but we set it explicitly for clarity
	USART2.CTRLC |= USART_CMODE0_bm | USART_PMODE0_bm | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc;
}

void SPI0_init()
{
	// Set MOSI, SCK, and SS to output
	PORTA.DIR |= PIN4_bm | PIN6_bm | PIN7_bm;
	
	// Set endianness, master mode, /4 prescale, no clock doubling, SPI enable
	SPI0.CTRLA = SPI_DORD_bm | SPI_MASTER_bm & ~SPI_CLK2X_bm | SPI_PRESC_DIV4_gc | SPI_ENABLE_bm;
}

uint8_t SPI0_write(uint8_t data)
{
	PORTA.OUT &= ~PIN7_bm;
	SPI0.DATA = data;
	
	while (!(SPI0.INTFLAGS & SPI_IF_bm))
	{
		;
	}
	
	PORTA.OUT |= PIN7_bm;
	return SPI0.DATA;
}


uint8_t USART2_readChar()
{
	// Wait for unread data in the receive buffer
	while(!(USART2.STATUS & USART_RXCIF_bm))
	{
		;
	}
	
	return USART2.RXDATAL;
}

void USART2_readString(void)
{
	
}

void USART2_sendChar(char c)
{
	// Wait until transmit data register is empty
	while (!(USART2.STATUS & USART_DREIF_bm))
	{
		;
	}
	
	// Send data
	USART2.TXDATAL = c;
}

void USART2_sendString(char *str)
{
	for(size_t i = 0; i < strlen(str); i++)
	{
		USART2_sendChar(str[i]);
	}
}

void parse(char* cmd)
{
	/* Command format will be [t1, a1, t2, a2, t3, a3, ...] where
	 * tn -> time in microseconds
	 * an -> action to take at that time
	 * times are relative to message receipt
	 */
	if(*cmd != '[')
	{
		USART2_sendString("Bad input!");
		return;
	}
	
	for(;;)
	{
		errno = 0;
		char* end;
		
		// Extract number
		const long i = strtol(cmd, &end, 10);
	}
}


int main(void)
{
	CLOCK_init();
	USART2_init(9600);
	
	// USART message vars
	char input[MAX_INPUT_LEN];
	char c;
	uint8_t index = 0;
	
    while (1) 
    {
		c = USART2_readChar();
		if(c != '\n' && c != '\r')
		{
			input[index++] = c;
	
			// overwrite old data if it overruns the input buffer
			if(index > MAX_INPUT_LEN)
			{
				index = 0;
			}
		}

		if(c == '\n')
		{
			input[index] = '\0';
			index = 0;
			parse(input);
		}
    }
}

