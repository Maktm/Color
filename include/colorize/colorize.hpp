#pragma once

#include <ostream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#error "Unsupport platform in use"
#endif // #if defined(_WIN32)

namespace colorize
{
enum class Color
{
  Red = 1,
  Green = 2,
  Yellow = 3,
  Blue = 4,
  Cyan = 5,
  Magenta = 6,
  White = 7
};

enum Foreground : std::uint16_t
{
  Red = 0x4,
  Green = 0x2,
  Yellow = 0x6,
  Blue = 0x9,
  Cyan = 0xB,
  Magenta = 0xD,
  White = 0x7
};

template <typename CharT> class basic_colorizer
{
  using string_type = std::basic_string<CharT>;
  using stream_type = std::basic_ostream<CharT>;

public:
  basic_colorizer(string_type const& s)
    : text_{s}, std_handle_{GetStdHandle(STD_OUTPUT_HANDLE)}, old_attrib_{0}
  {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(std_handle_, &csbi);
    old_attrib_ = csbi.wAttributes;
  }

  ~basic_colorizer()
  {
    restore();
  }

  void operator()(stream_type& os)
  {
    auto is_color_string = [](CharT const* s) -> bool {
      char const kEscapeChar = '^';

      return (s && *(s) == kEscapeChar && *(s + 1) &&
              *((s) + 1) != kEscapeChar);
    };

    CharT const* s = text_.c_str();
    string_type cut{};
    while (*s)
    {
      if (is_color_string(s))
      {
        os << cut;
        if (!cut.empty())
          cut.clear();

        std::uint16_t color_num = *(s + 1) - '0';
        if (color_num >= 1 && color_num <= 7)
        {
          set(static_cast<Color>(color_num), true);
          s += 2;
          continue;
        }
        else
        {
          cut += *s;
          s++;
          continue;
        }
      }

      cut += *s;
      s++;
    }

    if (!cut.empty())
    {
      os << cut;
      cut.clear();
    }
  }

private:
  void set(std::uint16_t attr)
  {
    SetConsoleTextAttribute(std_handle_, attr);
  }

  void set(Color color, bool intensify)
  {
    std::uint16_t win_color = 0;
    switch (color)
    {
    case Color::Red:
      win_color = Foreground::Red;
      break;
    case Color::Green:
      win_color = Foreground::Green;
      break;
    case Color::Yellow:
      win_color = Foreground::Yellow;
      break;
    case Color::Blue:
      win_color = Foreground::Blue;
      break;
    case Color::Cyan:
      win_color = Foreground::Cyan;
      break;
    case Color::Magenta:
      win_color = Foreground::Magenta;
      break;
    case Color::White:
      win_color = Foreground::White;
      break;
    }

    if (intensify)
      win_color |= FOREGROUND_INTENSITY;

    SetConsoleTextAttribute(std_handle_, win_color);
  }

  void restore()
  {
    set(old_attrib_);
  }

  string_type text_;
  HANDLE std_handle_;
  std::uint16_t old_attrib_;
};

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os,
                                             basic_colorizer<CharT> colorizer)
{
  colorizer(os);
  return os;
}

template <typename CharT> inline basic_colorizer<CharT> col(CharT const* s)
{
  std::basic_string<CharT> str(s);
  basic_colorizer<CharT> cl(s);

  return cl;
}

/**
 * NOTE: Make sure this only call this once.
 *
 * Use this when you want to ensure that the color doesn't stay
 * on the console longer than the program is alive.
 */
void set_atexit_handler()
{
  static bool has_registered_cb = false;
  static std::uint16_t orig_attr = 0x0;
  HANDLE std_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (!has_registered_cb)
  {
    has_registered_cb = true;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(std_handle, &csbi);
    orig_attr = csbi.wAttributes;

    std::atexit(set_atexit_handler);
  }
  else
  {
    SetConsoleTextAttribute(std_handle, orig_attr);
  }
}
}