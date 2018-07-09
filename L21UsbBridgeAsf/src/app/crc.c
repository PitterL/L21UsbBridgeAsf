/*
 * crc.c
 *
 * Created: 6/20/2018 6:57:50 PM
 *  Author: A41450
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "crc.h"

static uint8_t __crc8(uint8_t crc, uint8_t val)
{
	const uint8_t CRC_PLOY = 0x8c;
	uint8_t fb;
	
	for(int i = 0; i < 8; i++) {
		fb = (crc ^ val) & 0x01;
		val >>= 1;
		crc >>= 1;
		if(fb)
			crc ^= CRC_PLOY;
	}

    return crc;
}

uint8_t crc8(const uint8_t *data, uint32_t len)
{
	uint8_t crc = 0;
	
	for(uint32_t i = 0; i < len; i++) {
		crc = __crc8(crc, data[i]);
	}

	return crc;
}

static void __crc24(uint32_t *crc, uint8_t firstbyte, uint8_t secondbyte)
{
	const uint32_t crcpoly = 0x80001B;
	uint32_t result;
	uint32_t data_word;

	data_word = (secondbyte << 8) | firstbyte;
	result = ((*crc << 1) ^ data_word);

	if (result & 0x1000000)
		result ^= crcpoly;

	*crc = result;
}

uint32_t crc24(const uint8_t *data, uint32_t len)
{
	uint32_t crc = 0;
	const uint8_t *ptr = data;
	const uint8_t *last_val = data + len - 1;

	while (ptr < last_val) {
		__crc24(&crc, *ptr, *(ptr + 1));
		ptr += 2;
	}

	/* if len is odd, fill the last byte with 0 */
	if (ptr == last_val)
		__crc24(&crc, *ptr, 0);

	/* Mask to 24-bit */
	crc &= 0x00FFFFFF;

	return crc;
}