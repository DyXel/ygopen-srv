#ifndef __STRING_UTILS_HPP__
#define __STRING_UTILS_HPP__
#include <string>
#include <locale>
#include <codecvt>

// WARNING: these functions are only intended helper
// functions for the legacy server.
// Do not use them outside of the legacy server.

namespace su {
#if _MSC_VER >= 1900

static std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> ucs2conv;

static std::string u16tos(const std::u16string& u16) {
	auto p = reinterpret_cast<const int16_t *>(u16.data());
	return ucs2conv.to_bytes(p, p + u16.size());
}

static std::u16string stou16(const std::string& s) {
	return reinterpret_cast<const char16_t *>(ucs2conv.from_bytes(s).data());
}
#else
static std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> ucs2conv;

static std::string u16tos(const std::u16string& u16) {
	return ucs2conv.to_bytes(u16);
}

static std::u16string stou16(const std::string& s) {
	return ucs2conv.from_bytes(s);
}
#endif
}
#endif // __STRING_UTILS_HPP__