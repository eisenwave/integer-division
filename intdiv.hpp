#ifndef INTDIV_HPP
#define INTDIV_HPP

#include <compare>
#include <type_traits>

using __suppress_unused_include_compare_warning = std::strong_ordering;

template<class T>
concept __cv_unqualified = !std::is_const_v<T> && !std::is_volatile_v<T>;

template<class T>
concept __non_integer =
    std::same_as<T, char> || std::same_as<T, wchar_t> || std::same_as<T, char8_t> ||
    std::same_as<T, char16_t> || std::same_as<T, char32_t> || std::same_as<T, bool>;

template<class T>
concept __integer = std::integral<T> && !__non_integer<T> && __cv_unqualified<T>;

template<class T>
struct div_result {
  T quotient;
  T remainder;
  friend auto operator<=>(const div_result &, const div_result &) = default;
};

template<__integer T>
inline constexpr T __sgn2(T x) {
  if constexpr (std::is_signed_v<T>) {
    return x < 0 ? -1 : 1;
  } else {
    return 1;
  }
}

template<__integer T>
inline constexpr div_result<T> __div_rem_offset_quotient(T x, T y, T d) {
  if constexpr (std::is_signed_v<T>) {
    using U = std::make_unsigned_t<T>;
    return {
        .quotient = T(x / y + d),
        .remainder = T(U(x % y) - U(d) * U(y)),
    };
  } else {
    return {
        .quotient = T(x / y + d),
        .remainder = T(x % y - d * y),
    };
  }
}

template<__integer T>
constexpr div_result<T> div_rem_to_zero(T x, T y) {
  return {
      .quotient = T(x / y),
      .remainder = T(x % y),
  };
}

template<__integer T>
constexpr T div_to_zero(T x, T y) {
  return T(x / y);
}

template<__integer T>
constexpr div_result<T> div_rem_away_zero(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  bool increment = x % y != 0;
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_away_zero(T x, T y) {
  return div_rem_away_zero(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_to_pos_inf(T x, T y) {
  bool quotient_positive = (x ^ y) >= 0;
  bool adjust = (x % y != 0) & quotient_positive;
  return {
      .quotient = T(x / y + T(adjust)),
      .remainder = T(x % y - T(adjust) * y),
  };
}

template<__integer T>
constexpr T div_to_pos_inf(T x, T y) {
  return div_rem_to_pos_inf(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_to_neg_inf(T x, T y) {
  bool quotient_negative = (x ^ y) < 0;
  bool adjust = (x % y != 0) & quotient_negative;
  return {
      .quotient = T(x / y - T(adjust)),
      .remainder = T(x % y + T(adjust) * y),
  };
}

template<__integer T>
constexpr T div_to_neg_inf(T x, T y) {
  return div_rem_to_neg_inf(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_to_odd(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  bool increment = (x % y != 0) & (x / y % 2 == 0);
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_to_odd(T x, T y) {
  return div_rem_to_odd(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_to_even(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  bool increment = (x % y != 0) & (x / y % 2 != 0);
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_to_even(T x, T y) {
  return div_rem_to_even(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_ties_to_zero(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  T abs_rem = x % y * __sgn2(x);
  T abs_half_y = y / 2 * __sgn2(y);
  bool increment = abs_rem > abs_half_y;
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_ties_to_zero(T x, T y) {
  return div_rem_ties_to_zero(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_ties_away_zero(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  T abs_rem = x % y * __sgn2(x);
  T abs_half_y = y / 2 * __sgn2(y);
  bool increment = abs_rem >= abs_half_y + T(y % 2 != 0);
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_ties_away_zero(T x, T y) {
  return div_rem_ties_away_zero(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_ties_to_pos_inf(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  T abs_rem = x % y * __sgn2(x);
  T abs_half_y = y / 2 * __sgn2(y);
  bool increment = abs_rem >= abs_half_y + T((y % 2 != 0) | (quotient_sign < 0));
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_ties_to_pos_inf(T x, T y) {
  return div_rem_ties_to_pos_inf(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_ties_to_neg_inf(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  T abs_rem = x % y * __sgn2(x);
  T abs_half_y = y / 2 * __sgn2(y);
  bool increment = abs_rem >= abs_half_y + T((y % 2 != 0) | (quotient_sign > 0));
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_ties_to_neg_inf(T x, T y) {
  return div_rem_ties_to_neg_inf(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_ties_to_odd(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  T abs_rem = x % y * __sgn2(x);
  T abs_half_y = y / 2 * __sgn2(y);
  T quotient = x / y;
  bool increment = abs_rem >= abs_half_y + T((y % 2 != 0) | (quotient % 2 != 0));
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_ties_to_odd(T x, T y) {
  return div_rem_ties_to_odd(x, y).quotient;
}

template<__integer T>
constexpr div_result<T> div_rem_ties_to_even(T x, T y) {
  T quotient_sign = __sgn2(x) * __sgn2(y);
  T abs_rem = x % y * __sgn2(x);
  T abs_half_y = y / 2 * __sgn2(y);
  T quotient = x / y;
  bool increment = abs_rem >= abs_half_y + T((y % 2 != 0) | (quotient % 2 == 0));
  return __div_rem_offset_quotient(x, y, T(increment) * quotient_sign);
}

template<__integer T>
constexpr T div_ties_to_even(T x, T y) {
  return div_rem_ties_to_even(x, y).quotient;
}

template<__integer T>
constexpr T mod(T x, T y) {
  if constexpr (std::is_signed_v<T>) {
    bool quotient_negative = (x ^ y) < 0;
    T rem = x % y;
    return rem + y * T((rem != 0) & quotient_negative);
  } else {
    return x % y;
  }
}

#endif
