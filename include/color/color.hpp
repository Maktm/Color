#pragma once

#include <map>

#if defined(_WIN32)
#include <windows.h>
#else
#error "Unsupported platform in use"
#endif // #if defined(_WIN32)

namespace color
{
	namespace detail
	{
		/* Masks for extracting individual colors. */
		static uint16_t const kForegroundMask = 0x00F0;
		static uint16_t const kBackgroundMask = 0x000F;

		inline HANDLE& GetStdHandleRef()
		{
			static HANDLE std_handle = GetStdHandle(STD_OUTPUT_HANDLE);
			return std_handle;
		}

		inline WORD GetConsoleColor()
		{
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			GetConsoleScreenBufferInfo(GetStdHandleRef(), &csbi);
			return csbi.wAttributes;
		}

		inline WORD GetForegroundColor()
		{
			auto color = GetConsoleColor();
			return color & kForegroundMask;
		}

		inline WORD GetBackgroundColor()
		{
			auto color = GetConsoleColor();
			return color & kBackgroundMask;
		}

		inline void SetConsoleColor(WORD color)
		{
			SetConsoleTextAttribute(GetStdHandleRef(), color);
		}

		inline void SetForegroundColor(WORD color)
		{
			if (color & 0x00F0)
				color /= 0x10; // background color -> foreground color

			auto current_color = GetConsoleColor();
			auto updated_color =
				(current_color & kForegroundMask) | color;
			SetConsoleColor(updated_color);
		}

		inline void SetBackgroundColor(WORD color)
		{
			if (color & 0x000F)
				color *= 0x10; // foreground color -> background color

			auto current_color = GetConsoleColor();
			auto updated_color =
				(current_color & kBackgroundMask) | color;
			SetConsoleColor(updated_color);
		}
	}

#pragma region Color Constants
	/* Foreground Colors */
	constexpr uint16_t kForegroundBlack = 0x0000;
	constexpr uint16_t kForegroundDarkBlue = 0x0001;
	constexpr uint16_t kForegroundDarkGreen = 0x0002;
	constexpr uint16_t kForegroundDarkAqua = 0x0003;
	constexpr uint16_t kForegroundDarkRed = 0x0004;
	constexpr uint16_t kForegroundDarkPurple = 0x0005;
	constexpr uint16_t kForegroundDarkYellow = 0x0006;
	constexpr uint16_t kForegroundWhite = 0x0007;
	constexpr uint16_t kForegroundGray = 0x0008;

	/* Light Foreground Colors */
	constexpr uint16_t kForegroundLBlue = 0x0009;
	constexpr uint16_t kForegroundLGreen = 0x000A;
	constexpr uint16_t kForegroundLAqua = 0x000B;
	constexpr uint16_t kForegroundLRed = 0x000C;
	constexpr uint16_t kForegroundLPurple = 0x000D;
	constexpr uint16_t kForegroundLYellow = 0x000E;
	constexpr uint16_t kForegroundBWhite = 0x000F;

	/* Background Colors */
	constexpr uint16_t kBackgroundBlack = 0x0000;
	constexpr uint16_t kBackgroundDarkBlue = 0x0010;
	constexpr uint16_t kBackgroundDarkGreen = 0x0020;
	constexpr uint16_t kBackgroundDarkAqua = 0x0030;
	constexpr uint16_t kBackgroundDarkRed = 0x0040;
	constexpr uint16_t kBackgroundDarkPurple = 0x0050;
	constexpr uint16_t kBackgroundDarkYellow = 0x0060;
	constexpr uint16_t kBackgroundWhite = 0x0070;
	constexpr uint16_t kBackgroundGray = 0x0080;

	/* Light Background Colors */
	constexpr uint16_t kBackgroundLBlue = 0x0090;
	constexpr uint16_t kBackgroundLGreen = 0x00A0;
	constexpr uint16_t kBackgroundLAqua = 0x00B0;
	constexpr uint16_t kBackgroundLRed = 0x00C0;
	constexpr uint16_t kBackgroundLPurple = 0x00D0;
	constexpr uint16_t kBackgroundLYellow = 0x00E0;
	constexpr uint16_t kBackgroundBWhite = 0x00F0;
#pragma endregion

	enum Color
	{
		/* Foreground Colors */
		Black = kForegroundBlack,
		DarkBlue = kForegroundDarkBlue,
		DarkGreen = kForegroundDarkGreen,
		DarkAqua = kForegroundDarkAqua,
		DarkRed = kForegroundDarkRed,
		DarkPurple = kForegroundDarkPurple,
		DarkYellow = kForegroundDarkYellow,
		White = kForegroundWhite,
		Gray = kForegroundGray,
		LightBlue = kForegroundLBlue,
		LightGreen = kForegroundLGreen,
		LightAqua = kForegroundLAqua,
		LightRed = kForegroundLRed,
		LightPurple = kForegroundLPurple,
		LightYellow = kForegroundLYellow,
		BrightWhite = kForegroundBWhite,

		/* Background Colors */
		BgBlack = kBackgroundBlack,
		BgBlue = kBackgroundDarkBlue,
		BgGreen = kBackgroundDarkGreen,
		BgAqua = kBackgroundDarkAqua,
		BgRed = kBackgroundDarkRed,
		BgPurple = kBackgroundDarkPurple,
		BgYellow = kBackgroundDarkYellow,
		BgWhite = kBackgroundWhite,
		BgGray = kBackgroundGray,
		BgLightBlue = kBackgroundLBlue,
		BgLightGreen = kBackgroundLGreen,
		BgLightAqua = kBackgroundLAqua,
		BgLightRed = kBackgroundLRed,
		BgLightPurple = kBackgroundLPurple,
		BgLightYellow = kBackgroundLYellow,
		BgBrightWhite = kBackgroundBWhite
	};

	class DynamicColorMapper
	{
	public:
		DynamicColorMapper(std::map<uint16_t, Color> const& map)
			: map_(map)
		{
		}

		uint16_t GetIndex(Color color) const
		{
			Color mapped_color;
			bool found = false;
			for (auto it = map_.begin(); it != map_.end(); ++it)
			{
				if (it->second == color)
				{
					mapped_color = it->second;
					found = true;
				}
			}

			if (!found)
				throw std::invalid_argument(
					"Provided color is not available");

			return mapped_color;
		}

		Color GetColor(uint16_t index) const
		{
			return map_.at(index);
		}

		size_t Size() const
		{
			return map_.size();
		}

		bool IsColorPresent(uint16_t dec) const
		{
			return map_.find(dec) != map_.end();
		}

	private:
		std::map<uint16_t, Color> map_;
	};

	inline DynamicColorMapper& GetMappingRef()
	{
		static DynamicColorMapper mapping{
			{
				/* Light Colors */
				{0x0001, LightRed},
				{0x0002, LightGreen},
				{0x0003, LightYellow},
				{0x0004, LightBlue},
				{0x0005, LightAqua},
				{0x0006, LightPurple},
				{0x0007, BrightWhite},

				/* Dark Colors */
				{0x0008, DarkRed},
				{0x0009, DarkGreen},
				{0x000A, DarkYellow},
				{0x000B, DarkBlue},
				{0x000C, DarkAqua},
				{0x000D, DarkPurple},

				/* Colors */
				{0x000E, Gray},
				{0x000F, White},
				{0x0000, Black}
			}
		};

		return mapping;
	}

	inline WORD& GetDefaultFgColorRef()
	{
		static WORD foreground_color = White;
		return foreground_color;
	}

	inline WORD& GetDefaultBgColorRef()
	{
		static WORD background_color = Black;
		return background_color;
	}

#pragma region Escape References
	inline char& GetForegroundEscRef()
	{
		static char foreground_escape = '^';
		return foreground_escape;
	}

	inline char& GetBackgroundEscRef()
	{
		static char background_escape = '*';
		return background_escape;
	}

	inline char& GetForegroundResetOptionRef()
	{
		static char reset_escape = '!';
		return reset_escape;
	}

	inline char& GetBackgroundResetOptionRef()
	{
		static char reset_escape = ':';
		return reset_escape;
	}
#pragma endregion


	/*!
	Format class that takes in a string describing a
	colored output and transforms it into real, colored
	output into the console in combination with std::cout,
	std::cerr, and std::clog.
	*/
	class FormattedString
	{
	public:
		FormattedString(std::string const& format)
			: format_(format)
		{
		}

		template <typename CharT>
		void operator()(std::basic_ostream<CharT>& os) const
		{
			// Disabled for now since it causes an issue with code
			// like the following:
			//
			//   std::cout << LightBlue << fcol(" hello^!");
			//
			// Issue: this doesn't set the default background/foreground
			// unless the user uses the FormattedString class.
			// if (detail::GetForegroundColor() != GetDefaultFgColorRef())
			// 	detail::SetForegroundColor(GetDefaultFgColorRef());
			// 
			// if (detail::GetBackgroundColor() != GetDefaultBgColorRef())
			// 	detail::SetBackgroundColor(GetDefaultBgColorRef());

			std::string buffer("");
			for (auto it = format_.cbegin(); it != format_.cend();)
			{
				if (IsEscapeSeq(it, format_.cend()))
				{
					if (!buffer.empty())
					{
						// Flush buffer to change color context.
						os << buffer.c_str();
						buffer.clear();
					}

					char option_char = tolower(*(it + 1));
					uint16_t option_dec
						= (option_char > '9') ?
						(option_char &~0x20) - 'A' + 10 :
						(option_char - '0');

					auto mapping_ref = GetMappingRef();
					if (mapping_ref.IsColorPresent(option_dec))
					{
						auto mapped_color = mapping_ref.GetColor(option_dec);
						if (*it == GetForegroundEscRef())
							detail::SetForegroundColor(mapped_color);
						else if (*it == GetBackgroundEscRef())
							detail::SetBackgroundColor(mapped_color);

						it += 2;
					}
					else if (option_char == GetForegroundResetOptionRef())
					{
						detail::SetForegroundColor(GetDefaultFgColorRef());
						it += 2;
					}
					else if (option_char == GetBackgroundResetOptionRef())
					{
						detail::SetBackgroundColor(GetDefaultBgColorRef());
						it += 2;
					}
					else
					{
						buffer += *it;
						it++;
					}
				}
				else
				{
					buffer += *it;
					++it;
				}
			}

			if (!buffer.empty())
				os << buffer.c_str();
		}

	private:
		bool IsEscapeSeq(std::string::const_iterator beg,
			std::string::const_iterator end) const
		{
			return (beg != end) &&
				(*beg == GetForegroundEscRef() ||
					*beg == GetBackgroundEscRef());
		}

		std::string format_;
	};

	/*!
	Classes used to differentiate between the two events where
	a user wants to reset the foreground _or_ the background.
	*/
	class ResetForeground;
	class ResetBackground;

	/*!
	Class used for its type for resetting the foreground _or_
	background depending on the type of context passed.
	*/
	template <typename ContextT>
	class ResetColor
	{
	};

	template <typename ContextT>
	inline std::ostream& operator<<(std::ostream& os, ResetColor<ContextT> reset);

	template <typename ContextT>
	inline std::wostream& operator<<(std::wostream& os, ResetColor<ContextT> reset);

	template <>
	inline std::ostream& operator<<<ResetForeground>(
		std::ostream& os,
		ResetColor<ResetForeground> reset)
	{
		detail::SetForegroundColor(GetDefaultFgColorRef());
		return os;
	}

	template <>
	inline std::ostream& operator<<<ResetBackground>(
		std::ostream& os,
		ResetColor<ResetBackground> reset)
	{
		detail::SetBackgroundColor(GetDefaultBgColorRef());
		return os;
	}

	template <>
	inline std::wostream& operator<<<ResetForeground>(
		std::wostream& os,
		ResetColor<ResetForeground> reset)
	{
		detail::SetForegroundColor(GetDefaultFgColorRef());
		return os;
	}

	template <>
	inline std::wostream& operator<<<ResetBackground>(
		std::wostream& os,
		ResetColor<ResetBackground> reset)
	{
		detail::SetBackgroundColor(GetDefaultBgColorRef());
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, Color color)
	{
		if (color >= Black && color <= BrightWhite)
			detail::SetForegroundColor(color);
		else
			detail::SetBackgroundColor(color);
		return os;
	}

	inline std::wostream& operator<<(std::wostream& os, Color color)
	{
		if (color >= Black && color <= BrightWhite)
			detail::SetForegroundColor(color);
		else
			detail::SetBackgroundColor(color);
		return os;
	}

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

	/*!
	Used to reset the color on exit without requiring the use of
	resetfg/resetbg after the last output.
	*/
	inline void ResetOnExit()
	{
		static bool has_registered_cb = false;
		static uint16_t attr = 0x0000;

		if (!has_registered_cb)
		{
			has_registered_cb = true;
			attr = detail::GetConsoleColor();

			atexit(&ResetOnExit);
		}
		else
		{
			detail::SetForegroundColor(
				GetDefaultFgColorRef()
			);
			detail::SetBackgroundColor(
				GetDefaultBgColorRef()
			);
		}
	}

	using fcol = FormattedString;
	using resetfg = ResetColor<ResetForeground>;
	using resetbg = ResetColor<ResetBackground>;
}