// Copyright (C) 2017-2018 Michael Kiros
// Home at http://www.github.com/michaelkiros/color

#ifndef COLOR_DETAIL_LANGUAGE_HPP
#define COLOR_DETAIL_LANGUAGE_HPP

#include <type_traits>
#include <iterator>
#include <cassert>

namespace color
{
	namespace detail
	{
		template <typename T>
		inline constexpr typename std::underlying_type<T>::type underlying_value(T enum_type)
		{
			// Serves as an alternative to explicit type casts from
			// an enum type the underlying type.
			return static_cast<typename std::underlying_type<T>::type>(enum_type);
		}

		enum class language_token_type : char
		{
			caret_keyword = '^',
			open_brace = '{',
			closed_brace = '}'
		};

		struct language_token
		{
			bool operator==(language_token_type rhs) const
			{
				return underlying_value(value) == underlying_value(rhs);
			}

			bool operator!=(language_token_type rhs) const
			{
				return !(*this == rhs);
			}

			language_token_type value;
		};

		struct token
		{
			char value;
		};

		inline bool is_language_token(token const& v)
		{
			auto const is_lang_token = [&v]()
			{
				char lang_tokens[] = "^{}";
				auto beg = std::begin(lang_tokens);
				auto end = std::end(lang_tokens);
				return std::find(beg, end, v.value) != end;
			};

			return is_lang_token();
		}

		inline language_token token_to_language_token(token const& tkn)
		{
			// DANGER: Do not call this function as an external interface.
			// Instead, call it when you _know_ that the token is a langu-
			// age token.
			assert(is_language_token(tkn));

			language_token lang_token;
			switch (tkn.value)
			{
				case underlying_value(language_token_type::caret_keyword):
					lang_token.value = language_token_type::caret_keyword;
					break;

				case underlying_value(language_token_type::open_brace):
					lang_token.value = language_token_type::open_brace;
					break;

				case underlying_value(language_token_type::closed_brace):
					lang_token.value = language_token_type::closed_brace;
					break;
			}

			return lang_token;
		}
	} // namespace detail
} // namespace color

#endif // #ifndef COLOR_LANGUAGE_HPP
