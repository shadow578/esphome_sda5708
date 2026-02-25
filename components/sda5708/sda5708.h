#pragma once
#include <cstdint>
#include <array>

#include "esphome/core/hal.h"
#include "esphome/core/component.h"
#include "esphome/core/time.h"

#include "esphome/components/display/display.h"

namespace esphome::sda5708
{
  /// Glyph data for a single 5x7 character on the screen.
  /// Each byte represents one row (of the 7 rows) of the character,
  /// with the 5 least significant bits representing the 5 columns (1: lit, 0: unlit).
  typedef std::array<uint8_t, 7> SDAGlyph_t;

  class SDA5708Component;

  using sda5708_writer_t = display::DisplayWriter<SDA5708Component>;

  /// ESPHome component for controlling a
  /// Siemens SDA5708-24 8 character 5x7 dot matrix LED display.
  class SDA5708Component : public PollingComponent
  {
  public:
    void setup() override;
    void dump_config() override;
    void update() override;
    float get_setup_priority() const override;

    /// Clear the display.
    void clear();

    /// Send the current display buffer to the screen.
    void display();

  private: // Print & Writer API
    std::array<char, 8> display_buffer_{};

  public:
    /// Evaluate the printf-format and print the result at the given position.
    uint8_t printf(uint8_t pos, const char *format, ...) __attribute__((format(printf, 3, 4)));
    /// Evaluate the printf-format and print the result at position 0.
    uint8_t printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

    /// Print `str` at the given position.
    uint8_t print(uint8_t pos, const char *str);
    /// Print `str` at position 0.
    uint8_t print(const char *str);

    /// Evaluate the strftime-format and print the result at the given position.
    uint8_t strftime(uint8_t pos, const char *format, ESPTime time) __attribute__((format(strftime, 3, 0)));

    /// Evaluate the strftime-format and print the result at position 0.
    uint8_t strftime(const char *format, ESPTime time) __attribute__((format(strftime, 2, 0)));

  private: // CodeGen API
    sda5708_writer_t writer_;
    GPIOPin *data_pin_;
    GPIOPin *clock_pin_;
    GPIOPin *load_pin_;
    GPIOPin *reset_pin_;

    bool init_peak_current_ = false;
    uint8_t init_brightness_ = 7; // 0-7

  public:
    void set_writer(sda5708_writer_t &&writer)
    {
      this->writer_ = std::move(writer);
    }

    void set_data_pin(GPIOPin *pin)
    {
      this->data_pin_ = pin;
    }

    void set_clock_pin(GPIOPin *pin)
    {
      this->clock_pin_ = pin;
    }

    void set_load_pin(GPIOPin *pin)
    {
      this->load_pin_ = pin;
    }

    void set_reset_pin(GPIOPin *pin)
    {
      this->reset_pin_ = pin;
    }

    void set_init_peak_current(const bool low_peak_current)
    {
      this->init_peak_current_ = low_peak_current;
    }

    void set_init_brightness(const uint8_t brightness)
    {
      this->init_brightness_ = brightness;
    }

  public: // High-Level Screen API
    /// send a clear command to the screen
    void screen_clear();

    /// Set the brightness of the screen
    /// @param brightness The brightness level (0-7, 0: 0%, 7: 100%)
    /// @note Brightness level is inverted from the control register value.
    void set_brightness(const uint8_t brightness);

    /// Set peak current configuration of the screen
    /// @param low_peak_current Use reduced peak current (true; 12.5%) or maximum peak current (false)?
    void set_peak_current(const bool low_peak_current);

    /// Write a raw glyph to the screen.
    /// @param digit The digit to be written (0-7, 0 is the leftmost digit)
    /// @param glyph The glyph data to be written
    void write_glyph(const uint8_t digit, const SDAGlyph_t &glyph) const;

  private: // Low-Level API
    struct SDAControlRegister
    {
      bool m_bCLR : 1;   // clear screen (0: clear, 1: normal)
      bool m_bIP : 1;    // peak current (0: maximum, 1: 12.5%)
      uint8_t m_nBR : 3; // brightness (0-7, 0: 100%, 7: 0%)

      SDAControlRegister() :                // after hardware reset:
                             m_bCLR(false), // normal operation
                             m_bIP(false),  // maximum peak current
                             m_nBR(0)       // brightness 100%
      {
      }
    };

    SDAControlRegister control_register_;

    /// Perform a hardware reset of the screen.
    void screen_reset();

    /// Write control register to the screen.
    /// @param data Control register data
    void write_control_register(const SDAControlRegister &data) const;

    /// Select a digit for subsequent data writing.
    /// @param digit The digit to select (0-7, 0 is the leftmost digit)
    void select_digit(const uint8_t digit) const;

    /// Write digit data to the previously selected digit.
    /// @param data Data to write to the digit.
    void write_digit_data(const SDAGlyph_t &data) const;

    /// Write a byte to the screen.
    /// @param byte the byte to write.
    void write_byte(const uint8_t byte) const;

    /// Delay for a short time to allow the screen to process commands.
    void screen_delay() const;
  };
} // namespace esphome::sda5708
