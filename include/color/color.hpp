#pragma once

#include <ostream>
#include <cctype>
#include <map>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#else
#error "Unsupported platform in use"
#endif // #if defined(_WIN32)

namespace color
{
namespace detail
{
/**
* Call this before instantiating an object of FormattedString type to
* control the console output buffer (target output location).
*/
inline HANDLE GlobalStdHandle(HANDLE std_handle = INVALID_HANDLE_VALUE)
{
	static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (std_handle != INVALID_HANDLE_VALUE)
		handle = std_handle;

	return handle;
}

inline WORD GetCurrentConsoleColor()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GlobalStdHandle(), &csbi);

	return csbi.wAttributes;
}

inline void UpdateConsoleColor(HANDLE std_handle, std::uint16_t color)
{
	SetConsoleTextAttribute(std_handle, color);
}
}

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
public:
	DynamicColorMapper() = default;

	DynamicColorMapper(std::map<std::uint16_t, Color> const& m)
		: map_(m)
	{
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

inline DynamicColorMapper GlobalMapper(DynamicColorMapper mapper = DynamicColorMapper{})
{
	static DynamicColorMapper default_mapping{
		{
			/* Light Foreground Colors */
			{0x0001, LightRed},
			{0x0002, LightGreen},
			{0x0003, LightYellow},
			{0x0004, LightBlue},
			{0x0005, LightAqua},
			{0x0006, LightPurple},
			{0x0007, BrightWhite},

			/* Foreground Colors */
			{0x0008, Red},
			{0x0009, Green},
			{0x000A, Yellow},
			{0x000B, Blue},
			{0x000C, Aqua},
			{0x000D, Purple},
			{0x000E, Gray},
			{0x000F, White},
			{0x0000, Black}
		}
	};

	if (mapper.Size() > 0)
		default_mapping = mapper;

	return default_mapping;
}

inline Color DefaultColor(Color color = static_cast<Color>(0xFF))
{
	static Color default_color = Color::White;

	if (static_cast<std::uint16_t>(color) != 0xFF)
		default_color = color;

	return default_color;
}

/************
* Formatter *
*************/
class FormattedString
{
	char const kForegroundEscape = '^';
	char const kBackgroundEscape = '*'; // For future background colors implementation.

	char const kResetToDefOpt = '!';

public:
	FormattedString(std::string const& buffer)
		: buffer_{buffer},
		  mapper_{GlobalMapper()},
		  std_handle_{detail::GlobalStdHandle()}
	{
	}

	template <typename CharT>
	void operator()(std::basic_ostream<CharT>& os) const
	{
		// Fixes the issue of the default color not being set
		// when no color changes are made by the user.
		if (detail::GetCurrentConsoleColor() != DefaultColor())
			SetConsoleColor(DefaultColor(), false);

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
				std::uint16_t dec = (chr > '9') ? (chr &~0x20) - 'A' + 10 : (chr - '0');

				if (mapper_.IsColorPresent(dec))
				{
					SetConsoleColor(dec);
					iter += 2;
				}
				else if (chr == kResetToDefOpt)
				{
					SetConsoleColor(DefaultColor(), false);
					iter += 2;
				}
				else
				{
					portion += *iter;
					iter++;
				}
			}
			else
			{
				portion += *iter;
				++iter;
			}
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
		detail::UpdateConsoleColor(std_handle_, color);
	}

	bool IsColorEscape(std::string::const_iterator iter, std::string::const_iterator end) const
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
	detail::UpdateConsoleColor(detail::GlobalStdHandle(), DefaultColor());

	return os;
}

inline std::wostream& operator<<(std::wostream& os, ResetColor const& rs)
{
	detail::UpdateConsoleColor(detail::GlobalStdHandle(), DefaultColor());

	return os;
}

inline std::ostream& operator<<(std::ostream& os, Color color)
{
	detail::UpdateConsoleColor(detail::GlobalStdHandle(), color);

	return os;
}

inline std::wostream& operator<<(std::wostream& os, Color color)
{
	detail::UpdateConsoleColor(detail::GlobalStdHandle(), color);

	return os;
}

/*****************
* atexit Handler *
******************/
inline void ResetOnExit()
{
	static bool has_registered_cb = false;
	static std::uint16_t attr = 0x0000;

	if (!has_registered_cb)
	{
		has_registered_cb = true;
		attr = detail::GetCurrentConsoleColor();

		std::atexit(ResetOnExit);
	}
	else
	{
		detail::UpdateConsoleColor(detail::GlobalStdHandle(), attr);
	}
}

using fs = color::FormattedString;
using reset = color::ResetColor;
}
