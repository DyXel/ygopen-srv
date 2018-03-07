#ifndef __STRING_UTILS_HPP__
#define __STRING_UTILS_HPP__
#include <string>
#include <locale>
#include <codecvt>

// WARNING: these functions are only intended helper
// functions for the legacy server.
// Do not use them outside of the legacy server.

namespace su
{
	static std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> ucs2conv;

	static std::string u16tos(std::u16string u16)
	{
		return ucs2conv.to_bytes(u16);
	}

	static std::u16string stou16(std::string s)
	{
		return ucs2conv.from_bytes(s);
	}
}
#endif // __STRING_UTILS_HPP__
