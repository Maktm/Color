// Copyright (C) 2017-2018 Michael Kiros
// Home at http://www.github.com/michaelkiros/color

#ifndef COLOR_COLOR_HPP
#define COLOR_COLOR_HPP

#include <ostream>
#include <string>
#include <cctype>

#include <color/detail/tokenizer.hpp>
#include <color/detail/language.hpp>

namespace color
{
	class color
	{
		friend std::ostream& operator<<(std::ostream&, color const&);

	public:
		color(std::string const& s)
			: s_{s}
		{
		}

	private:
		std::string s_;
	};

	enum class processing_flags : std::int32_t
	{
		searching_for_close_brace,
		searching_for_caret_args,
		none_of_the_above
	};

	inline std::ostream& operator<<(std::ostream& os, color const& color)
	{
		detail::tokenizer tokenizer(color.s_);
		if (tokenizer.good())
		{
			std::string buffer;
			std::string buffer_for_braces;
			processing_flags state = processing_flags::none_of_the_above;
			do
			{
				auto token = tokenizer.current();
				switch (token.value)
				{
					case detail::underlying_value(
						detail::language_token_type::caret_keyword):
					{
						if (state == processing_flags::searching_for_caret_args)
						{
							// Collapse.
							buffer.push_back(detail::underlying_value(
								detail::language_token_type::caret_keyword));
							state = processing_flags::none_of_the_above;
						} else
						{
							state = processing_flags::searching_for_caret_args;
						}
					}break;

					case detail::underlying_value(
						detail::language_token_type::open_brace):
					{
						if (state == processing_flags::searching_for_close_brace)
						{
							// If we've already opened the braces then this part
							// is just part of the inner formatting.
							buffer.push_back(token.value);
						} else
						{
							state = processing_flags::searching_for_close_brace;
						}
					}break;

					case detail::underlying_value(
						detail::language_token_type::closed_brace):
					{
						if (state != processing_flags::searching_for_close_brace)
						{
							// Don't really have to do anything because we haven't
							// opened up the brace for formatting.
							buffer.push_back(token.value);
						} else
						{
							// We're closing an open brace. Take the contents in the
							// brace buffer and parse it.
							if (buffer_for_braces.size() > 0)
							{
								// Parse the data and change the color as needed.
								// As in just take the number inside.
								buffer.append("[COLOR]");
							}
							state = processing_flags::none_of_the_above;
						}
					}break;

					default:
					{
						if (state == processing_flags::searching_for_caret_args &&
							std::isdigit(token.value))
						{
							// Change the color.
						} else if (state == processing_flags::searching_for_close_brace)
						{
							// Push back the contents inside of the braces.
							buffer_for_braces.push_back(token.value);
						} else
						{
							buffer.push_back(token.value);
						}
					}
				}
			} while (tokenizer.next());

			os << buffer;
			os.flush();
		}

		return os;
	}
}

#endif // #ifndef COLOR_COLOR_HPP