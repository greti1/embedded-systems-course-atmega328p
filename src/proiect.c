#include "proiect.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "drivers/gpio/gpio.h"
#include "drivers/adc/adc.h"
#include "drivers/lcd/lcd.h"

#define GREEN_LED   4   // PD4
#define RED_LED     6   // PD6
#define YELLOW_LED  7   // PD7
#define BUZZER      0   // PB0
#define FAN         1   // PB1

#define LIGHT_ON_LIMIT   500
#define LIGHT_OFF_LIMIT  500

static uint16_t read_light(void)
{
    uint32_t sum = 0;

    for (int i = 0; i < 30; i++)
    {
        sum += ADC_Read(0);
        _delay_ms(2);
    }

    return sum / 30;
}

static void gpio_init_project(void)
{
    GPIO_Init(GPIO_PORTD, GREEN_LED, GPIO_OUTPUT);
    GPIO_Init(GPIO_PORTD, RED_LED, GPIO_OUTPUT);
    GPIO_Init(GPIO_PORTD, YELLOW_LED, GPIO_OUTPUT);

    GPIO_Init(GPIO_PORTB, BUZZER, GPIO_OUTPUT);
    GPIO_Init(GPIO_PORTB, FAN, GPIO_OUTPUT);
}

static void all_off(void)
{
    GPIO_Write(GPIO_PORTD, GREEN_LED, GPIO_LOW);
    GPIO_Write(GPIO_PORTD, RED_LED, GPIO_LOW);
    GPIO_Write(GPIO_PORTD, YELLOW_LED, GPIO_LOW);

    GPIO_Write(GPIO_PORTB, BUZZER, GPIO_LOW);
    GPIO_Write(GPIO_PORTB, FAN, GPIO_LOW);
}

static void buzzer_soft_beep(void)
{
    GPIO_Write(GPIO_PORTB, BUZZER, GPIO_HIGH);
    _delay_ms(50);
    GPIO_Write(GPIO_PORTB, BUZZER, GPIO_LOW);
    _delay_ms(150);
}

void Proiect_Init(void)
{
    gpio_init_project();
    ADC_Init();
    LCD_Init();
}

void Proiect_Run(void)
{
    while (1)
    {
        uint16_t light = read_light();

        if (light > LIGHT_ON_LIMIT)
        {
            all_off();

            GPIO_Write(GPIO_PORTD, GREEN_LED, GPIO_HIGH);

            LCD_Message("Lumina aprinsa",
                        "Sistem normal");
        }
        else
        {
            all_off();

            GPIO_Write(GPIO_PORTD, RED_LED, GPIO_HIGH);

            LCD_Message("Lumina stinsa",
                        "Asteapta 5 sec");

            for (int i = 0; i < 25; i++)
            {
                light = read_light();

                if (light > LIGHT_ON_LIMIT)
                {
                    break;
                }

                buzzer_soft_beep();
            }

            light = read_light();

            if (light <= LIGHT_OFF_LIMIT)
            {
                all_off();

                GPIO_Write(GPIO_PORTD, YELLOW_LED, GPIO_HIGH);
                GPIO_Write(GPIO_PORTB, FAN, GPIO_HIGH);
                GPIO_Write(GPIO_PORTB, BUZZER, GPIO_LOW);

                LCD_Message("Ventilator ON",
                            "Lumina stinsa");

                while (read_light() <= LIGHT_OFF_LIMIT)
                {
                    GPIO_Write(GPIO_PORTD, YELLOW_LED, GPIO_HIGH);
                    GPIO_Write(GPIO_PORTB, FAN, GPIO_HIGH);
                    GPIO_Write(GPIO_PORTB, BUZZER, GPIO_LOW);

                    _delay_ms(200);
                }
            }
        }

        _delay_ms(100);
    }
}