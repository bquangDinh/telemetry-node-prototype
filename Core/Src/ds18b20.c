/*
 * ds18b20.c
 *
 *  Created on: Mar 12, 2026
 *      Author: buiqu
 */
#include <string.h>
#include <stdio.h>

#include "stm32f1xx_hal.h"

#include "ds18b20.h"
#include "can.h"
#include "utilities.h"

#define DS18B20_DAT_GPIO_PORT GPIOB
#define DS18B20_DAT_GPIO_PIN GPIO_PIN_0
#define DS18B20_TIMER (&htim2)

#define DS18B20_MAX_SENSORS 4

#define DS18B20_SEARCH_ROM_CMD 0xF0
#define DS18B20_MATCH_ROM_CMD 0x55
#define DS18B20_CONVERT_T_CMD 0x44
#define DS18B20_READ_POWER_SUPPLY_CMD 0xB4
#define DS18B20_READ_SCRATCHPAD_CMD 0xBE

typedef enum {
	DS18B20_DEVICE_PRESENCE, DS18B20_DEVICE_NOT_PRESENCE
} device_presence_t;

extern TIM_HandleTypeDef htim2;

static uint64_t rom_list[DS18B20_MAX_SENSORS];

static uint64_t rom_count = 0;

/* States for search ROM command */
static uint16_t last_conflict_bit = 0;
static uint8_t last_device_flag = 0;
static uint8_t rom[8];

static void delay_us(uint16_t us);

// Pull the line low
static void ds18b20_pull_low();

// Release the line
static void ds18b20_release();

// Read the line
// return 0 if the line is pull low
// return 1 if the line stays high
static uint8_t ds18b20_read_bit();

static uint8_t ds18b20_read_byte();

static void ds18b20_write_bit(uint8_t val);

static void ds18b20_write_byte(uint8_t byte);

// return 0 if there is no devices presence
// return 1 otherwises
static device_presence_t reset();

static void perform_search();

static uint8_t search(uint8_t new_addr[8]);

static void read_temp_all();

/**
 * Send the given ROM value to the bus
 */
static void send_rom(const uint64_t rom);

/**
 * Check if the given sensor is running on parasite mode or external power supply mode
 * @return: 0 if parasite, 1 if external power supply, 0xFF if error occurred during the reading
 */
static uint8_t ds18b20_is_power_external(uint64_t rom);

static void rom_to_string(uint8_t rom[8], char *out);

static void rom64_to_string(uint64_t rom, char *out);

static uint8_t ds18b20_crc8(const uint8_t *data, uint8_t len);

void ds18b20_init() {
	uart_print("Checking if DS18B20 sensor is presence!\r\n");

	HAL_TIM_Base_Stop(DS18B20_TIMER);

	HAL_TIM_Base_Start(DS18B20_TIMER);

	if (reset() == DS18B20_DEVICE_PRESENCE) {
		uart_print("DS18B20 sensor is presence!\r\n");
	} else {
		uart_print("DS18B20 sensor is not presence!\r\n");
		return;
	}

	perform_search();
}

void ds18b20_update() {
	read_temp_all();
}

static void delay_us(uint16_t us) {
	uint16_t start = __HAL_TIM_GET_COUNTER(DS18B20_TIMER);

	while ((uint16_t) (__HAL_TIM_GET_COUNTER(DS18B20_TIMER) - start) < us) {
	}
}

// Pull the line low
static void ds18b20_pull_low() {
	HAL_GPIO_WritePin(DS18B20_DAT_GPIO_PORT, DS18B20_DAT_GPIO_PIN,
			GPIO_PIN_RESET);
}

// Release the line
static void ds18b20_release() {
	HAL_GPIO_WritePin(DS18B20_DAT_GPIO_PORT, DS18B20_DAT_GPIO_PIN,
			GPIO_PIN_SET);
}

static uint8_t ds18b20_read_bit() {
	ds18b20_pull_low();

	delay_us(3);

	ds18b20_release();

	delay_us(10);

	GPIO_PinState ret = HAL_GPIO_ReadPin(DS18B20_DAT_GPIO_PORT,
	DS18B20_DAT_GPIO_PIN);

	delay_us(50);

	if (ret == GPIO_PIN_RESET)
		return 0;

	return 1;
}

static uint8_t ds18b20_read_byte() {
	uint8_t data = 0;

	for (uint8_t i = 0; i < 8; ++i) {
		data >>= 1;

		if (ds18b20_read_bit()) {
			data |= 0x80;
		}
	}

	return data;
}

static void ds18b20_write_bit(uint8_t val) {
	ds18b20_pull_low();

	if (val == 0) {
		delay_us(60);

		ds18b20_release();

		// at least is 1us, 10us is a safe value
		delay_us(10);

		return;
	}

	// It should be noted that the line should be high "immediately" after pulled low
	// within 15us, so 6 is a safe value
	delay_us(6);

	ds18b20_release();

	// The sample window is 60us, so 64us also count for waiting for the line to recover
	delay_us(64);

	return;
}

static void ds18b20_write_byte(uint8_t byte) {
	for (uint8_t i = 0; i < 8; ++i) {
		ds18b20_write_bit(byte & 1);
		byte >>= 1;
	}
}

static device_presence_t reset() {
	ds18b20_pull_low();

	delay_us(480);

	ds18b20_release();

	delay_us(70);

	GPIO_PinState ret = HAL_GPIO_ReadPin(DS18B20_DAT_GPIO_PORT,
	DS18B20_DAT_GPIO_PIN);

	delay_us(410);

	if (ret == GPIO_PIN_RESET) {
		// There is some devices pulling the line low
		// mean device presence
		return DS18B20_DEVICE_PRESENCE;
	}

	return DS18B20_DEVICE_NOT_PRESENCE;
}

static void perform_search() {
	uart_print("Perform search all DS18B20 sensors\r\n");

	// Reset search state
	last_conflict_bit = 0;
	last_device_flag = 0;

	for (uint8_t i = 0; i < 8; ++i)
		rom[i] = 0;

	uint8_t device_addr[8];
	uint8_t sensor_num = 0;
	char rom_str[24];
	char out_msg[256];

	while (search(device_addr) && rom_count < DS18B20_MAX_SENSORS) {
		rom_to_string(device_addr, rom_str);

		sprintf(out_msg, "[DS18B20] Sensor %d ROM: %s\r\n", sensor_num,
				rom_str);

		uart_print(out_msg);

		sensor_num++;

		// Add to ROM list
		if (rom_count < DS18B20_MAX_SENSORS) {
			for (uint8_t i = 0; i < 8; ++i) {
				rom_list[rom_count] |= ((uint64_t) device_addr[i]) << (8 * i);
			}

			rom_count++;

			uart_print("[DS18B20] Added sensor to list\r\n");
		} else {
			uart_print("[DS18B20] Maximum sensors reached\r\n");
		}
	}

	uart_print("Search done\r\n");
}

static uint8_t search(uint8_t new_addr[8]) {
	uint8_t bit_idx = 1;
	uint8_t last_zero = 0;
	uint8_t rom_byte_idx = 0;
	uint8_t rom_byte_mask = 1;
	uint8_t search_direction = 0;
	uint8_t first_read_bit, second_read_bit;

	if (last_device_flag) {
		uart_print("[DS18B20] Search - last device flag set\r\n");
		return 0;
	}

	if (reset() == DS18B20_DEVICE_NOT_PRESENCE) {
		uart_print(
				"[DS18B20] Search - reset failed due to no device presence on the bus\r\n");
		return 0;
	}

	ds18b20_write_byte(DS18B20_SEARCH_ROM_CMD);

	do {
		first_read_bit = ds18b20_read_bit();
		second_read_bit = ds18b20_read_bit();

		if (first_read_bit == 1 && second_read_bit == 1) {
			break; // no devices
		}

		if (first_read_bit != second_read_bit) {
			// no conflict
			search_direction = first_read_bit;
		} else {
			// conflict
			// some devices have 0, some have 1
			if (bit_idx < last_conflict_bit) {
				// Just replay the whole thing again since we haven't reached the last_conflict_bit yet
				search_direction = (rom[rom_byte_idx] & rom_byte_mask);
			} else {
				// Since we have chosen 0 (always prefer choosing 0 first when there is a conflict) in the previous pass, we choose 1 in the next pass
				// If bit_idx > last_conflict_bit (for simplicity, this is the first conflict happens), it would choose zero since the comparation returns false (0)
				search_direction = (bit_idx == last_conflict_bit);
			}

			if (search_direction == 0)
				last_zero = bit_idx;
		}

		if (search_direction == 1) {
			rom[rom_byte_idx] |= rom_byte_mask;
		} else {
			rom[rom_byte_idx] &= ~rom_byte_mask;
		}

		ds18b20_write_bit(search_direction);

		bit_idx++;
		rom_byte_mask <<= 1;

		if (rom_byte_mask == 0) {
			rom_byte_idx++;
			rom_byte_mask = 1;
		}
	} while (rom_byte_idx < 8);

	if (bit_idx > 64) {
		last_conflict_bit = last_zero;

		if (last_conflict_bit == 0) {
			last_device_flag = 1;
		}

		for (uint8_t i = 0; i < 8; ++i) {
			new_addr[i] = rom[i];
		}

		return 1;
	}

	return 0;
}

static void read_temp_all() {
	uint8_t i = 0;
	uint8_t scratchpad[9]; // 8 data bytes + CRC byte
	uint8_t computed_crc;
	int16_t raw_temp;
	int temp;
	char out_msg[24];
	char rom_str[24];

	uint32_t base_can_id = 0x1;
	uint8_t can_data[8];

	while (i < DS18B20_MAX_SENSORS) {
		if (rom_list[i] == 0) {
			i++;
			continue;
		}

		if (!ds18b20_is_power_external(rom_list[i])) {
			i++;

			// Do not support reading temp in parasite mode
			continue;
		}

		// First, reset
		if (reset() == DS18B20_DEVICE_NOT_PRESENCE) {
			uart_print(
					"[DS18B20] Reading temp failed due to no device presence on the bus\r\n");
			break;
		}

		// Issue a MATCH ROM command
		ds18b20_write_byte(DS18B20_MATCH_ROM_CMD);

		// Follow by ROM value
		send_rom(rom_list[i]);

		// Issue Convert T command
		ds18b20_write_byte(DS18B20_CONVERT_T_CMD);

		// wait until conversion is done
		HAL_Delay(750);

		// reset again
		if (reset() == DS18B20_DEVICE_NOT_PRESENCE) {
			uart_print(
					"[DS18B20] Reading temp failed due to no device presence on the bus\r\n");
			break;
		}

		// Issue a MATCH ROM command
		ds18b20_write_byte(DS18B20_MATCH_ROM_CMD);

		// Follow by ROM value
		send_rom(rom_list[i]);

		// Issue Read ScratchPad command
		ds18b20_write_byte(DS18B20_READ_SCRATCHPAD_CMD);

		// Read 9 bytes
		for (uint8_t j = 0; j < 9; ++j) {
			scratchpad[j] = ds18b20_read_byte();
		}

		// Compute CRC and check if they match
		// If they are not match, mean the data is wrong
		// Do it again
		computed_crc = ds18b20_crc8(scratchpad, 8);

		if (computed_crc == scratchpad[8]) {
			// data is good
			// read temp
			raw_temp = (int16_t) (((uint16_t) scratchpad[1] << 8) | scratchpad[0]);

			temp = (raw_temp * 100) / 16;

			can_data[0] = scratchpad[0];
			can_data[1] = scratchpad[1];

			can_send(base_can_id + i, can_data);

			rom64_to_string(rom_list[i], rom_str);

			uart_print("[DS18B20] ");

			uart_print(rom_str);

			sprintf(out_msg, " %d C\r\n", temp);

			uart_print(out_msg);

			i++;
		} else {
			uart_print("Data corrupted. Try again...\r\n");
		}
	}
}

static void send_rom(const uint64_t rom) {
	for (uint8_t i = 0; i < 8; ++i) {
		uint8_t byte = (rom >> (8 * i)) & 0xFF;
		ds18b20_write_byte(byte);
	}
}

static uint8_t ds18b20_is_power_external(uint64_t rom) {
	if (reset() == DS18B20_DEVICE_NOT_PRESENCE) {
		return 0xFF;
	}

	ds18b20_write_byte(DS18B20_MATCH_ROM_CMD);

	send_rom(rom);

	ds18b20_write_byte(DS18B20_READ_POWER_SUPPLY_CMD);

	return ds18b20_read_bit();
}

static void rom_to_string(uint8_t rom[8], char *out) {
	const char hex[] = "0123456789ABCDEF";

	for (int i = 0; i < 8; i++) {
		out[i * 3] = hex[(rom[i] >> 4) & 0xF];
		out[i * 3 + 1] = hex[(rom[i]) & 0xF];
		out[i * 3 + 2] = ' ';
	}

	out[23] = '\0';
}

static void rom64_to_string(uint64_t rom, char *out) {
	const char hex[] = "0123456789ABCDEF";

	for (int i = 0; i < 8; i++) {
		uint8_t byte = (rom >> (8 * i)) & 0xFF;

		out[i * 3] = hex[(byte >> 4) & 0x0F];
		out[i * 3 + 1] = hex[byte & 0x0F];
		out[i * 3 + 2] = ' ';
	}

	out[23] = '\0';
}

static uint8_t ds18b20_crc8(const uint8_t *data, uint8_t len) {
	uint8_t crc = 0;

	for (uint8_t i = 0; i < len; i++) {
		uint8_t inbyte = data[i];

		for (uint8_t j = 0; j < 8; j++) {
			uint8_t input_bit = inbyte & 0x01;
			uint8_t crc_bit = crc & 0x01;
			uint8_t mix = input_bit ^ crc_bit;

			crc >>= 1;

			if (mix)
				crc ^= 0x8C;

			inbyte >>= 1;
		}
	}

	return crc;
}
