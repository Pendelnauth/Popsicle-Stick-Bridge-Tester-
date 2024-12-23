//make sure that if 16x or possibly even 8x gains are used input impedance compensation is implemented
//this can be done by multiplying by

void ADC_Take_Reading() {
  attachInterrupt(digitalPinToInterrupt(ADC_IRQ0), ADC_Read_Value0, FALLING);
  attachInterrupt(digitalPinToInterrupt(ADC_IRQ1), ADC_Read_Value1, FALLING);

  digitalWrite(ADC_CS0, LOW);
  SPI1.beginTransaction(SPISettings(15'000'000, MSBFIRST, SPI_MODE0));

  SPI1.transfer(0b01101000);  //fast command start adc conversion
  SPI1.endTransaction();
  digitalWrite(ADC_CS0, HIGH);

  digitalWrite(ADC_CS1, LOW);
  SPI1.beginTransaction(SPISettings(15'000'000, MSBFIRST, SPI_MODE0));

  SPI1.transfer(0b01101000);  //fast command start adc conversion
  SPI1.endTransaction();
  digitalWrite(ADC_CS1, HIGH);

  ADC_Busy = 1;
}

uint32_t Get_ADC_Register(uint8_t ADC, uint8_t Register_Number) {
  uint8_t Return_Bytes[16] = { 3, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 1, 1, 2, 2 };
  uint8_t ADC_CS;
  if (ADC) {
    ADC_CS = ADC_CS1;
  } else {
    ADC_CS = ADC_CS0;
  }
  digitalWrite(ADC_CS, LOW);
  SPI1.beginTransaction(SPISettings(15'000'000, MSBFIRST, SPI_MODE0));
  uint32_t ADC_Data = 0, Transfer_Data = 0;
  //0b01xxxx01//device addr 01, register number, static read
  if (!(Register_Number >= 16)) {
    Transfer_Data = (Register_Number << 2) + 0b01000001;
    SPI1.transfer(Transfer_Data);  //request data
    for (int count = 0; count < Return_Bytes[Register_Number]; count++) {
      ADC_Data = ADC_Data << 8;
      ADC_Data = ADC_Data + SPI1.transfer(0);  //transfer the addr
    }
  }
  digitalWrite(ADC_CS, HIGH);
  SPI1.endTransaction();
  return ADC_Data;
}


void ADC_Read_Value0() {
  ADC_Read_Value_Flag0 = 1;
  detachInterrupt(digitalPinToInterrupt(ADC_IRQ0));
}

void ADC_Read_Value1() {
  ADC_Read_Value_Flag1 = 1;
  detachInterrupt(digitalPinToInterrupt(ADC_IRQ1));
}

void SPI_Restart(uint8_t ADC_CS) {
  SPI1.endTransaction();
  digitalWrite(ADC_CS, HIGH);
  SPI1.beginTransaction(SPISettings(15'000'000, MSBFIRST, SPI_MODE0));
  digitalWrite(ADC_CS, LOW);
}
