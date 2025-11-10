#pragma once
#include <string>
#include <stringapiset.h>

namespace StringUtility {
std::wstring ConvertString(const std::string &str);
std::string ConvertString(const std::wstring &wstr);
} // namespace StringUtility
