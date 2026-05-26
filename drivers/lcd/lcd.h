#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);

void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);
void LCD_Message(const char *line1, const char *line2);

#endif