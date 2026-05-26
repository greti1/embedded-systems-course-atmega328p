#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// ---------------- PINI ----------------
#define GREEN_LED   PD5
#define RED_LED     PD6
#define YELLOW_LED  PD7
#define BUZZER      PB0

#define TEMP_CHANNEL 0

// ---------------- I2C (TWI) ----------------
void TWI_init() {
    TWSR = 0x00;
    TWBR = 72;
    TWCR = (1 << TWEN);
}

void TWI_start() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void TWI_stop() {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void TWI_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

// LCD I2C address (modifică dacă e nevoie)
#define LCD_ADDR 0x4E

void lcd_pulse(uint8_t data) {
    TWI_write(data | 0x04);
    TWI_write(data & ~0x04);
}

void lcd_send(uint8_t data, uint8_t mode) {

    uint8_t high = (data & 0xF0) | mode | 0x08;
    uint8_t low  = ((data << 4) & 0xF0) | mode | 0x08;

    TWI_start();
    TWI_write(LCD_ADDR);

    lcd_pulse(high);
    lcd_pulse(low);

    TWI_stop();
}

void lcd_cmd(uint8_t cmd) {
    lcd_send(cmd, 0x00);
    _delay_ms(2);
}

void lcd_char(char c) {
    lcd_send(c, 0x01);
}

void lcd_print(char *s) {
    while (*s) lcd_char(*s++);
}

void lcd_init() {
    _delay_ms(50);

    lcd_cmd(0x02);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
}

// ---------------- ADC ----------------
void adc_init() {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | ch;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

// ---------------- TEMP (STABILIZAT) ----------------
float last_temp = 0;

float read_temp() {

    uint32_t sum = 0;

    for (int i = 0; i < 20; i++) {
        sum += adc_read(TEMP_CHANNEL);
        _delay_ms(1);
    }

    uint16_t adc = sum / 20;

    float voltage = adc * (5.0 / 1023.0);
    float temp = (voltage * 100.0) - 273.15;

    // smoothing (foarte important)
    temp = (temp * 0.6) + (last_temp * 0.4);
    last_temp = temp;

    return temp;
}

int main(void) {

    DDRD |= (1 << GREEN_LED) | (1 << RED_LED) | (1 << YELLOW_LED);
    DDRB |= (1 << BUZZER);

    adc_init();
    TWI_init();
    lcd_init();

    char line[20];

    float temp;

    // STATE MACHINE
    uint8_t state = 0;

    // praguri (histerezis)
    float T_LOW  = 20.0;
    float T_HIGH = 35.0;

    while (1) {

        temp = read_temp();

        int t = (int)(temp * 10);

        snprintf(line, sizeof(line),
                 "Temp:%d.%dC",
                 t / 10,
                 t % 10);

        lcd_cmd(0x01);
        lcd_cmd(0x80);
        lcd_print(line);

        lcd_cmd(0xC0);

        // ---------------- STATE 0: OK ----------------
        if (state == 0) {

            PORTD |= (1 << GREEN_LED);
            PORTD &= ~((1 << RED_LED) | (1 << YELLOW_LED));
            PORTB &= ~(1 << BUZZER);

            lcd_print("STATUS: OK");

            if (temp > T_LOW) {
                state = 1;
            }
        }

        // ---------------- STATE 1: ALARM ----------------
        else if (state == 1) {

            PORTD |= (1 << RED_LED);
            PORTD &= ~((1 << GREEN_LED) | (1 << YELLOW_LED));
            PORTB |= (1 << BUZZER);

            lcd_print("STATUS: ALARM");

            if (temp > T_HIGH) {
                state = 2;
            }
            else if (temp < T_LOW) {
                state = 0;
            }
        }

        // ---------------- STATE 2: COOLING ----------------
        else if (state == 2) {

            PORTD |= (1 << YELLOW_LED);
            PORTD &= ~((1 << GREEN_LED) | (1 << RED_LED));
            PORTB &= ~(1 << BUZZER);

            lcd_print("COOLING MODE");

            if (temp < T_HIGH - 1) {
                state = 1;
            }
        }

        _delay_ms(300);
    }
}