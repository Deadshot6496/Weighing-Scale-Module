typedef union{
  unsigned char dat[4];
  uint32_t f_data;
}bytes_buffer;

void EEPROM_BULK_SAVE(uint8_t add,uint32_t myData)
{
  bytes_buffer bulk_dat;
  bulk_dat.f_data = myData;
  for(int i = 0;i<4;i++)
  {
    EEPROM.write(add,bulk_dat.dat[i]);
    add++;
  }
}

uint32_t EEPROM_BULK_READ(uint8_t add)
{
  bytes_buffer bulk_dat;
  for(int i = 0; i<4;i++)
  {
    bulk_dat.dat[i] = EEPROM.read(add);
    add++;
  }
  return bulk_dat.f_data;
}
