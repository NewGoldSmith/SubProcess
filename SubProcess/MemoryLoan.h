/**
 * @file MemoryLoan.h
 * @brief テンプレートメモリプールクラスの実装
 * @author Gold Smith<br>
 * @date 2022-2024<br>
 * @copyright SPDX-License-Identifier: MIT<br>
 * Released under the MIT license<br>
 * https: //opensource.org/licenses/mit-license.php<br>
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */

#pragma once
 /// @attention このクラスはWindows専用です。<br>
 /// 異常が検出された場合、必ず例外が発生します。

// ********使用条件を設定**************************

/// @def ML_USING_CRITICAL_SECTION
/// @brief クリティカルセクションを使用する場合
#define ML_USING_CRITICAL_SECTION

/// @def ML_CONFIRM_RANGE
/// @brief 範囲外確認を行う場合。
#define ML_CONFIRM_RANGE

/// @def ML_USING_DEBUG_OUT
/// @brief デバッグ出力をする場合。
#define ML_USING_DEBUG_OUT

/// @def ML_USING_STD_ERROR
/// @brief std::cerr出力をする場合。
#define ML_USING_STD_ERROR

// **********条件設定終わり************************

#include <memory>
#include <string>
#include <exception>
#include <iostream>
#include <sstream>

#ifdef ML_USING_DEBUG_OUT
#include <debugapi.h>
#endif // ML_USING_DEBUG_OUT
#if defined(ML_CONFIRM_RANGE) || defined(ML_USING_DEBUG_OUT) || defined(ML_USING_STD_ERROR)
#include <algorithm>
#endif
#ifdef ML_USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // ML_USING_CRITICAL_SECTION

template <typename T>
class MemoryLoan {
public:
	MemoryLoan() = delete;
	// @attention sizeInは2のべき乗でなくてはなりません。
	MemoryLoan(T *const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, front(0)
		, end(0)
#if defined(ML_CONFIRM_RANGE) || defined(ML_USING_DEBUG_OUT) || defined(ML_USING_STD_ERROR)
		, max_using(0)
#endif
		, mask(sizeIn - 1)
#ifdef ML_USING_CRITICAL_SECTION
		, cs{}
#endif // ML_USING_CRITICAL_SECTION

	{
		if ((sizeIn & (sizeIn - 1)) != 0) {
			std::string estr("The number of units for MemoryLoan must be specified as a power of 2.\r\n");
#ifdef ML_USING_DEBUG_OUT
			::OutputDebugStringA(estr.c_str());
#endif// ML_USING_DEBUG_OUT
#ifdef ML_USING_STD_ERROR
			std::cerr << estr;
#endif // ML_USING_STD_ERROR
			throw std::invalid_argument(estr);
		}
#ifdef ML_USING_CRITICAL_SECTION
		(void)::InitializeCriticalSection(&cs);
#endif // ML_USING_CRITICAL_SECTION
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryLoan(const MemoryLoan &) = delete;
	MemoryLoan(const MemoryLoan &&)noexcept = delete;
	MemoryLoan &operator ()(const MemoryLoan &) = delete;
	MemoryLoan &operator =(const MemoryLoan &) = delete;
	MemoryLoan &operator ()(MemoryLoan &&) = delete;
	MemoryLoan &operator =(MemoryLoan &&) = delete;
	~MemoryLoan() {
#ifdef ML_USING_DEBUG_OUT
		std::stringstream ss;
		ss << "MemoryLoan is destructing."
			<< " DebugMessage:" << "\"" << strDebug << "\""
			<< " TypeName:" << "\"" << typeid(T).name() << "\""
			<< " BytesPerUnit:" << sizeof(T) << "bytes"
			<< " TotalNumberOfLoans:" << std::to_string(end)
			<< " TotalNumberOfReturns:" << std::to_string(front)
			<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
			<< " NumberOfUnits:" << std::to_string(mask + 1)
			<< " MaximumNumberOfLoans:" << std::to_string(max_using)
			<< "\r\n";
		::OutputDebugStringA(ss.str().c_str());
#endif // ML_USING_DEBUG_OUT

#ifdef ML_USING_CRITICAL_SECTION
		::DeleteCriticalSection(&cs);
#endif // ML_USING_CRITICAL_SECTION
		delete[] ppBuf;
	}

	// @attention sizeInは2のべき乗でなくてはなりません。
	void ReInitialized(T *const pBufIn, size_t sizeIn) {
#ifdef ML_USING_DEBUG_OUT
		std::stringstream ss;
		ss << "MemoryLoan reinitialized."
			<< " DebugMessage:" << "\"" << strDebug << "\""
			<< " TypeName:" << "\"" << typeid(T).name() << "\""
			<< " BytesPerUnit:" << sizeof(T) << "bytes"
			<< " TotalNumberOfLoans:" << std::to_string(end)
			<< " TotalNumberOfReturns:" << std::to_string(front)
			<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
			<< " NumberOfUnits:" << std::to_string(mask + 1)
			<< " MaximumNumberOfLoans:" << std::to_string(max_using)
			<< "\r\n";
		::OutputDebugStringA(ss.str().c_str());
#endif // ML_USING_DEBUG_OUT
		delete[] ppBuf;
		front = 0;
		end = 0;
		mask = sizeIn - 1;
		if ((sizeIn & (sizeIn - 1)) != 0) {
			std::string estr("The number of units for MemoryLoan must be specified as a power of 2.\r\n");
#ifdef ML_USING_DEBUG_OUT
			::OutputDebugStringA(estr.c_str());
#endif// ML_USING_DEBUG_OUT
#ifdef ML_USING_STD_ERROR
			std::cerr << estr;
#endif // ML_USING_STD_ERROR
			throw std::invalid_argument(estr);
		}
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}

	inline T *Lend() {
#ifdef ML_USING_CRITICAL_SECTION
		std::unique_ptr< ::CRITICAL_SECTION, decltype(::LeaveCriticalSection) *> qcs
			= { [&]() {::EnterCriticalSection(&cs); return &cs; }(),::LeaveCriticalSection };
#endif // ML_USING_CRITICAL_SECTION
#ifdef ML_CONFIRM_RANGE
		if ((front + mask) < end) {
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "):"
				<< "Loans will soon surpass units."
				<< " DebugMessage:" << "\"" << strDebug << "\""
				<< " TypeName:" << "\"" << typeid(T).name() << "\""
				<< " BytesPerUnit:" << sizeof(T) << "bytes"
				<< " TotalNumberOfLoans:" << std::to_string(end)
				<< " TotalNumberOfReturns:" << std::to_string(front)
				<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
				<< " NumberOfUnits:" << std::to_string(mask + 1)
				<< " MaximumNumberOfLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef ML_USING_STD_ERROR
			std::cerr << ss.str();
#endif // ML_USING_STD_ERROR
#ifdef ML_USING_DEBUG_OUT
			::OutputDebugStringA(ss.str().c_str());
#endif// ML_USING_DEBUG_OUT
			throw std::out_of_range(ss.str().c_str()); // 例外送出
		}
#endif // ML_CONFIRM_RANGE
		T **ppT = &ppBuf[end & mask];
		++end;
#if defined(ML_CONFIRM_RANGE) || defined(ML_USING_DEBUG_OUT) || defined(ML_USING_STD_ERROR)
		max_using = std::max<size_t>(end - front, max_using);
#endif
		return *ppT;
	}

	inline void Return(T *const pT) {
#ifdef ML_USING_CRITICAL_SECTION
		std::unique_ptr< ::CRITICAL_SECTION, decltype(::LeaveCriticalSection) *> qcs
			= { [&]() {::EnterCriticalSection(&cs); return &cs; }(),::LeaveCriticalSection };
#endif // ML_USING_CRITICAL_SECTION
#ifdef ML_CONFIRM_RANGE
		if ((front + 1) > end) {
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "):"
				<< " Returns exceed loans."
				<< " DebugMessage:" << "\"" << strDebug << "\""
				<< " TypeName:" << "\"" << typeid(T).name() << "\""
				<< " BytesPerUnit:" << sizeof(T) << "bytes"
				<< " TotalNumberOfLoans:" << std::to_string(end)
				<< " TotalNumberOfReturns:" << std::to_string(front)
				<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
				<< " NumberOfUnits:" << std::to_string(mask + 1)
				<< " MaximumNumberOfLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef ML_USING_STD_ERROR
			std::cerr << ss.str();
#endif // ML_USING_STD_ERROR
#ifdef ML_USING_DEBUG_OUT
			::OutputDebugStringA(ss.str().c_str());
#endif// ML_USING_DEBUG_OUT
			throw std::out_of_range(ss.str().c_str()); // 例外送出
		}
#endif // ML_CONFIRM_RANGE
		ppBuf[front & mask] = pT;
		++front;
	}

	void DebugString(const std::string &str) {
#if defined(ML_CONFIRM_RANGE) ||defined(ML_USING_DEBUG_OUT) || defined(ML_USING_STD_ERROR)
		strDebug = str;
#endif
	}

protected:
	T **ppBuf;
	size_t front;
	size_t end;
	size_t mask;
#if defined(ML_CONFIRM_RANGE) ||defined(ML_USING_DEBUG_OUT) || defined(ML_USING_STD_ERROR)
	size_t max_using;
	std::string strDebug;
#endif

#ifdef ML_USING_CRITICAL_SECTION
	::CRITICAL_SECTION cs;
#endif // ML_USING_CRITICAL_SECTION

};

#undef ML_USING_CRITICAL_SECTION
#undef ML_CONFIRM_RANGE
#undef ML_USING_DEBUG_OUT
#undef ML_USING_STD_ERROR
