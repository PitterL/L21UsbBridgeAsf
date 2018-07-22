/*
 * common.c
 *
 * Created: 7/21/2018 11:25:03 AM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//#include "../driver_init.h"
#include "crc.h"
#include "board/board.h"
#include "common.h"
#include "external/utils.h"
#include "external/err_codes.h"

uint8_t i2c_transfer_data(void *host, const uint8_t *wdata, uint32_t wlen, uint8_t *rdata, uint32_t rlen, uint8_t *ecode, int32_t retry)
{
	controller_t *hc = (controller_t *)host;
	uint8_t cmd_rsp;
	uint8_t len_rsp;
	int32_t ret;
	
	do {
		cmd_rsp = IIC_DATA_OK;
		len_rsp = 0;
		ret = ERR_NONE;
		
		//write
		if (wlen) {
			ret = iic_write(&hc->iic, wdata, wlen);
			if (ret == ERR_BAD_ADDRESS) {
				cmd_rsp = IIC_DATA_NAK_ADDR;
			}else if(ret) {
				cmd_rsp = IIC_DATA_NAK_WRITE;
			}
		}

		//read
		if (ret == ERR_NONE) {
			if (rlen) {
				ret = iic_read(&hc->iic, rdata, rlen);
				if (ret == ERR_BAD_ADDRESS) {
					cmd_rsp = IIC_DATA_NAK_ADDR;
				}else if(ret) {
					cmd_rsp = IIC_DATA_NAK_READ;
				}else {
					len_rsp = rlen;
				}
			}else {
				cmd_rsp = IIC_DATA_FINISHED_WITHOUT_R;
			}
		}

		if (ret == ERR_NONE)
			break;
		retry--;
		//TBD: here need a retry delay

	}while(ret != ERR_NONE && retry > 0);

	if (ecode)
		*ecode = cmd_rsp;

	return len_rsp;
}
