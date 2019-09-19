/*
 * hiddf_intf.h
 *
 * Created: 7/21/2018 10:39:20 AM
 *  Author: A41450
 */ 


#ifndef HIDDF_INTF_H_
#define HIDDF_INTF_H_

typedef bool (*cb_hiddf_func_data_send)(uint8_t *);

void hiddf_intf_receive(const uint8_t *data, uint32_t size);
void hiddf_intf_send(cb_hiddf_func_data_send cb_send);
int32_t hiddf_intf_init(void);
void hiddf_intf_deinit(void);

#endif