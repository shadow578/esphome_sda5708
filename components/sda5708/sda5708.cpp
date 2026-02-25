#include "sda5708.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::sda5708
{
  static const char *const TAG = "sda5708";

  void SDA5708Component::setup()
  {
    this->data_pin_->setup(); // OUTPUT
    this->data_pin_->digital_write(false);

    this->clock_pin_->setup(); // OUTPUT
    this->clock_pin_->digital_write(false);

    this->load_pin_->setup();             // OUTPUT
    this->load_pin_->digital_write(true); // active LOW

    this->reset_pin_->setup();             // OUTPUT
    this->reset_pin_->digital_write(true); // active LOW

    // TODO this->reset();
  }

  void SDA5708Component::dump_config()
  {
    ESP_LOGCONFIG(TAG, "SDA5708:");
    LOG_PIN("  Data Pin: ", this->data_pin_);
    LOG_PIN("  Clock Pin: ", this->clock_pin_);
    LOG_PIN("  Load Pin: ", this->load_pin_);
    LOG_PIN("  Reset Pin: ", this->reset_pin_);
    LOG_UPDATE_INTERVAL(this);
  }

  void SDA5708Component::update()
  {
    if (this->writer_.has_value())
    {
      clear();
      (*this->writer_)(*this);
    }

    display();
  }

  float SDA5708Component::get_setup_priority() const
  {
    return setup_priority::PROCESSOR;
  }

  void SDA5708Component::clear()
  {
    std::fill(this->display_buffer_.begin(), this->display_buffer_.end(), ' ');
  }

  void SDA5708Component::display()
  {
    // TODO write buffer to display
  }

  uint8_t SDA5708Component::print(uint8_t pos, const char *str)
  {
    if (pos >= this->display_buffer_.size())
      return pos;

    uint8_t i = 0;
    while (str[i] != '\0' && pos + i < this->display_buffer_.size())
    {
      this->display_buffer_[pos + i] = str[i];
      i++;
    }

    return pos + i;
  }

  uint8_t SDA5708Component::print(const char *str)
  {
    return print(0, str);
  }

  uint8_t SDA5708Component::printf(uint8_t pos, const char *format, ...)
  {
    char buffer[64];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len < 0)
      return pos;

    return print(pos, buffer);
  }

  uint8_t SDA5708Component::printf(const char *format, ...)
  {
    char buffer[64];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len < 0)
      return 0;

    return print(0, buffer);
  }

  uint8_t SDA5708Component::strftime(uint8_t pos, const char *format, ESPTime time)
  {
    char buffer[64];
    size_t ret = time.strftime(buffer, sizeof(buffer), format);

    if (ret == 0)
      return pos;

    return print(pos, buffer);
  }

  uint8_t SDA5708Component::strftime(const char *format, ESPTime time)
  {
    return strftime(0, format, time);
  }

} // namespace esphome::sda5708
