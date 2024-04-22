/*
 * led-controller.cpp
 *
 * Created: 16/04/2024 7:16:16 PM
 * Author : jmcna
 */ 

// Clock params
#define F_CPU 8000000UL

// USART 2 params
#define USART_BAUD_RATE(BAUD_RATE) ((float)(4000000 * 64 / (16 * (float)BAUD_RATE)) + 0.5)
#define MAX_INPUT_LEN 128
#define MAX_NUM_COMMANDS 32
#define MAX_NUM_INTS 3*MAX_NUM_COMMANDS

// SPI params

// General params
#define CAMERA_TRIGGER_PULSE_WIDTH 100 // us

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

// These are the specific actions that
enum Action
{
	WRITE_DAC_A,
	WRITE_DAC_B,
	LED_SHORT_ON,
	LED_SHORT_OFF,
	LED_ON,
	LED_OFF,
	LED_WHITE_ON,
	LED_WHITE_OFF,
	TRIGGER_HIGH,
	TRIGGER_LOW
};

struct Command
{
	int action;
	uint32_t time;
	uint16_t value;
};


void writeDACn(uint16_t n, uint16_t value)
{
	value = value > 4096 ? 4096 : value;

	// Add configuration bits for DAC selection (a/b) and normal operation
	value |= (n >= 0x01) << 15 | 0x01 << 13 | 0x01 << 12;

	// Send SPI
}

void shutdownDACn(uint16_t n)
{
	// Add configuration bits for DAC selection (a/b) and the shutdown bit
	uint16_t value = (n >= 0x01) << 15 & ~(0x01 << 12);

	value++;
}

void CLOCK_init()
{
	_PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_8M_gc);
	//_PROTECTED_WRITE(CLKCTRL.XOSC32KCTRLA,  CLKCTRL_ENABLE_bm);
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PEN_bm);
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSCHF_gc);
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
	USART2.CTRLC |= USART_PMODE0_bm | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc;
}

void SPI0_init()
{
	// Set MOSI, SCK, and SS to output
	PORTA.DIR |= PIN4_bm | PIN6_bm | PIN7_bm;
	
	// Set endianness, master mode, /4 prescale, no clock doubling, SPI enable
	SPI0.CTRLA = SPI_DORD_bm | SPI_MASTER_bm & (~SPI_CLK2X_bm) | SPI_PRESC_DIV4_gc | SPI_ENABLE_bm;
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

void USART2_sendString(char *str)
{
	for(size_t i = 0; i < strlen(str); i++)
	{
		USART2_sendChar(str[i]);
	}
}

void USART2_readString(char* str)
{
	USART2_sendString("Started reading\n");
	char c;
	uint8_t index = 0;

	while (1)
	{
		c = USART2_readChar();
		if(c != '\n' && c != '\r')
		{
			str[index++] = c;
		
			// overwrite old data if it overruns the input buffer
			if(index > MAX_INPUT_LEN)
			{
				index = 0;
			}
		}

		if(c == '\n')
		{
			str[index] = '\0';
			index = 0;
			USART2_sendString("char!\n");
			return;
		}
	}
}



bool parse(const char* c, char** end, Command* cmds, size_t& idx)
{
	//printf("Parsing '%s':\n", c);
	uint32_t buffer[MAX_NUM_INTS];
	uint8_t n = 0;

	USART2_sendString("Started parsing\n");
	while (n < MAX_NUM_INTS)
	{
		// Reset errno, as its used by many std functions
		errno = 0;

		// Attempt to extract a long from the string
		const uint32_t val = strtoul(c, end, 10);

		// Check if there was a range error
		const bool range_error = errno == ERANGE;

		// Conversion failed, stop
		if (c == *end || range_error)
		break;

		// Update the start of the string to where the last number extracted finished
		c = *end;

		// Add the new number to the buffer
		buffer[n] = val;

		// Increment counter
		n++;
	}

	// Need multiple of 3 to make valid commands
	if (n % 3 != 0)
	{
		return false;
	}

	//
	idx = n / 3;
	for(int i = 0; i < n; i += 3)
	{
		cmds[i/3] = {int(buffer[i]), buffer[i+1], uint16_t(buffer[i+2]) };
	}

	return true;
}


int main(void)
{
	CLOCK_init();
	USART2_init(9600);
			
	char buffer[MAX_INPUT_LEN];
	Command commands[MAX_NUM_COMMANDS];

	_delay_ms(30);
	USART2_sendString("Rebooted\n");

	while (1)
	{
		// Wait for a line
		USART2_readString(buffer);

		char* end;
		size_t idx = 0;
		if ( parse(buffer, &end, commands, idx) )
		{
			USART2_sendString("Good parse!\n");
		}
		else
		{
			USART2_sendString("Bad parse!\n");
			USART2_sendString(end);
		}
	}
}

