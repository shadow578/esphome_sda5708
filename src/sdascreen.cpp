#include "sdascreen.h"
#include <Arduino.h>

void SDAScreen::Initialize() const
{
  pinMode(m_nLoadPin, OUTPUT);
  pinMode(m_nDataPin, OUTPUT);
  pinMode(m_nSdclkPin, OUTPUT);
  pinMode(m_nResetPin, OUTPUT);

  digitalWrite(m_nLoadPin, HIGH); // Active low load
  digitalWrite(m_nDataPin, LOW);
  digitalWrite(m_nSdclkPin, LOW);
  digitalWrite(m_nResetPin, HIGH); // Active low reset
}

void SDAScreen::Reset()
{
  digitalWrite(m_nResetPin, LOW);
  delay(1);
  digitalWrite(m_nResetPin, HIGH);
  delay(1);

  // set internal control register mirror to default values after reset
  m_ControlRegister = ControlRegisterData();
}

void SDAScreen::Clear()
{
  m_ControlRegister.m_bCLR = true;
  WriteControlRegister(m_ControlRegister);

  m_ControlRegister.m_bCLR = false;
  WriteControlRegister(m_ControlRegister);
}

void SDAScreen::SetBrightness(const uint8_t p_nBrightness)
{
  // invert brightness level for control register (0 -> 7, 7 -> 0)
  m_ControlRegister.m_nBR = 7 - (p_nBrightness & 0b111);
  WriteControlRegister(m_ControlRegister);
}

void SDAScreen::SetPeakCurrent(const bool p_bLowPeakCurrent)
{
  m_ControlRegister.m_bIP = p_bLowPeakCurrent;
  WriteControlRegister(m_ControlRegister);
}

void SDAScreen::WriteDigit(const uint8_t p_nDigit, const DigitData_t &p_aData) const
{
  SelectDigit(p_nDigit);
  WriteDigitData(p_aData);
}

void SDAScreen::WriteControlRegister(const ControlRegisterData &p_Data) const
{
  uint8_t data = 0b11000000;              // select control register with D7=1, D6=1 and D4=0
  data |= (p_Data.m_bCLR ? 0 : (1 << 5)); // CLR bit (active low) on D5
  data |= (p_Data.m_bIP ? (1 << 4) : 0);  // IP bit on D4
  data |= (p_Data.m_nBR & 0b111);         // BR bits on D2-D0

  WriteByte(data);
}

void SDAScreen::SelectDigit(const uint8_t p_nDigit) const
{
  if (p_nDigit > 7)
    return;

  uint8_t data = 0b10100000;  // address register on D7=1, D6=0, D5=1
  data |= (p_nDigit & 0b111); // digit on D2-D0

  WriteByte(data);
}

void SDAScreen::WriteDigitData(const DigitData_t &p_aRowData) const
{
  for (int i = 0; i < 6; i++)
  {
    uint8_t data = 0b00000000; // column data register with D7=0, D6=0, D5=0
    data |= (p_aRowData[i] & 0b11111);

    WriteByte(data);
  }
}

void SDAScreen::WriteByte(const uint8_t p_nData) const
{
  // load LOW to start transfer
  digitalWrite(m_nLoadPin, LOW);

  // transfer 8 bits of data, LSB (D0) first
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(m_nDataPin, (p_nData >> i) & 1);

    digitalWrite(m_nSdclkPin, HIGH);
    delay(1);
    digitalWrite(m_nSdclkPin, LOW);
    delay(1);
  }

  // load HIGH to latch data in
  digitalWrite(m_nLoadPin, HIGH);

  // allow time to process
  delay(1);
}