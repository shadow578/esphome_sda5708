#pragma once
#include "esphome/core/component.h"
#include "esphome/core/time.h"

#include "esphome/components/display/display.h"

namespace esphome::sda5708
{
  class SDA5708Component;

  using sda5708_writer_t = display::DisplayWriter<SDA5708Component>;

  class SDA5708Component : public PollingComponent
  {
  public:
    void setup() override;
    void dump_config() override;
    void update() override;
    void loop() override;
    float get_setup_priority() const override;

    /// Send the current display buffer to the screen.
    void display();

    /// Clear the display.
    void clear();

  public: // Print API
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

  private: // Setters for CodeGen
    sda5708_writer_t m_Writer;
    GPIOPin *m_pDataPin;
    GPIOPin *m_pClockPin;
    GPIOPin *m_pLoadPin;
    GPIOPin *m_pResetPin;

  public:
    void set_writer(sda5708_writer_t &&p_Writer)
    {
      this->m_Writer = std::move(p_Writer);
    }

    void set_data_pin(GPIOPin *p_pPin)
    {
      this->m_pDataPin = p_pPin;
    }

    void set_clock_pin(GPIOPin *p_pPin)
    {
      this->m_pClockPin = p_pPin;
    }

    void set_load_pin(GPIOPin *p_pPin)
    {
      this->m_pLoadPin = p_pPin;
    }

    void set_reset_pin(GPIOPin *p_pPin)
    {
      this->m_pResetPin = p_pPin;
    }
  };
} // namespace esphome::sda5708
