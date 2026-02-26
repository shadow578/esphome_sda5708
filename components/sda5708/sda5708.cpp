#include "sda5708.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::sda5708
{
  static const char *const TAG = "sda5708";

#pragma region ESPHome Component Implementation
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

    this->screen_reset();
  }

  void SDA5708Component::dump_config()
  {
    ESP_LOGCONFIG(TAG, "SDA5708:");
    LOG_PIN("  Data Pin: ", this->data_pin_);
    LOG_PIN("  Clock Pin: ", this->clock_pin_);
    LOG_PIN("  Load Pin: ", this->load_pin_);
    LOG_PIN("  Reset Pin: ", this->reset_pin_);
    ESP_LOGCONFIG(TAG, "  Initial Brightness: %u", this->init_brightness_);
    ESP_LOGCONFIG(TAG, "  Initial Peak Current: %s", this->init_peak_current_ ? "12.5%" : "Maximum");
    LOG_UPDATE_INTERVAL(this);
  }

  void SDA5708Component::update()
  {
    // writer set and auto-redraw enabled?
    if (this->writer_.has_value() && this->automatic_redraw_skip_frames_ == 0)
    {
      clear();
      (*this->writer_)(*this);
    }

    // decrement auto-redraw
    if (this->automatic_redraw_skip_frames_ > 0)
    {
      this->automatic_redraw_skip_frames_--;
    }

    // send buffer to screen
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
    const auto &font = get_font();

    for (uint8_t i = 0; i < this->display_buffer_.size(); i++)
    {
      char c = this->display_buffer_[i];

      if (const auto glyph_opt = font.get_glyph(c); glyph_opt.has_value())
        write_glyph(i, glyph_opt.value());
      else
        ESP_LOGW(TAG, "No glyph found for character '%c' (0x%02X)", c, static_cast<uint8_t>(c));
    }
  }
#pragma endregion

#pragma region Print & Writer API
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

  void SDA5708Component::pause_automatic_redraw(const int frames)
  {
    this->automatic_redraw_skip_frames_ = frames;
  }

  void SDA5708Component::resume_automatic_redraw()
  {
    this->automatic_redraw_skip_frames_ = 0;
  }
#pragma endregion

#pragma region High-Level Screen API
  void SDA5708Component::screen_clear()
  {
    this->control_register_.m_bCLR = true;
    write_control_register(this->control_register_);

    // set CLR back to normal operation after clearing
    this->control_register_.m_bCLR = false;
    write_control_register(this->control_register_);
  }

  void SDA5708Component::set_brightness(const uint8_t brightness)
  {
    // invert brightness level for control register (0 -> 7, 7 -> 0)
    this->control_register_.m_nBR = 7 - (brightness & 0b111);
    write_control_register(this->control_register_);
  }

  uint8_t SDA5708Component::get_brightness() const
  {
    // invert brightness level from control register
    return 7 - (this->control_register_.m_nBR & 0b111);
  }

  void SDA5708Component::set_peak_current(const bool low_peak_current)
  {
    this->control_register_.m_bIP = low_peak_current;
    write_control_register(this->control_register_);
  }

  bool SDA5708Component::get_peak_current() const
  {
    return this->control_register_.m_bIP;
  }

  void SDA5708Component::write_glyph(const uint8_t digit, const SDAGlyph_t &glyph) const
  {
    select_digit(digit);
    write_digit_data(glyph);
  }
#pragma endregion

#pragma region Low-Level API
  void SDA5708Component::screen_reset()
  {
    // #RESET LOW to reset
    this->reset_pin_->digital_write(false);
    screen_delay();

    // #RESET HIGH to end reset
    this->reset_pin_->digital_write(true);
    screen_delay();

    // reset internal control register mirror to default values
    control_register_ = SDAControlRegister();

    // apply control register settings set by codegen after reset
    set_peak_current(init_peak_current_);
    set_brightness(init_brightness_);
  }

  void SDA5708Component::write_control_register(const SDAControlRegister &data) const
  {
    uint8_t cr = 0b11000000;            // select control register with D7=1, D6=1 and D4=0
    cr |= (data.m_bCLR ? 0 : (1 << 5)); // CLR bit (active low) on D5
    cr |= (data.m_bIP ? (1 << 4) : 0);  // IP bit on D4
    cr |= (data.m_nBR & 0b111);         // BR bits on D2-D0

    write_byte(cr);
  }

  void SDA5708Component::select_digit(const uint8_t digit) const
  {
    if (digit > 7)
      return;

    uint8_t cs = 0b10100000; // address register on D7=1, D6=0, D5=1
    cs |= (digit & 0b111);   // digit idx on D2-D0

    write_byte(cs);
  }

  void SDA5708Component::write_digit_data(const SDAGlyph_t &data) const
  {
    for (int i = 0; i < 7; i++)
    {
      uint8_t cd = 0b00000000;   // column data register with D7=0, D6=0, D5=0
      cd |= (data[i] & 0b11111); // remaining 5 bits for column data

      write_byte(cd);
    }
  }

  void SDA5708Component::write_byte(const uint8_t data) const
  {
    // #LOAD LOW to start transfer
    this->load_pin_->digital_write(false);

    // shift out 8 bits, LSB first
    for (int i = 0; i < 8; i++)
    {
      this->data_pin_->digital_write((data >> i) & 0x01);

      this->clock_pin_->digital_write(true);
      screen_delay();
      this->clock_pin_->digital_write(false);
      screen_delay();
    }

    // #LOAD HIGH to end transfer
    this->load_pin_->digital_write(true);

    // add a slight delay after each byte to allow
    // the screen to process the data
    screen_delay();
  }

  void SDA5708Component::screen_delay() const
  {
    // the screen requires a short delay for data processing.
    // per SB-Projects' post, the minimum is ~200ns, but to be safe
    // we use a generous 5x margin.
    delayMicroseconds(1);
  }
#pragma endregion
} // namespace esphome::sda5708
