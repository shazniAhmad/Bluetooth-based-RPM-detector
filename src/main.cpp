#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <wire.h>

// Constants
#define IR_SENSOR_PIN   PINC
#define IR_SENSOR_BIT   7
#define BLADES_PER_REV  4
#define LCD_RS (1 << PB0)
#define LCD_EN (1 << PB2)
#define LCD_DB4 (1 << PB4)
#define LCD_DB5 (1 << PB5)
#define LCD_DB6 (1 << PB6)
#define LCD_DB7 (1 << PB7)
#define LCD_DATA_PORT PORTB

#define EICRA   _SFR_MEM8(0x69)
#define EIMSK   _SFR_MEM8(0x3D)

// UART Constants for Bluetooth
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)


// Global Variables
volatile uint32_t bladeCount = 2;
volatile uint32_t timeStart = 0;
volatile uint32_t timeEnd = 0;
volatile uint32_t rpm = 0;

// initialize IR sensor and interrupts
void initializeIRSensor() {
    IR_SENSOR_PIN &= ~(1 << IR_SENSOR_BIT);
    EICRA |= (1 << ISC00) | (1 << ISC01);
    EIMSK |= (1 << INT0);
    sei();
}

void sendLCDCommand(uint8_t command) {
    PORTB &= ~LCD_RS; 
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (command & 0xF0);
    PORTB |= LCD_EN;
    _delay_us(1);
    PORTB &= ~LCD_EN;

    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((command << 4) & 0xF0);
    PORTB |= LCD_EN;
    _delay_us(1);
    PORTB &= ~LCD_EN;

    _delay_ms(2);  // Allowing the LCD time to process the command
}

//send a character to the LCD
void sendLCDCharacter(char data) {
    PORTB |= LCD_RS; 
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (data & 0xF0);
    PORTB |= LCD_EN;
    _delay_us(1);
    PORTB &= ~LCD_EN;

    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((data << 4) & 0xF0);
    PORTB |= LCD_EN;
    _delay_us(1);
    PORTB &= ~LCD_EN;
}

//initialize the LCD
void initializeLCD() {
    DDRB |= LCD_RS | LCD_EN | LCD_DB4 | LCD_DB5 | LCD_DB6 | LCD_DB7;
    _delay_ms(15);  // LCD power-up delay

    // Initialize the LCD
    sendLCDCommand(0x02); // Initialize
    sendLCDCommand(0x28); // 4-bit, 2-line, 5x8 font
    sendLCDCommand(0x0C); // Display on, cursor off, blink off
    sendLCDCommand(0x06); // Increment cursor, no shift
    sendLCDCommand(0x01); // Clear display
    _delay_ms(2);  // LCD initialization delay
}



// Function to display RPM on the LCD
void displayRPMOnLCD(uint32_t rpm) {
    sendLCDCommand(0x01); // Clear display
    char buffer[16];
    sprintf(buffer, "RPM: %lu", rpm);
    for (int i = 0; buffer[i]; i++) {
        sendLCDCharacter(buffer[i]);
    }
}



// Function to initialize UART for Bluetooth communication
void initializeUART() {
    UBRRH = (unsigned char)(MYUBRR >> 8);
    UBRRL = (unsigned char)MYUBRR;

    // Enable receiver and transmitter
    UCSRB = (1 << RXEN) | (1 << TXEN);

    // Set frame format: 8 data bits, 1 stop bit
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

// Function to send data over UART
void sendUARTData(uint32_t data) {
    // Wait for empty transmit buffer
    while (!(UCSRA & (1 << UDRE)));

    // Put data into the buffer, sends the data
    UDR = data;
}

int getTime(){
  long t = millis();
  for(int i = 0; i<100; i++) {
    while(PINC7==0);
    while(PINC7==1);
  }
  return millis()-t;
}

int main(void) {
    // Initialize IR sensor and interrupts
    initializeIRSensor();

    // Initialize LCD
    initializeLCD();

    // Initialize UART for Bluetooth
    initializeUART();

    while (1) {
        // Calculate RPM and update 'rpm' variable
        rpm = 100*60/(bladeCount*getTime()/1000);

        // Display RPM on the LCD
        displayRPMOnLCD(rpm);

        // Send RPM data via Bluetooth
        sendUARTData(rpm);

    
    }

    return 0;
}
