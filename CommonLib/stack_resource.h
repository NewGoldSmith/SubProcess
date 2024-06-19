/**
 * @file stack_resource.hpp
 * @brief スタック領域からメモリを割り当てるmemory_resource実装
 * @author cpprefjp<br>
 * @anchor https:// cpprefjp.github.io/reference/memory_resource/memory_resource.html
 * @copyright This work is licensed under a Creative Commons Attribution 3.0 Unported License (CC BY 3.0 DEED).<br>
 * To view a copy of this license, visit https:// creativecommons.org/licenses/by/3.0/deed.ja<br>
 * SPDX-Identifier CC BY 3.0 DEED<br>
 * @date 2024
 * @author modified by Gold Smith
 */
#include <iostream>
#include <memory_resource>
#include <cstddef>
#pragma once

namespace cc30 {
   /// <summary>
   /// スタック領域からメモリを割り当てる
   /// </summary>
   /// <typeparam name="N">スタック領域のサイズ</typeparam>
   template<std::size_t N>
   struct stack_resource : public std::pmr::memory_resource {
 /**
  * @section 使用例
  *
  * @code{ .cpp }
  * #include "stack_resource.hpp"
  * #include <vector>
  *
  * void example() {
  *    cc30::stack_resource<100> s{};
  *    std::pmr::memory_resource* mr = &s;
  *    std::vector<int, std::pmr::polymorphic_allocator<int>> vi = { {1,2,3},{mr} };
  *    for (const auto& i : vi) {
  *       std::cout << i << std::endl;
  *    }
  * }
  * @endcode
  */

      stack_resource() = default;
      //コピーに意味がないので禁止
      stack_resource(const stack_resource&) = delete;
      stack_resource& operator=(const stack_resource&) = delete;

      /**
      * @brief メモリを割り当てる
      * @param bytes 割り当てるバイト数
      * @param alignment アライメント
      * @return 割り当てられたメモリのポインタ
      * @throw std::bad_alloc メモリが不足している場合
      */
      void* do_allocate(std::size_t bytes, std::size_t alignment) override {
         //空きがない
         if (N <= m_index) throw std::bad_alloc{};

         //2の累乗をチェック（AVX512のアライメント要求である64byteを最大としておく）
         bool is_pow2 = false;
         for (std::size_t pow2 = 1; pow2 <= std::size_t(64); pow2 *= 2) {
            if (alignment == pow2) {
               is_pow2 = true;
               break;
            }
         }

         //2の累乗でないアライメント要求はalignof(std::max_align_t)へ
         if (!is_pow2) {
            alignment = alignof(std::max_align_t);
         }

         auto addr = reinterpret_cast<std::uintptr_t>(&m_buffer[m_index]);

         //アライメント要求に合わせる
         while ((addr & std::uintptr_t(alignment - 1)) != 0) {
            ++addr;
            ++m_index;
         }

         m_index += bytes;

         //サイズが足りなくなったら
         if (N <= m_index) throw std::bad_alloc{};

         return reinterpret_cast<void*>(addr);
      }

      void do_deallocate(void* p, std::size_t bytes, [[maybe_unused]] std::size_t alignment) override {
         auto addr = static_cast<std::byte*>(p);
         auto end = std::end(m_buffer);

         if (m_buffer <= addr && addr < end) {
            //当てた領域をゼロ埋めするだけ
            for (std::size_t i = 0; i < bytes; ++i) {
               if ((addr + i) < end) {
                  addr[i] = std::byte(0);
               }
            }
         }

      }

      bool do_is_equal(const memory_resource& other) const noexcept override {
         return this == &other;
      }

	private:
		std::byte m_buffer[N]{};
		std::size_t m_index{};
	};
}
