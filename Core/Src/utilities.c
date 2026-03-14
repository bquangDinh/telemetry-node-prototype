/*
 * utilities.c
 *
 *  Created on: Mar 13, 2026
 *      Author: buiqu
 */
#include <string.h>

#include "usbd_cdc_if.h"

#include "utilities.h"

void usb_printf(const char *s) {
	while (CDC_Transmit_FS((uint8_t*) s, strlen(s)) == USBD_BUSY) {
	}
}
