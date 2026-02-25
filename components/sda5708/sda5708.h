#pragma once
#include "esphome/core/hal.h"
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
    float get_setup_priority() const override;

    /// Clear the display.
    void clear();

    /// Send the current display buffer to the screen.
    void display();

  private: // Print API
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

  private: // Setters for CodeGen
    sda5708_writer_t writer_;
    GPIOPin *data_pin_;
    GPIOPin *clock_pin_;
    GPIOPin *load_pin_;
    GPIOPin *reset_pin_;

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
  };
} // namespace esphome::sda5708
