// Copyright (C) 2017-2018 Michael Kiros
// Home at http://www.github.com/michaelkiros/color

#ifndef COLOR_DETAIL_TOKENIZER_HPP
#define COLOR_DETAIL_TOKENIZER_HPP

#include <string>

#include <color/detail/language.hpp>

// NOTICE: Make sure to make use of the .good() member before
// doing any operations to check if the state of tokenizer
// is good (i.e. containing > 0 elements).

namespace color
{
	namespace detail
	{
		class tokenizer
		{
			using const_iterator = std::string::const_iterator;

		public:
			tokenizer(std::string const& s)
				: s_{s},
				  curr_{s_.cbegin()},
				  last_{s_.cend()},
				  good_{true}
			{
				if (curr_ != last_)
				{
					// Avoid thes issue of initializing the tokenizer
					// with an empty string and cend() - 1 throwing.
					last_ -= 1;
				}
				else
				{
					// An empty string was provided so initialize the
					// tokenizer to an already bad state.
					good_ = false;
				}
			}

			token current() const
			{
				token tkn;
				tkn.value = *curr_;

				return tkn;
			}

			bool next()
			{
				bool performed_next = false;
				if (curr_ != last_)
				{
					++curr_;
					performed_next = true;
				}

				return performed_next;
			}

			bool good() const
			{
				return good_;
			}

			bool is_end() const
			{
				return curr_ == last_;
			}

		private:
			std::string s_;
			const_iterator curr_;
			const_iterator last_;
			bool good_;
		};
	} // namespace detail
} // namespace color

#endif // #ifndef COLOR_DETAIL_TOKENIZER_HPP