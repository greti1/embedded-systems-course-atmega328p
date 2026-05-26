#include "lcd.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define LCD_ADDR 0x27

#define LCD_ENABLE 0x04
#define LCD_BACKLIGHT 0x08
#define LCD_RS 0x01

// ---------- I2C ----------

static void I2C_Init(void)
{
    TWSR = 0;

    // SCL ≈100kHz la F_CPU =16MHz
    TWBR = 72;

    TWCR = (1 << TWEN);
}

static void I2C_Start(void)
{
    TWCR = (1 << TWINT) |
           (1 << TWSTA) |
           (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

static void I2C_Stop(void)
{
    TWCR = (1 << TWINT) |
           (1 << TWSTO) |
           (1 << TWEN);
}

static void I2C_Write(uint8_t data)
{
    TWDR = data;

    TWCR = (1 << TWINT) |
           (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

// ---------- LCD intern ----------

static void LCD_SendNibble(uint8_t data)
{
    I2C_Start();

    I2C_Write(LCD_ADDR << 1);

    I2C_Write(data | LCD_ENABLE | LCD_BACKLIGHT);

    _delay_us(1);

    I2C_Write((data & ~LCD_ENABLE) | LCD_BACKLIGHT);

    I2C_Stop();

    _delay_us(50);
}

static void LCD_SendByte(uint8_t data, uint8_t mode)
{
    LCD_SendNibble((data & 0xF0) | mode);

    LCD_SendNibble(((data << 4) & 0xF0) | mode);
}

// ---------- API ----------

void LCD_Command(uint8_t cmd)
{
    LCD_SendByte(cmd, 0);
}

void LCD_Data(uint8_t data)
{
    LCD_SendByte(data, LCD_RS);
}

void LCD_Init(void)
{
    I2C_Init();

    _delay_ms(50);

    LCD_SendNibble(0x30);
    _delay_ms(5);

    LCD_SendNibble(0x30);
    _delay_us(150);

    LCD_SendNibble(0x30);

    LCD_SendNibble(0x20);

    LCD_Command(0x28);

    // display ON
    LCD_Command(0x0C);

    // cursor move right
    LCD_Command(0x06);

    // clear display
    LCD_Command(0x01);

    _delay_ms(2);
}

void LCD_Print(const char *str)
{
    while (*str)
    {
        LCD_Data(*str);
        str++;
    }
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    if (row == 0)
    {
        LCD_Command(0x80 + col);
    }
    else
    {
        LCD_Command(0xC0 + col);
    }
}

void LCD_Message(const char *line1,
                 const char *line2)
{
    LCD_SetCursor(0,0);
    LCD_Print("                ");

    LCD_SetCursor(1,0);
    LCD_Print("                ");

    LCD_SetCursor(0,0);
    LCD_Print(line1);

    LCD_SetCursor(1,0);
    LCD_Print(line2);
}