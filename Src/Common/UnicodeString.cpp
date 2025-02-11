// SPDX-License-Identifier: GPL-2.0-or-later
/** 
 * @file  UnicodeString.cpp
 *
 * @brief String utilities.
 */

// String formatting code originally from Paul Senzee:
// http://www.senzee5.com/2006/05/c-formatting-stdstring.html

#include "pch.h"
#include "UnicodeString.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>

namespace strutils
{

/**
 * @brief Convert a string to lower case string.
 * @param [in] str String to convert to lower case.
 * @return Lower case string.
 */
String makelower(const String &str)
{
	String ret(str);
	String::size_type i = 0;
	for (i = 0; i < ret.length(); i++)
		ret[i] = _totlower(ret[i]);
	return ret;
}

/**
 * @brief Convert a string to upper case string.
 * @param [in] str String to convert to upper case.
 * @return upper case string.
 */
String makeupper(const String &str)
{
	String ret(str);
	String::size_type i = 0;
	for (i = 0; i < ret.length(); i++)
		ret[i] = _totupper(ret[i]);
	return ret;
}

String strip_hot_key(const String& str)
{
	String str2 = str;
	auto it = str2.find(_T("(&"));
	if (it != String::npos)
		str2.erase(it, it + 2);
	strutils::replace(str2, _T("&"), _T(""));
	return str2;
}

TCHAR from_charstr(const String& str)
{
	TCHAR ch = 0;
	String str2 = strutils::makelower(str);
	strutils::replace(str2, _T("-"), _T(""));
	if (str2 == _T("\\a") || str2 == _T("bel"))
		ch = '\a';
	else if (str2 == _T("\\b") || str2 == _T("bs"))
		ch = '\b';
	else if (str2 == _T("\\f") || str2 == _T("ff"))
		ch = '\f';
	else if (str2 == _T("\\n") || str2 == _T("lf"))
		ch = '\n';
	else if (str2 == _T("\\r") || str2 == _T("cr"))
		ch = '\r';
	else if (str2 == _T("\\t") || str2 == _T("tab"))
		ch = '\t';
	else if (str2 == _T("\\v") || str2 == _T("vt"))
		ch = '\v';
	else if (str2 == _T("\\'") || str2 == _T("sq") || str2 == _T("singlequote"))
		ch = '\'';
	else if (str2 == _T("\\\"") || str2 == _T("dq") || str2 == _T("doublequote"))
		ch = '"';
	else if (str2.find(_T("\\x"), 0) == 0 || str2.find(_T("0x"), 0) == 0)
	{
		TCHAR *pend = nullptr;
		ch = static_cast<TCHAR>(_tcstol(str2.substr(2).c_str(), &pend, 16));
	}
	else
		ch = str.c_str()[0];
	return ch;
}

String to_charstr(TCHAR ch)
{
	if (iscntrl(ch))
		return strutils::format(_T("\\x%02x"), ch);
	return String(1, ch);
}

/**
 * @brief Replace a string inside a string with another string.
 * This function searches for a string inside another string an if found,
 * replaces it with another string. Function can replace several instances
 * of the string inside one string.
 * @param [in] target A string containing another string to replace.
 * @param [in] find A string to search and replace with another (@p replace).
 * @param [in] replace A string used to replace original (@p find).
 */
void replace(String &target, const String &find, const String &replace)
{
	const String::size_type find_len = find.length();
	const String::size_type replace_len = replace.length();
	String::size_type pos = 0;
	while ((pos = target.find(find, pos)) != String::npos)
	{
		target.replace(pos, find_len, replace);
		pos += replace_len;
	}
}

/**
 * @brief Compare two strings ignoring the character casing.
 * @param [in] str1 First string to compare.
 * @param [in] str2 Second string to compare.
 * @return As strcmp(), 0 if strings match.
 */
int compare_nocase(const String &str1, const String &str2)
{
	return _tcsicoll(str1.c_str(), str2.c_str());
}

/**
 * @brief Trims whitespace chars from begin and end of the string.
 * @param [in] str the original string.
 * @return Trimmed string.
 */
String trim_ws(const String & str)
{
	if (str.empty())
		return str;

	String result(str);
	String::iterator it = result.begin();
	while (it != result.end() && _istspace(*it))
		++it;
	
	if (it != result.begin())
		result.erase(result.begin(), it);

	if (result.empty())
		return result;

	it = result.end() - 1;
	while (it != result.begin() && _istspace(*it))
		--it;

	if (it != result.end() - 1)
		result.erase(it + 1, result.end());
	return result;
}

/**
 * @brief Trims whitespace chars from begin of the string.
 * @param [in] str the original string.
 * @return Trimmed string.
 */
String trim_ws_begin(const String & str)
{
	if (str.empty())
		return str;

	String result(str);
	String::iterator it = result.begin();
	while (it != result.end() && _istspace(*it))
		++it;
	
	if (it != result.begin())
		result.erase(result.begin(), it);
	return result;
}

/**
 * @brief Trims whitespace chars from end of the string.
 * @param [in] str the original string.
 * @return Trimmed string.
 */
String trim_ws_end(const String & str)
{
	if (str.empty())
		return str;

	String result(str);
	String::reverse_iterator it = result.rbegin();
	while (it != result.rend() && _istspace(*it))
		++it;

	if (it != result.rbegin())
		result.erase(it.base(), result.end());
	return result;
}

String format_arg_list(const TCHAR *fmt, va_list args)
{
	if (fmt == nullptr)
		return _T("");
	int result = -1;
	int length = 256;
	std::vector<TCHAR> buffer(length, 0);
	while (result == -1)
	{
		result = _vsntprintf_s(&buffer[0], length, _TRUNCATE, fmt, args);
		length *= 2;
		buffer.resize(length, 0);
	}
	String s(&buffer[0]);
	return s;
}

/**
 * @brief printf()-style formatting for STL string.
 * Use this function to format String:s in printf() style.
 */
String format_varg(const TCHAR *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String s = format_arg_list(fmt, args);
	va_end(args);
	return s;
}

String format_strings(const String& fmt, const String *args[], size_t nargs)
{
	String str;
	str.reserve(fmt.length() * 2);
	String::const_iterator it;
	for (it = fmt.begin(); it != fmt.end(); ++it)
	{
		if (*it == '%')
		{
			++it;
			if (it == fmt.end())
				break;
			int n = *it - '0';
			if (n > 0 && static_cast<unsigned int>(n) <= nargs)
				str += *args[n - 1];
			else
				str += *it;
		}
		else
		{
			str += *it;
		}
	}
	return str;
}

String format_string1(const String& fmt, const String& arg1)
{
	const String* args[] = {&arg1};
	return format_strings(fmt, args, 1);
}

String format_string2(const String& fmt, const String& arg1, const String& arg2)
{
	const String* args[] = {&arg1, &arg2};
	return format_strings(fmt, args, 2);
}

}
