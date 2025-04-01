#include <Arduino.h>
#include "POPB/button.h"

void button_init()
{
	uint8_t temp;
	temp = DDRD;
	temp &= ~(1 << 5);
	DDRD = temp;
}

uint8_t ReadButton()
{
	uint8_t button;
	button = PIND & (1 << 5);
	return button;
}