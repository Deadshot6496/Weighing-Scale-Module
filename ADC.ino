
void readADC(void)
{
  uint8_t data[3] = { 0 };
  //while(HAL_GPIO_ReadPin(SCALE_DATA_GPIO_Port, SCALE_DATA_Pin) != 0);
  if ((PINC & (1 << 5)) == 0)
  {
    for (uint8_t j = 3; j--;) {
      for (int i = 7; i >= 0; i--)
      {
        PORTC |= (1 << 4);
        //         while(wait < 15)
        //         {
        //           wait++;
        //         }
        //         wait = 0;
        //delayMicroseconds(1);
        if /*((GPIOC->IDR & SCALE_CLK_Pin) == 1)*/((PINC & (1 << 5)) == 1)
        {
          data[j] |= (1 << i);
        }
        else
        {
          data[j] &= ~(1 << i);
        }
        PORTC &= ~(1 << 4);
        //         while(wait < 15)
        //       {
        //         wait++;
        //       }
        //             wait = 0;
        //delayMicroseconds(1);
      }
    }

    // set gain
    for (int i = 0; i < 1; i++) {
      PORTC |= (1 << 4);
      //         while(wait < 15)
      //         {
      //           wait++;
      //         }
      //         wait = 0;
      data[2] ^= 0x80;
      copy_adc = ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
      PORTC &= ~(1 << 4);

    }
    char buffer[10];
    snprintf(buffer, 10, "%lu", copy_adc);
    UART_TRANSMIT_STRING_NL(buffer, 8);

    //accumulator = ((float)y_one * (float)val1) / (float)val2;
    if (copy_adc < 8100000)
    {
      myAdc += copy_adc;
      //      accumulator += ((float)y_one * (float)val1) / (float)val2;
      adc_index++;
      if (adc_index == 30)
      {
        myAdc = myAdc / 30;
        int32_t val1 = (myAdc - x_zero);
        int32_t val2 = (x_one - x_zero);
        scale_weight = ((float)y_one * (float)val1) / (float)val2;
        myAdc = 0;
        accumulator /= 30;
        if (accumulator > -0.5)
        {
          scale_weight = accumulator;
        }
        adc_index = 0;
      }
    }

  }
}

unsigned long int read_adc()
{
 //digitalWrite(A5,HIGH);
 digitalWrite(A4,LOW);
 unsigned long int Count=0;
 while(digitalRead(A5) == HIGH);
 for (int i=0;i<24;i++){
 digitalWrite(A4,HIGH);
 Count=Count<<1;
 digitalWrite(A4,LOW);
 if(digitalRead(A5)==HIGH)
  {
    Count++;
  }
 }
 digitalWrite(A4,HIGH);
 Count=Count^0x800000;
 digitalWrite(A4,LOW); 
 return Count;
}

int64_t read_data()
{
  if(digitalRead(A5) == 0)
  {
        byte data[3];
  
    for (byte j = 3; j--;) {
      data[j] = shiftIn(A5, A4, MSBFIRST);
    }
  
    // set gain
    for (int i = 0; i < 1; i++) {
      digitalWrite(A1, HIGH);
      digitalWrite(A1, LOW);
    }
  
    data[2] ^= 0x80;
    return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
  }
  return -1;
}


void reverse(char *str, int len)
{
  int i = 0, j = len - 1, temp;
  while (i < j)
  {
    temp = str[i];
    str[i] = str[j];
    str[j] = temp;
    i++; j--;
  }
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
  int i = 0;
  while (x)
  {
    str[i++] = (x % 10) + '0';
    x = x / 10;
  }

  // If number of digits required is more, then
  // add 0s at the beginning
  while (i < d)
    str[i++] = '0';

  reverse(str, i);
  str[i] = ' ';
  return i;
}

// Converts a floating point number to string.
void ftoa( char *res, float n, int afterpoint)
{
  // Extract integer part
  int ipart = (int)n;

  // Extract floating part
  float fpart = n - (float)ipart;

  // convert integer part to string
  int i = intToStr(ipart, res, 0);

  // check for display option after point
  if (afterpoint != 0)
  {
    res[i] = '.';  // add dot

    // Get the value of fraction part upto given no.
    // of points after dot. The third parameter is needed
    // to handle cases like 233.007
    fpart = fpart * pow(10, afterpoint);

    intToStr((int)fpart, res + i + 1, afterpoint);
  }
}
