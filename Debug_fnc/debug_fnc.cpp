/**
 * @file debug_fnc.cpp
 * @brief デバッグ関数の実装
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "debug_fnc.h"

std::string debug_fnc::binary_to_string(uint8_t b)
{
	std::stringstream ss;
	for (int j = 7; j >= 0; --j) {
		ss << (b >> j & 1) << " ";
	}
	return ss.str().c_str();
}

std::vector<std::string> debug_fnc::boardToString(
	uint64_t p
	, uint64_t o
	, uint64_t m
	, char cp
	, char co
	, char cm
	, char cv) {
	std::vector<std::string> result(8);
	for (int i = 63; i >= 0; --i) {
		if (p & (1ULL << i)) {
			result[i / 8] += cp;
		}
		else if (o & (1ULL << i)) {
			result[i / 8] += co;
		}
		else if (m & (1ULL << i)) {
			result[i / 8] += cm;
		}
		else {
			result[i / 8] += cv;
		}
		if (i % 8 != 0) {
			result[i / 8] += ' ';
		}
	}
	return result;
}

void debug_fnc::print_binary(uint8_t b)
{
	std::cout << binary_to_string(b) << std::endl;
}

void debug_fnc::dout_binary(uint8_t b)
{
	OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

std::string debug_fnc::binary_to_string(uint16_t b)
{
	std::stringstream ss;
	ss << binary_to_string(((uint8_t*)&b)[1]);
	ss << std::endl;
	ss << binary_to_string(((uint8_t*)&b)[0]);
	return ss.str().c_str();
}

void debug_fnc::print_binary(uint16_t b)
{
	std::cout << binary_to_string(b) << std::endl;
}

void debug_fnc::dout_binary(uint16_t b)
{
	OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

std::string debug_fnc::binary_to_string(uint64_t b)
{
	std::stringstream ss;
	for (int j = 7; j >= 0; --j) {
		ss << binary_to_string(((uint8_t*)&b)[j]);
		if (j == 0) {
			break;
		}
		ss << std::endl;
	}
	return ss.str().c_str();
}

void debug_fnc::print_binary(uint64_t b)
{
	std::cout << binary_to_string(b) << std::endl;
}

std::string debug_fnc::_d(const std::string strFile, const std::string strLine, const std::string str){
	std::string dstr = strFile + "(" + strLine + "):" + str + "\r\n";
	::OutputDebugStringA(dstr.c_str());
	return dstr;
}

void debug_fnc::dout_binary(uint64_t b)
{
	::OutputDebugStringA((binary_to_string(b) + "\r\n").c_str());
}

std::string debug_fnc::dout(const std::string& str)
{
	::OutputDebugStringA((str + "\r\n").c_str());
	return str;
}

std::string debug_fnc::GetErrString(DWORD dw)
{
		LPVOID lpMsgBuf;

		::FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_IGNORE_INSERTS
			| FORMAT_MESSAGE_MAX_WIDTH_MASK
			, NULL
			, dw
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, (LPSTR)&lpMsgBuf
			, 0, NULL);
		std::string str=(char*)lpMsgBuf;
		::LocalFree(lpMsgBuf);
		return str;
}

const std::string debug_fnc::ErrOut_(DWORD dw, LPCSTR lpcszFile, DWORD dwLine, LPCSTR lpcszFunction, const std::string &lpszOpMessage) {
	const std::string strOpMessage = lpszOpMessage.empty() ?
		"" :
		"User Message:\"" + lpszOpMessage + "\"";
	std::stringstream ss;
	ss << lpcszFile << "(" << dwLine << "):error C" << dw << ":"\
		<< GetErrString(dw)
		<< "function name:" << lpcszFunction
		<< " " << strOpMessage << "\r\n";
	::OutputDebugStringA(ss.str().c_str());
	return ss.str().c_str();
}
