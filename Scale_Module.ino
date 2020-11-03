#include "EEPROM.h"

#define led1  9
#define led2  10

#define SCK A4
#define DAT A5

#define TOT_SAMPLES (uint8_t)25 // Set Total Samples as dynamically altered via commands  


uint32_t copy_adc = 0;
int64_t myAdc = 0;
float accumulator = 0;
int adc_index = 0;
/*
   INTERRUPT SECTION
*/
float scale_weight = 0;
uint32_t x_zero, x_one;
float y_one;  // 53.790 STANDARD WEIGHT 
volatile bool start_clock = false, present_state = false, scale_data_ready = false;
const uint8_t clock_pulses = 25;
volatile uint8_t scale_bytes[3];
volatile int8_t scale_byte_index = 0, scale_bit_index = 0;

int8_t myInt = 0;
float myFloat = 0;


char myCommand[8] = {0}, received_char, command_index;
bool command_received = false;
void *cmd_callback(void *);

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  pinMode(SCK, OUTPUT);       //Scale Pins
  pinMode(DAT, INPUT_PULLUP);
  //  Serial.begin(9600);
  UART_INIT();
  read_cal_values();
  //init_timer1();
}

void loop()
{
  //char empty[8] = {0,0,0,0,0,0,0,0};
  int64_t temp = read_adc();
  if (temp != -1)
  {
    myAdc += temp;
  }
  adc_index++;
  if (adc_index == TOT_SAMPLES)
  {
    copy_adc = myAdc / (TOT_SAMPLES + 1);
    myAdc = copy_adc;
    int32_t val1 = (copy_adc - x_zero);
    int32_t val2 = (x_one - x_zero);
    scale_weight = ((float)y_one * (float)val1) / (float)val2;
    adc_index = 0;
    char buffer[10] = { 0 };
    //ftoa(buffer,scale_weight,3);
    int64_t myWeight = (scale_weight * 1000 + 0.5);
    snprintf(buffer, 10, "%ld", myWeight);
    UART_TRANSMIT_STRING_NL(buffer, 8);
  }


  //Serial.println(copy_adc);

  ////  ftoa(buffer,copy_adc,3);
  uint32_t weight = scale_weight * 1000;


  process_clear_command();
}

void process_clear_command()
{
  if (command_received)
  {
    if (myCommand[0] == 'T')
    {
      EEPROM_BULK_SAVE(0, copy_adc);
      x_zero = EEPROM_BULK_READ(0);
      //UART_TRANSMIT_STRING("DONE", 4);
      char buffer[10] = {0};
      snprintf(buffer, 10, "%lu", x_zero);
      UART_TRANSMIT_STRING_NL(buffer, 8);
    }
    else if (myCommand[0] == 'X')
    {
      EEPROM_BULK_SAVE(4, copy_adc);
      x_one = EEPROM_BULK_READ(4);
      //UART_TRANSMIT_STRING("DONE", 4);
      char buffer[10] = {0};
      snprintf(buffer, 10, "%lu", x_one);
      UART_TRANSMIT_STRING_NL(buffer, 8);
    }
    else if (myCommand[0] == 'Y')
    {
      int32_t myVal = atol(&myCommand[1]);
      EEPROM_BULK_SAVE(8, myVal);
      y_one = (float)(EEPROM_BULK_READ(8)) / 1000;
      //UART_TRANSMIT_STRING("DONE", 4);
      char buffer[10];
      ftoa(buffer, y_one, 3);
      //snprintf(buffer, 10, "%lu", y_one);
      UART_TRANSMIT_STRING_NL(buffer, 8);
    }
    memset(myCommand, 0, 8);
    command_received = false;
  }
}

void read_cal_values()
{
  x_zero = EEPROM_BULK_READ(0);
  x_one = EEPROM_BULK_READ(4);
  y_one = (float)(EEPROM_BULK_READ(8)) / 1000;
}


ISR(USART_RX_vect)
{
  received_char = UDR0;
  if (received_char != '\n')
  {
    myCommand[command_index] = received_char;
    command_index++;
    command_index = command_index > 7 ? 0 : command_index;
  }
  else if (received_char == '\n')
  {
    command_received = true;
    command_index = 0;
  }
}

void init_timer1()
{
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << 3) | (1 << 0);
  OCR1A = 0;        // 2.4us is the minimum toggle speed
  TIMSK1 |= (1 << 1); //OCIEA
  cli();
  sei();
}

ISR(TIMER1_COMPA_vect)
{
  PORTB ^= (1 << 1);
}

void UART_INIT()
{
  uint16_t br = 103;
  UBRR0H = (uint8_t)(br >> 8);
  UBRR0L = (uint8_t)(br);
  UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00) | (1 << USBS0);
  UCSR0B |= (1 << RXCIE0) | (1 << TXEN0) | (1 << RXEN0);
}
void UART_TRANSMIT_CHAR( unsigned char data )
{
  uint16_t timeout_approx = 0;
  /* Wait for empty transmit buffer */
  while ( !( UCSR0A & (1 << UDRE0)))
  {
    //    timeout_approx ++;
    //    if(timeout_approx > 10000)
    //    {
    //      break;
    //    }
  }
  /* Put data into buffer, sends the data */
  UDR0 = data;
}
/*
   LIMITED LENGTH
*/
void UART_TRANSMIT_STRING(uint8_t  *myString, uint8_t char_len)
{
  while (char_len > 0)
  {
    UART_TRANSMIT_CHAR((*myString));
    myString++;
    char_len--;
  }
}
/*
   LIMITED LENGTH
*/
void UART_TRANSMIT_STRING_NL(uint8_t  *myString, uint8_t char_len)
{
  while (char_len > 0)
  {
    UART_TRANSMIT_CHAR((*myString));
    myString++;
    char_len--;
  }
  UART_TRANSMIT_CHAR('\n');
}
/*
   Untill NuLL
*/
void UART_TRANSMIT_STRING_(uint8_t  *myString)
{
  while (*myString != '\0')
  {
    UART_TRANSMIT_CHAR((*myString));
    myString++;
  }
}
