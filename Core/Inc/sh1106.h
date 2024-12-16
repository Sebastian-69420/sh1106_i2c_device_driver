/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SH1106_H
#define __SH1106_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include <stdint.h>
#include <string.h>
#include "../Font/minecraft_condensed.h"

/* Defines -------------------------------------------------------------------*/
#define SH1106_WIDTH 132
#define SH1106_HEIGHT 64
#define SH1106_NUM_PAGES 8

#define SH1106_COMMAND 0x00
#define SH1106_DATA 0x40

#define DISPLAY_OFF 0xAE
#define DISPLAY_ON_NORMAL_MODE 0xAF
#define DISPLAY_ON_RAM_CONTENT 0xA4
#define DISPLAY_ON_RAM_IGNORE 0xA5

#define SET_DISPLAY_NORMAL 0xA6
#define SET_DISPLAY_INVERSE 0xA7

#define SET_CONTRAST_CONTROL 0x81

#define SET_MEMORY_ADDRESSING_MODE 0x20

#define SET_DISPLAY_START_LINE 0x40 // Allowed Values: 0x40 to 0x7F, Reset Value: 0x40 / x100 0000

#define SET_PAGE_ADDRESS_FOR_PAGE_ADDRESSING_MODE 0xB0
#define SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL_MODE 0xC0
#define SET_COM_OUTPUT_SCAN_DIRECTION_REMAPPED_MODE 0xC8

#define SET_SEGMENT_REMAP_SEG0_0 0xA0
#define SET_SEGMENT_REMAP_SEG0_131 0xA1

#define SET_MULTIPLEX_RATIO 0xA8
#define SET_DISPLAY_OFFSET 0xD3

#define SET_DISPLAY_CLOCK_DIVIDE_RATIO 0xD5
#define SET_PRECHARGE_PERIOD 0xD9

#define SET_COM_PINS_HARDWARE_CONFIGURATION 0xDA
#define SET_VCOMH_DESELECT_LEVEL 0xDB

#define CHARGE_PUMP_SETTING 0x8D

#define DEACTIVATE_SCROLL 0x2E

/* Variables -----------------------------------------------------------------*/
extern I2C_HandleTypeDef *HI2C;
extern uint8_t DEVICE_ADDRESS;

extern uint8_t POS_X;
extern uint8_t POS_Y;

/* Macros --------------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
void SH1106_Init(I2C_HandleTypeDef *hi2c, uint8_t deviceAddress);

void SH1106_Display_On();
void SH1106_Display_All_On();
void SH1106_Display_Off();
void SH1106_Display_Clear();

void SH1106_Set_Cursor(uint8_t page, uint8_t column);
void SH1106_Set_Page_Address(uint8_t page);
void SH1106_Set_Column_Address(uint8_t column);
void SH1106_Set_Page_And_Column_Address(uint8_t page, uint8_t column);

void SH1106_Transmit_Data(uint8_t data);
void SH1106_Transmit_Command(uint8_t command);

void SH1106_Display_Character(uint8_t character);
void SH1106_Print_Line(char* string);
void SH1106_Display_String(char* string);
void SH1106_Display_Text(char* string);

void SH1106_Set_Pixel(uint8_t x, uint8_t y);

#ifdef __cplusplus
}
#endif

#endif /* __SH1106_H */
