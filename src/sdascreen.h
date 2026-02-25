#pragma once
#include <cstdint>
#include <array>

/**
 * An array of 6 bytes representing the row data for the selected digit.
 * Only the 5 least significant bits of each byte are used.
 */
typedef std::array<uint8_t, 6> DigitData_t;

class SDAScreen
{
public:
  SDAScreen(int p_nLoadPin, int p_nDataPin, int p_nSdclkPin, int p_nResetPin) : m_nLoadPin(p_nLoadPin), m_nDataPin(p_nDataPin), m_nSdclkPin(p_nSdclkPin), m_nResetPin(p_nResetPin)
  {
  }

  /**
   * Initialize the screen IO interface.
   */
  void Initialize() const;

  /**
   * Perform a hardware reset of the screen. Call before any other operations.
   */
  void Reset();

  /**
   * Clear the screen.
   */
  void Clear();

  /**
   * Set the brightness of the screen
   * @param p_nBrightness The brightness level (0-7, 0: 0%, 7: 100%)
   * @note Brightness level is inverted from the control register value!
   */
  void SetBrightness(const uint8_t p_nBrightness);

  /**
   * Set the peak current of the screen
   * @param p_bLowPeakCurrent Use reduced peak current (12.5%)?
   */
  void SetPeakCurrent(const bool p_bLowPeakCurrent);

  /**
   * Write raw digit data to the screen.
   * @param p_nDigit The digit to be written (0-7, 0 is the leftmost digit)
   * @param p_aData Digit column data to write.
   */
  void WriteDigit(const uint8_t p_nDigit, const DigitData_t &p_aData) const;

private: // Low-Level API
  struct ControlRegisterData
  {
    bool m_bCLR : 1;   // clear screen (0: clear, 1: normal)
    bool m_bIP : 1;    // peak current (0: maximum, 1: 12.5%)
    uint8_t m_nBR : 3; // brightness (0-7, 0: 100%, 7: 0%)

    ControlRegisterData() :                // after hardware reset:
                            m_bCLR(false), // normal operation
                            m_bIP(false),  // maximum peak current
                            m_nBR(0)       // brightness 100%
    {
    }
  };

  ControlRegisterData m_ControlRegister = ControlRegisterData();

  /**
   * Write control register data to screen
   * @param p_Data The control register data to be sent
   */
  void WriteControlRegister(const ControlRegisterData &p_Data) const;

  /**
   * Select the digit to be displayed (0-7). Call before WriteDigitData().
   * @param p_nDigit The digit to be selected (0-7, 0 is the leftmost digit)
   */
  void SelectDigit(const uint8_t p_nDigit) const;

  /**
   * Write digit row data to the screen. Call after SelectDigit().
   * @param p_aRowData Digit row data to write.
   */
  void WriteDigitData(const DigitData_t &p_aRowData) const;

private: // IO driver
  const int m_nLoadPin;
  const int m_nDataPin;
  const int m_nSdclkPin;
  const int m_nResetPin;

  /**
   * Send 8-bit data to the screen
   */
  void WriteByte(const uint8_t p_nData) const;
};
