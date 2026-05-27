#include "proiect.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

#include "drivers/gpio/gpio.h"
#include "drivers/adc/adc.h"
#include "drivers/lcd/lcd.h"
#include "drivers/usart/usart.h"

#define GREEN_LED   4
#define RED_LED     6
#define YELLOW_LED  7
#define BUZZER      0
#define FAN         1

#define LIGHT_ON_LIMIT   500
#define LIGHT_OFF_LIMIT  500

static uint8_t manual_fan = 0;

static void send_text(const char *text)
{
    USART_Transmit((void *)text, strlen(text));
}

static void check_python_command(void)
{
    if (UCSR0A & (1 << RXC0))
    {
        char c = UDR0;

        if (c == '1')
        {
            manual_fan = 1;

            GPIO_Write(GPIO_PORTB, FAN, GPIO_HIGH);

            send_text("FAN_MANUAL_ON\n");
        }
        else if (c == '0')
        {
            manual_fan = 0;

            GPIO_Write(GPIO_PORTB, FAN, GPIO_LOW);

            send_text("FAN_MANUAL_OFF\n");
        }
    }
}

static uint16_t read_light(void)
{
    uint32_t sum = 0;

    for (int i = 0; i < 30; i++)
    {
        sum += ADC_Read(0);

        _delay_ms(2);

        check_python_command();
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

    if (manual_fan == 0)
    {
        GPIO_Write(GPIO_PORTB, FAN, GPIO_LOW);
    }
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

    USART_Init(16000000, 57600);

    send_text("SYSTEM_READY\n");
}

void Proiect_Run(void)
{
    while (1)
    {
        check_python_command();

        uint16_t light = read_light();

        // ---------- LIGHT ON ----------
        if (light > LIGHT_ON_LIMIT)
        {
            all_off();

            GPIO_Write(GPIO_PORTD, GREEN_LED, GPIO_HIGH);

            if (manual_fan)
            {
                GPIO_Write(GPIO_PORTB, FAN, GPIO_HIGH);

                LCD_Message("Light ON",
                            "Fan manual ON");

                send_text("FAN_MANUAL_ON\n");
            }
            else
            {
                LCD_Message("Light ON",
                            "System normal");

                send_text("FAN_OFF\n");
            }

            send_text("LIGHT_ON\n");
        }

        // ---------- LIGHT OFF ----------
        else
        {
            all_off();

            GPIO_Write(GPIO_PORTD, RED_LED, GPIO_HIGH);

            LCD_Message("Light OFF",
                        "Wait 5 sec");

            send_text("LIGHT_OFF\n");

            for (int sec = 5; sec >= 1; sec--)
            {
                light = read_light();

                if (light > LIGHT_ON_LIMIT)
                {
                    break;
                }

                if (sec == 5) send_text("COUNTDOWN:5\n");
                if (sec == 4) send_text("COUNTDOWN:4\n");
                if (sec == 3) send_text("COUNTDOWN:3\n");
                if (sec == 2) send_text("COUNTDOWN:2\n");
                if (sec == 1) send_text("COUNTDOWN:1\n");

                for (int i = 0; i < 5; i++)
                {
                    buzzer_soft_beep();
                }
            }

            light = read_light();

            // ---------- FAN AUTO ----------
            if (light <= LIGHT_OFF_LIMIT)
            {
                all_off();

                GPIO_Write(GPIO_PORTD, YELLOW_LED, GPIO_HIGH);

                GPIO_Write(GPIO_PORTB, FAN, GPIO_HIGH);

                GPIO_Write(GPIO_PORTB, BUZZER, GPIO_LOW);

                LCD_Message("Fan ON",
                            "Light OFF");

                send_text("FAN_AUTO_ON\n");

                while (read_light() <= LIGHT_OFF_LIMIT)
                {
                    check_python_command();

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