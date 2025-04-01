#include "Logic/fletcher.h"

/**
 * @brief Compute 16bit fletcher checksum. Since max(length)=10.
 *        f2 < 2^16, modulu can be executed at the end of summation.
 *
 * @param array
 * @param length
 * @return fletcher checksum concatenated into 16 bits
 */
uint16_t fletcher_get(const uint8_t* array, uint16_t length)
{
	uint16_t f0 = 0;
	uint16_t f1 = 0;
	int index;

	for (index = 0; index < length; index++) {
		f0 += array[index];
		f1 += f0;
	}

	f0 = f0 % 255;
	f1 = f1 % 255;

	return (uint16_t) ((f1 << 8) | f0);
}