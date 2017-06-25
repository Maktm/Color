#pragma once

#include <cassert>
#include <ostream>
#include <cctype>
#include <string>
#include <map>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#error "Unsupported platform in use"
#endif // #if defined(_WIN32)

namespace colorize
{
/********************
* Windows Constants *
*********************/

/* Foreground Colors */
constexpr std::int16_t kForegroundBlack = 0x0000;
constexpr std::int16_t kForegroundBlue = 0x0001;
constexpr std::int16_t kForegroundGreen = 0x0002;
constexpr std::int16_t kForegroundAqua = 0x0003;
constexpr std::int16_t kForegroundRed = 0x0004;
constexpr std::int16_t kForegroundPurple = 0x0005;
constexpr std::int16_t kForegroundYellow = 0x0006;
constexpr std::int16_t kForegroundWhite = 0x0007;
constexpr std::int16_t kForegroundGray = 0x0008;

/* Light Foreground Colors */
constexpr std::int16_t kForegroundLBlue = 0x0009;
constexpr std::int16_t kForegroundLGreen = 0x000A;
constexpr std::int16_t kForegroundLAqua = 0x000B;
constexpr std::int16_t kForegroundLRed = 0x000C;
constexpr std::int16_t kForegroundLPurple = 0x000D;
constexpr std::int16_t kForegroundLYellow = 0x000E;
constexpr std::int16_t kForegroundBWhite = 0x000F;

enum Color : std::int16_t
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

inline Color DefaultColor(Color color = static_cast<Color>(0xFF))
{
	static Color default_color = Color::White;
	if (static_cast<std::int16_t>(color) != 0xFF)
	{
		default_color = color;
	}

	return default_color;
}

class NumberToColorMapper
{
	std::size_t const kAvailableColors = 16;

public:
	NumberToColorMapper() = default;

	NumberToColorMapper(std::map<std::int16_t, Color> const& m)
		: map_(m)
	{
		// Verify that the mapper contains all the colors that
		// need to be defined to prevent any future errors.
		// Check if there's a performance hit.		
#if defined(COLORIZE_ENABLE_MAPPING_CHECK)
		std::size_t cnt = 0;
		std::vector<Color> cnted;
		for (auto iter = map_.begin(); iter != map_.end(); ++iter)
		{
			if (std::find(cnted.begin(), cnted.end(), iter->second) == cnted.end())
			{
				cnted.push_back(iter->second);
			}
		}

		assert(cnted.size() == kAvailableColors);
#endif
	}

	NumberToColorMapper& operator=(NumberToColorMapper const& rhs)
	{
		map_ = rhs.map_;
		return *this;
	}


	Color GetColor(std::int16_t idx) const
	{
		assert(idx > 0x0000 && idx <= map_.size());
		return map_.at(idx);
	}

	std::size_t Size() const
	{
		return map_.size();
	}

private:
	std::map<std::int16_t, Color> map_;
};

inline NumberToColorMapper GlobalMapper(NumberToColorMapper tcm = NumberToColorMapper{})
{
	static NumberToColorMapper default_mapping{
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

class FormattedString
{
	char const kForegroundEscape = '^';
	char const kBackgroundEscape = '*';
	char const kResetToDefEscape = '!';

public:
	FormattedString(char const* buffer)
		: buffer_{buffer},
		mapper_{GlobalMapper()}
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
				// Output the string saved so far.
				if (!portion.empty())
				{
					os << portion.c_str();
					portion.clear();
				}

				// Check this for errors.
				char chr = std::tolower((iter + 1)[0]);
				auto dec = (chr > '9') ? (chr &~0x20) - 'A' + 10 : (chr - '0');;

				if (dec >= 0x0000 && dec <= 0x000F)
				{
					SetConsoleColor(dec);
					iter += 2;
					continue;
				}
				else if (chr == kResetToDefEscape)
				{
					SetConsoleColor(DefaultColor(), false);
					iter += 2;
					continue;
				}
				else
				{
					portion += *iter;
					iter++;
					continue;
				}
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
	void SetConsoleColor(std::int16_t idx, bool use_mapper = true) const
	{
		auto color = use_mapper ? mapper_.GetColor(idx) : idx;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}

	bool IsHex(char c) const
	{
		return c >= '0' && c <= 'f';
	}

	bool IsColorEscape(std::string::const_iterator iter, std::string::const_iterator end) const
	{
		return (iter != end && (*iter == kForegroundEscape || *iter == kBackgroundEscape));
	}

	std::string buffer_;
	NumberToColorMapper mapper_{GlobalMapper()};
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
inline void SetAtexitHandler()
{
	static bool has_registered_cb = false;
	static std::int16_t attr = 0x0000;

	HANDLE std_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!has_registered_cb)
	{
		has_registered_cb = true;

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(std_handle, &csbi);
		attr = csbi.wAttributes;

		std::atexit(SetAtexitHandler);
	}
	else
	{
		SetConsoleTextAttribute(std_handle, attr);
	}
}

using format = colorize::FormattedString;
using reset = colorize::ResetColor;
}