/**
 * @file debug_fnc.h
 * @brief デバッグ関数の宣言
 * @author Gold Smith
 * @date 2023 2024
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <minwindef.h>
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

namespace debug_fnc {
	std::string binary_to_string(uint8_t b);
	std::vector<std::string> boardToString(uint64_t p, uint64_t o, uint64_t m, char cp = 'X', char co = 'C', char cm = 'M', char cv = '#');
	void print_binary(uint8_t b);
	void dout_binary(uint8_t b);
	std::string binary_to_string(uint16_t b);
	void print_binary(uint16_t b);
	void dout_binary(uint16_t b);
	std::string binary_to_string(uint64_t b);
	void print_binary(uint64_t b);
	void dout_binary(uint64_t b);
	void dout(const std::string& str);
	std::string GetErrString(DWORD dw);
	const std::string ErrOut_(
		DWORD dw
		, LPCSTR lpcszFile
		, DWORD dwLine
		, LPCSTR lpcszFunction
		, const std::string &lpszOpMessage = "");
#define EOut ErrOut_(::GetLastError(),__FILE__,__LINE__,__FUNCTION__)
#define ENOut(err_num) ErrOut_(err_num,__FILE__,__LINE__,__FUNCTION__)
#define _MES(s) {::OutputDebugStringA((std::string(__FILE__ "(" STRINGIZE(__LINE__) "):")+std::string(s)+"\r\n").c_str());}
#ifdef _DEBUG
#define _D(s) {::OutputDebugStringA((std::string(__FILE__ "(" STRINGIZE(__LINE__) "):")+std::string(s)+"\r\n").c_str());}
#define _DOB(b){::OutputDebugStringA(__FILE__ "(" STRINGIZE(__LINE__)"):" #b "\r\n");debug_fnc::dout_binary(b);}
#define _DOS(b){::OutputDebugStringA(__FILE__ "(" STRINGIZE(__LINE__)"):" #b "\r\n");debug_fnc::dout_status(b);}
	/// <summary>
/// デバッグ変数名と値を出力。NODEBUG時には評価されない。
/// </summary>
#define _DVV(s) {::OutputDebugStringA((std::string(__FILE__)+ "(" + STRINGIZE(__LINE__) + "):"+ #s + ":" + std::to_string(s) + "\r\n").c_str());}
#else
#define _D(s) __noop
#define _DOB(b) __noop
#define _DOS(b) __noop
#define _DVV(s) __noop
#endif // _DEBUG
}