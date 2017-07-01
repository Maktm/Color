#pragma once

#include <cassert>
#include <ostream>
#include <cctype>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#else
#error "Unsupported platform in use"
#endif // #if defined(_WIN32)

namespace color
{
/********************
* Windows Constants *
*********************/

/* Foreground Colors */
constexpr std::uint16_t kForegroundBlack = 0x0000;
constexpr std::uint16_t kForegroundBlue = 0x0001;
constexpr std::uint16_t kForegroundGreen = 0x0002;
constexpr std::uint16_t kForegroundAqua = 0x0003;
constexpr std::uint16_t kForegroundRed = 0x0004;
constexpr std::uint16_t kForegroundPurple = 0x0005;
constexpr std::uint16_t kForegroundYellow = 0x0006;
constexpr std::uint16_t kForegroundWhite = 0x0007;
constexpr std::uint16_t kForegroundGray = 0x0008;

/* Light Foreground Colors */
constexpr std::uint16_t kForegroundLBlue = 0x0009;
constexpr std::uint16_t kForegroundLGreen = 0x000A;
constexpr std::uint16_t kForegroundLAqua = 0x000B;
constexpr std::uint16_t kForegroundLRed = 0x000C;
constexpr std::uint16_t kForegroundLPurple = 0x000D;
constexpr std::uint16_t kForegroundLYellow = 0x000E;
constexpr std::uint16_t kForegroundBWhite = 0x000F;

enum Color : std::uint16_t
{
	Black = kForegroundBlack,
	Blue = kForegroundBlue,
	Green = kForegroundGreen,
	Aqua = kForegroundAqua,
	Red = kForegroundRed,
	Purple = kForegroundPurple,
	Yellow = kForegroundYellow,
	White = kForegroundWhite,
	Gray = kForegroundGray,
	LightBlue = kForegroundLBlue,
	LightGreen = kForegroundLGreen,
	LightAqua = kForegroundLAqua,
	LightRed = kForegroundLRed,
	LightPurple = kForegroundLPurple,
	LightYellow = kForegroundLYellow,
	BrightWhite = kForegroundBWhite
};

/****************
* Color Mapping *
*****************/

class DynamicColorMapper
{
	std::size_t const kAvailableColors = 16;

public:
	DynamicColorMapper() = default;

	DynamicColorMapper(std::map<std::uint16_t, Color> const& m)
		: map_(m)
	{
	}

	DynamicColorMapper& operator=(DynamicColorMapper const& rhs)
	{
		map_ = rhs.map_;
		return *this;
	}


	Color GetColor(std::uint16_t idx) const
	{
		return map_.at(idx);
	}

	std::size_t Size() const
	{
		return map_.size();
	}

	bool IsColorPresent(std::uint16_t dec) const
	{
		return map_.find(dec) != map_.end();
	}

private:
	std::map<std::uint16_t, Color> map_;
};

inline DynamicColorMapper GlobalMapper(DynamicColorMapper tcm = DynamicColorMapper{})
{
	static DynamicColorMapper default_mapping{
		{
			/* Light Foreground Colors */
			{0x0001, Color::LightRed},
			{0x0002, Color::LightGreen},
			{0x0003, Color::LightYellow},
			{0x0004, Color::LightBlue},
			{0x0005, Color::LightAqua},
			{0x0006, Color::LightPurple},
			{0x0007, Color::BrightWhite},

			/* Foreground Colors */
			{0x0008, Color::Red},
			{0x0009, Color::Green},
			{0x000A, Color::Yellow},
			{0x000B, Color::Blue},
			{0x000C, Color::Aqua},
			{0x000D, Color::Purple},
			{0x000E, Color::Gray},
			{0x000F, Color::White},
			{0x0000, Color::Black}
		}
	};

	if (tcm.Size() > 0)
	{
		default_mapping = tcm;
	}

	return default_mapping;
}

inline Color DefaultColor(Color color = static_cast<Color>(0xFF))
{
	static Color default_color = Color::White;

	if (static_cast<std::uint16_t>(color) != 0xFF)
	{
		default_color = color;
	}

	return default_color;
}

/**
* Call this before instantiating an object of this class type to
* control the console output buffer (target output location).
*/
inline HANDLE GlobalStdHandle(HANDLE std_handle = INVALID_HANDLE_VALUE)
{
	static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (std_handle != INVALID_HANDLE_VALUE)
		handle = std_handle;

	return handle;
}

/************
* Formatter *
*************/
class FormattedString
{
	char const kForegroundEscape = '^';
	char const kBackgroundEscape = '*'; // For future background colors implementation.
	char const kResetToDefEscape = '!';

public:
	FormattedString(std::string const& buffer)
		: buffer_{buffer},
		  mapper_{GlobalMapper()},
		  std_handle_{GlobalStdHandle()}
	{
	}

	template <typename CharT>
	void operator()(std::basic_ostream<CharT>& os) const
	{
		std::string portion{""};
		for (auto iter = buffer_.begin(); iter != buffer_.end();)
		{
			if (IsColorEscape(iter, buffer_.end()))
			{
				if (!portion.empty())
				{
					// Flush everything built so far so we can build
					// a string with a different color.
					os << portion.c_str();
					portion.clear();
				}

				char chr = std::tolower((iter + 1)[0]);
				std::uint16_t dec = (chr > '9') ? (chr &~0x20) - 'A' + 10 : (chr - '0');;

				if (mapper_.IsColorPresent(dec))
				{
					SetConsoleColor(dec);
					iter += 2;
				}
				else if (chr == kResetToDefEscape)
				{
					SetConsoleColor(DefaultColor(), false);
					iter += 2;
				}
				else
				{
					portion += *iter;
					iter++;
				}

				continue;
			}

			portion += *iter;
			++iter;
		}

		if (!portion.empty())
		{
			os << portion.c_str();
			portion.clear();
		}
	}

private:
	void SetConsoleColor(std::uint16_t idx, bool use_mapper = true) const
	{
		auto color = use_mapper ? mapper_.GetColor(idx) : idx;
		SetConsoleTextAttribute(std_handle_, color);
	}

	inline bool IsHex(char c) const
	{
		return c >= '0' && c <= 'f';
	}

	inline bool IsColorEscape(std::string::const_iterator iter, std::string::const_iterator end) const
	{
		return (iter != end && (*iter == kForegroundEscape || *iter == kBackgroundEscape));
	}

	std::string buffer_;
	DynamicColorMapper mapper_;
	HANDLE std_handle_;
};

/*****************
* Color Resetter *
******************/
struct ResetColor
{
};

/**********************
* <ostream> Overloads *
***********************/

inline std::ostream& operator<<(std::ostream& os, FormattedString const& fs)
{
	fs(os);
	return os;
}

inline std::wostream& operator<<(std::wostream& os, FormattedString const& fs)
{
	fs(os);
	return os;
}

inline std::ostream& operator<<(std::ostream& os, ResetColor const& rs)
{
	FormattedString fs("^!");
	fs(os);
	return os;
}

inline std::wostream& operator<<(std::wostream& os, ResetColor const& rs)
{
	FormattedString fs("^!");
	fs(os);
	return os;
}

/*****************
* atexit Handler *
******************/
WORD GetCurrentConsoleColor()
{
	HANDLE std_handle = GlobalStdHandle();
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(std_handle, &csbi);

	return csbi.wAttributes;
}

inline void ResetOnExit()
{
	static bool has_registered_cb = false;
	static std::uint16_t attr = 0x0000;

	HANDLE std_handle = GlobalStdHandle();
	if (!has_registered_cb)
	{
		has_registered_cb = true;
		attr = GetCurrentConsoleColor();

		std::atexit(ResetOnExit);
	}
	else
	{
		SetConsoleTextAttribute(std_handle, attr);
	}
}

using format = color::FormattedString;
using reset = color::ResetColor;
}
