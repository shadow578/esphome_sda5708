#pragma once
#include <cstdint>
#include <array>
#include <map>

/**
 * Display data for a single 5x7 pixel digit on the screen.
 * Each byte represents one row (of the 7 rows) of the digit,
 * with the 5 least significant bits representing the 5 columns (1: lit, 0: unlit).
 */
typedef std::array<uint8_t, 7> DigitData_t;

/**
 * Screen driver for Siemens SDA5708-24 8 character 5x7 dot matrix LED display.
 * Based on information from https://www.sbprojects.net/knowledge/footprints/sda5708/index.php
 */
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
   * @param p_bLowPeakCurrent Use reduced peak current (12.5%) or maximum peak current?
   * @note The screen tends to flicker slightly with low peak current.
   */
  void SetPeakCurrent(const bool p_bLowPeakCurrent);

  /**
   * Write raw digit data to the screen.
   * @param p_nDigit The digit to be written (0-7, 0 is the leftmost digit)
   * @param p_aData Digit column data to write.
   */
  void WriteDigit(const uint8_t p_nDigit, const DigitData_t &p_aData) const;

  /**
   * Write a character to the screen.
   * @param p_nDigit The digit to be written (0-7, 0 is the leftmost digit)
   * @param p_cChar The character to be written.
   */
  void WriteDigit(const uint8_t p_nDigit, const char p_cChar) const;

  /**
   * Set a custom character in the font map.
   * This allows you to define custom characters or override existing ones.
   * @param p_cChar The character to be set.
   * @param p_aData The 5x7 pixel data for the character.
   * @note Font data is shared across all instances of SDAScreen, so this will affect all screens using this driver.
   */
  static void SetCustomCharacter(const char p_cChar, const DigitData_t &p_aData);

private:
  static std::map<char, DigitData_t> g_mFont;

  /**
   * Lookup font data from the global font map.
   * @param p_cChar The character to look up.
   * @param p_aData Output parameter to store the font data for the character.
   * @return true if the character was found in the font map, false otherwise.
   */
  static bool GetFontData(const char p_cChar, DigitData_t &p_aData);

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
