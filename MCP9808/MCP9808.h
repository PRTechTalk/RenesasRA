/*
 * MCP9808.h
 *
 *  Created on: 9 mars 2021
 *      Author: Peter Ranemyr
 *	PR TechTalk
 */

#ifndef MCP9808_H_
#define MCP9808_H_



#endif /* MCP9808_H_ */


#define MCP9809_ADDRESS     0x18
#define MCP9809_TEMP        0x05
#define MCP9809_ID          0x06
#define MCP9809_RESOLUTION  0x08
#define ONE_BYTE            0x01
#define TWO_BYTE            0x02

#define RESET_VALUE         (0x00)

void MCP9808_Init(void);
void MCP9808_GetTemp(void);
void MCP9808_Convert2C(void);
