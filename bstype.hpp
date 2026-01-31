/**
 * Based on https://github.com/cemu-project/Cemu/blob/main/src/Common/betype.h
 */
#pragma once
#include <type_traits>
#include <bit>
#include <concepts>
#include <utility>

#include "ints.hpp"

using std::to_underlying;

template <std::integral T>
constexpr T SwapEndian(T value)
{
   return std::byteswap(value);
}

template <std::floating_point T>
constexpr T SwapEndian(T value)
{
   if constexpr (sizeof(T) == sizeof(u32))
   {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<u32>(value)));
   }
   if constexpr (sizeof(T) == sizeof(u64))
   {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<u64>(value)));
   }
   else
      std::unreachable();
}

template <typename TEnum>
   requires std::is_enum_v<TEnum>
constexpr TEnum SwapEndian(TEnum value)
{
   return static_cast<TEnum>(std::byteswap(to_underlying(value)));
}

// swap if native isn't big endian
template <typename T>
constexpr T from_be(T value)
{
   if constexpr (std::endian::native == std::endian::big)
      return value;
   else
      return SwapEndian(value);
}

// swap if native isn't little endian
template <typename T>
constexpr T from_le(T value)
{
   if constexpr (std::endian::native == std::endian::little)
      return value;
   else
      return SwapEndian(value);
}

template <typename T>
class bstype
{
public:
   constexpr bstype() = default;

   constexpr explicit(false) bstype(T value)
      : m_value(SwapEndian(value))
   {
   }

   constexpr bstype(const bstype& value) = default;

   template <typename U>
   constexpr explicit(false) bstype(const bstype<U>& value)
      : bstype(static_cast<T>(value.value()))
   {
   }

   // assigns
   static bstype from_bsvalue(T value)
   {
      bstype result;
      result.m_value = value;
      return result;
   }

   // returns native endian value
   [[nodiscard]] constexpr T value() const { return SwapEndian<T>(m_value); }

   // returns alternate endian value
   [[nodiscard]] constexpr T bsvalue() const { return m_value; }

   constexpr explicit(false) operator T() const { return value(); }

   constexpr bstype& operator=(const bstype& rhs) = default;

   constexpr bstype& operator+=(const bstype& v)
   {
      m_value = SwapEndian(T(value() + v.value()));
      return *this;
   }

   constexpr bstype& operator-=(const bstype& v)
   {
      m_value = SwapEndian(T(value() - v.value()));
      return *this;
   }

   constexpr bstype& operator*=(const bstype& v)
   {
      m_value = SwapEndian(T(value() * v.value()));
      return *this;
   }

   constexpr bstype& operator/=(const bstype& v)
   {
      m_value = SwapEndian(T(value() / v.value()));
      return *this;
   }

   constexpr bstype& operator&=(const bstype& v)
      requires (requires(T& x, const T& y) { x &= y; })
   {
      m_value &= v.m_value;
      return *this;
   }

   constexpr bstype& operator|=(const bstype& v)
      requires (requires(T& x, const T& y) { x |= y; })
   {
      m_value |= v.m_value;
      return *this;
   }

   constexpr bstype& operator^=(const bstype& v)
      requires (requires(T& x, const T& y) { x ^= y; })
   {
      m_value ^= v.m_value;
      return *this;
   }

   constexpr bstype operator|(const bstype& v) const
      requires (requires(const T& x, const T& y) { x | y; })
   {
      bstype tmp(*this);
      tmp.m_value = tmp.m_value | v.m_value;
      return tmp;
   }

   constexpr bstype operator&(const bstype& v) const
      requires (requires(const T& x, const T& y) { x & y; })
   {
      bstype tmp(*this);
      tmp.m_value = tmp.m_value & v.m_value;
      return tmp;
   }

   constexpr bstype operator^(const bstype& v) const
      requires (requires(const T& x, const T& y) { x ^ y; })
   {
      bstype tmp(*this);
      tmp.m_value = tmp.m_value ^ v.m_value;
      return tmp;
   }

   constexpr bstype& operator>>=(std::size_t idx)
      requires std::integral<T>
   {
      m_value = SwapEndian(T(value() >> idx));
      return *this;
   }

   constexpr bstype& operator<<=(std::size_t idx)
      requires std::integral<T>
   {
      m_value = SwapEndian(T(value() << idx));
      return *this;
   }

   constexpr bstype operator~() const
      requires std::integral<T>
   {
      return from_bsvalue(T(~m_value));
   }

   constexpr bstype& operator++()
      requires std::integral<T>
   {
      m_value = SwapEndian(T(value() + 1));
      return *this;
   }

   constexpr bstype operator++(int)
      requires std::integral<T>
   {
      const auto old = value();
      m_value = SwapEndian(T(old + 1));
      return old;
   }

   constexpr bstype& operator--()
      requires std::integral<T>
   {
      m_value = SwapEndian(T(value() - 1));
      return *this;
   }

   constexpr bstype operator--(int)
      requires std::integral<T>
   {
      const auto old = value();
      m_value = SwapEndian(T(old - 1));
      return old;
   }
private:
   T m_value;
};

template <typename TEnum> requires std::is_enum_v<TEnum>
bstype<std::underlying_type_t<TEnum>> to_underlying(bstype<TEnum> value)
{
   return std::to_underlying(static_cast<TEnum>(value));
}

template <typename TBase>
using betype = std::conditional_t<std::endian::native == std::endian::big, TBase, bstype<TBase>>;

using u64be = betype<u64>;
using u32be = betype<u32>;
using u16be = betype<u16>;
using u8be = u8;

using s64be = betype<s64>;
using s32be = betype<s32>;
using s16be = betype<s16>;
using s8be = s8;

using float32be = betype<float>;
using float64be = betype<double>;

template <typename TBase>
using letype = std::conditional_t<std::endian::native == std::endian::little, TBase, bstype<TBase>>;

using u64le = letype<u64>;
using u32le = letype<u32>;
using u16le = letype<u16>;
using u8le = u8;

using s64le = letype<s64>;
using s32le = letype<s32>;
using s16le = letype<s16>;
using s8le = s8;

using float32le = letype<float>;
using float64le = letype<double>;

static_assert(sizeof(bstype<u64>) == sizeof(u64));
static_assert(sizeof(bstype<u32>) == sizeof(u32));
static_assert(sizeof(bstype<u16>) == sizeof(u16));
static_assert(sizeof(bstype<u8>) == sizeof(u8));
static_assert(sizeof(bstype<float>) == sizeof(float));
static_assert(sizeof(bstype<double>) == sizeof(double));

static_assert(std::is_trivially_copyable_v<u32be>);
static_assert(std::is_trivially_constructible_v<u32be>);
static_assert(std::is_copy_constructible_v<u32be>);
static_assert(std::is_move_constructible_v<u32be>);
static_assert(std::is_copy_assignable_v<u32be>);
static_assert(std::is_move_assignable_v<u32be>);

#include <format>
template <typename TBase>
struct std::formatter<bstype<TBase>> : public std::formatter<TBase>
{
   auto format(const bstype<TBase>& value, std::format_context& ctx) const
   {
      return std::formatter<TBase>::format(static_cast<TBase>(value), ctx);
   }
};