/*
 * d21_protocol.h
 *
 * Created: 7/21/2018 11:26:10 AM
 *  Author: A41450
 */ 

#ifndef PROTOCOL_D21_H_
#define PROTOCOL_D21_H_

/*******************************
    D21 extension 
*******************************/
#define CMD_D21_EXTENTION_BULK_DATA 0x61
/*
    <CMD_D21_EXTENTION_BULK_DATA>
        Send bulk command
    Command: 
        DATA1: Sub Command 
            enum CMD_BULK
    Response:
        Depend the sub command
*/
enum CMD_D21_BULK{
   CMD61_OBP = 0x00,
   CMD61_HID,
   CMD61_I2CSCAN,
   CMD61_GETIO,
   CMD61_SETIO,
   CMD61_GetBoardNo = 0x05,
   CMD61_SetBoardNo = 0x06,
   CMD61_T37_OBP,
   CMD61_GetADC,
   CMD61_CalibrateADC,
   CMD61_GetCalibrate,
   CMD61_SetPower,
   MaxItems,
   // Support later.
   CMD61_SETRAMDataOBP,
   CMD61_SETRAMDataHID,
   CMD61_GetFWProfile,       // 8
   CMD61_SetFWProfile,       // 9
   CMD61_SetFWData,          // 10
   CMD61_StartFWUpg,         // 10
   CMD61_IsFrameReady,         // 5
   CMD61_UKNOWN = 0x1F
};
/*
    <CMD61_I2CSCAN>
        Scan IIC device address
    Command: 
        DATA2: Number address given
        DATA[3~n]: address list. The first two bytes is  ADDR_L and ADDR_H for OBP and HID protocol.
    Response:
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: valid address count
        DATA[3-n]: valid address list
*/
/*
    <CMD61_OBP>
        Read/Write IIC data
    Command: 
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: read size LSB
        DATA3: read size MSB
        DATA4: TBD(write size LSB)
        DATA5: TBD(write size MSB)
        DATA6: TBD(mode 0)
        DATA7: addr LSB
        DATA8: addr MSB
    Response:
        DATA0: TBD
        DATA1: TBD
        DATA2: Status
            0xFF: transfer completed
            Other values: TBD (Assumed as IIC_DATA_xxx of CMD_IIC_DATA_1)
        DATA4: data size return 
*/
#define OBP_DATA4_BULK_TRANSFER_COMPLETED 0xff

/*
    <CMD61_HID>
        Read/Write IIC data
    Command: 
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: read size LSB
        DATA3: read size MSB
        DATA4: write size LSB
        DATA5: write size MSB
        DATA6: mode 0
        DATA7: HID out reg LSB
        DATA8: HID out reg MSB
        DATA9: HID in reg LSB
        DATA10: HID in reg MSB
        DATA11: 0x51
        DATA12: addr LSB
        DATA13: addr MSB
    Response:
        TBD
*/
/*
    <CMD61_IsFrameReady>
        Check frame ready
    Command: 
        DATA0: CMD
        DATA1: Sub CMD
    Response:
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: 
            bit[7]: Error occur
            bit[0~6]: TBD
        DATA3:
            0x1: Good
            Other value: Error code
*/
/*
    <CMD61_SetFWProfile>
        Set config
    Command: 
        DATA0: CMD
        DATA1: Sub CMD
    Response:
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: Status
        DATA3: Port Num
        DATA4: DDR
        DATA5: PORT
        DATA6: PIN
*/

/*
    <CMD61_GetADC>
        Get ADC value
            "ADC0", "ADC1", "ADC2", "ADC3", "ADC4", "ADC5",
               "SDA", "SCL", "CHG", "RST", "VDDIO", "3V3", "5V", "USB_IN", "USB_IN2"
    Command: 
        DATA0: CMD
        DATA1: Sub CMD
    Response:
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: 
            Bit[7]: 1 Not supported
            Bit[0~6]: TBD
        DATA3: Num of ADC Pins
        DATA4: TBD
        DATA5: ADC0 value LSB
        DATA6: ADC0 value MSB
        ...
*/

/*
    <CMD61_SetPower>
        Set Power enable
    Command: 
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: Enable
        DATA3: 5V delay(ms), 
        DATA4: 3V3 delay(ms)
        DATA5: VIO delay(ms)
    Response:
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: 
            Bit[7]: 1 Not supported
            Bit[0~6]: TBD
*/

/*
    <CMD61_CalibrateADC>
        Calibrate ADC
    Command: 
        DATA2: PORT
        DATA3: vol_mV byte0
        DATA4: vol_mV byte1
        DATA5: vol_mV byte2
        DATA6: vol_mV byte3
    Response:
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: CMD
        DATA3: Number of ADC ports
        DATA4: Selected Port number
        DATA5: ADC port value (Low)
        DATA6: ADC port value (high)
        DATA7: Voltage_4(mV)
        DATA8: Voltage_3(mV)
        DATA9: Voltage_2(mV)
        DATA10: Voltage_1(mV)
*/
/*
    <CMD61_GetCalibrate>
        Calibrate ADC
    Command: 
        DATA2: PORT
    Response:
        DATA0: CMD
        DATA1: Sub CMD
        DATA2: CMD
        DATA3: Number of ADC ports
        DATA4: Selected Port number
        DATA5: ADC (Low) 2
        DATA6: ADC (Low) 1
        DATA7: ADC (High) 2
        DATA8: ADC (High) 1
        DATA9: Voltage_4(mV)
        DATA10: Voltage_3(mV)
        DATA11: Voltage_2(mV)
        DATA12: Voltage_1(mV)
*/
/*
    <CMD61_GetADC>
        Get ADC
    Command: 
        DATA2: 0
        DATA3: 0
        DATA4: ADCSRA
        DATA5: ADCSRB
        DATA6: ADMUX
        DATA7: TBD
        DATA8: TBD
    Response:
        DATA[0~8]
*/

/*
    <CMD61_GETIO>
        Get IO
    Command: 
        DATA2: 0x1
    Response:
        DATA[0~15]: GPIO value
*/


/*
    <CMD61_GETIO>
        Get IO
    Command: 
        DATA2: 0x1
    Response:
        DATA[0~15]: GPIO value
*/
/*
    <CMD61_SETIO>
        Set IO
        Same as CMD_SET_GPIOS
*/

int32_t d21_parse_command(void *host, const uint8_t *data, uint32_t count);
int32_t d21_init(void);
void d21_deinit(void);

#endif