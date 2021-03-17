/*
 * MCP9808.c
 *  *
 *  Created on: 9 mars 2021
 *      Author: Peter Ranemyr
 *      PR TechTalk
 */
#include "hal_data.h"
#include  "MCP9808.h"

float Temperature;
uint8_t UpperByte;
uint8_t LowerByte;
uint8_t data[3] = {0x00};
static volatile i2c_master_event_t g_master_event = (i2c_master_event_t)RESET_VALUE;


/* Callback function */
void sci_i2c_master_callback(i2c_master_callback_args_t *p_args)
{
    /* TODO: add your own code here */
    if (NULL != p_args)
        {
            g_master_event = p_args->event;

        }
}




void MCP9808_Init(void){
    R_SCI_I2C_Open(&g_i2c0_ctrl, &g_i2c0_cfg);
    R_BSP_SoftwareDelay(5,BSP_DELAY_UNITS_MILLISECONDS);
    data[0] = MCP9809_TEMP;
    R_SCI_I2C_Write(&g_i2c0_ctrl, &data[0], ONE_BYTE, false);
    R_BSP_SoftwareDelay(200,BSP_DELAY_UNITS_MILLISECONDS);
}


void MCP9808_GetTemp(void){
    R_SCI_I2C_Read(&g_i2c0_ctrl, &data[0], TWO_BYTE, false);

    /* Wait until slave write and master read process gets completed */
    while(I2C_MASTER_EVENT_RX_COMPLETE != g_master_event){}

    MCP9808_Convert2C();
}


void MCP9808_Convert2C(void){
    UpperByte = data[0];
    LowerByte = data[1];

    // Convert data to degres Celsius
    UpperByte= UpperByte & 0x1f;                                    // Clear flag bits
    if ((UpperByte & 0x10) == 0x10) {                               // TA < 0°C
        UpperByte = UpperByte & 0x0f;                               // Clear Sign
        Temperature = (256.0 - (UpperByte * 16.0 + LowerByte / 16.0));
    } else {
        Temperature = (UpperByte * 16.0 + LowerByte / 16.0);        // TA > 0°C
    }
}

