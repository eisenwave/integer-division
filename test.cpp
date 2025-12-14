#include <array>
#include <iostream>
#include <limits>
#include <random>

#include "intdiv.hpp"

using big_int = long long;

template<class T>
constexpr bool is_div_defined(T x, T y) {
  if (y == 0) return false;
  if constexpr (std::is_signed_v<T>) {
    if (x == std::numeric_limits<T>::min() && y == -1) return false;
  }
  return true;
}

// This function tests for two things that have to hold true irrespective of rounding mode:
// 1. The quotient has to be within 1 of the quotient of the builtin division.
// 2. The equation (x == y * q + r) has to hold true.
template<class T>
constexpr bool is_valid_division(T x, T y, T q, T r) {
  const auto q_to_zero = big_int(x) / big_int(y);
  const int q_tolerance = big_int(x) % big_int(y) == 0 ? 0 : 1;
  if (std::abs(big_int(q) - q_to_zero) > q_tolerance) {
    return false;
  }
  if constexpr (std::is_signed_v<T>) {
    return big_int(x) == big_int(y) * big_int(q) + big_int(r);
  } else {
    return x == T(y * q + r);
  }
}

// Subsequent functions further constrain the remainder.
// For example, when rounding towards -inf,
// the sign of the remainder is the sign of the divisor or the remainder is zero.

template<class T>
constexpr bool is_valid_division_to_zero(T x, T y, T q, T r) {
  if (!is_valid_division(x, y, q, r)) return false;
  if constexpr (std::is_signed_v<T>) {
    return r == 0 || __sgn2(r) == __sgn2(x);
  } else {
    return r < y;
  }
}

template<class T>
constexpr bool is_valid_division_away_zero(T x, T y, T q, T r) {
  if (!is_valid_division(x, y, q, r)) return false;
  if constexpr (std::is_signed_v<T>) {
    return r == 0 || __sgn2(r) != __sgn2(x);
  } else {
    return big_int(q) == div_away_zero(big_int(x), big_int(y));
  }
}

template<class T>
constexpr bool is_valid_division_to_pos_inf(T x, T y, T q, T r) {
  if (!is_valid_division(x, y, q, r)) return false;
  if constexpr (std::is_signed_v<T>) {
    return r == 0 || __sgn2(r) != __sgn2(y);
  } else {
    return big_int(q) == div_to_pos_inf(big_int(x), big_int(y));
  }
}

template<class T>
constexpr bool is_valid_division_to_neg_inf(T x, T y, T q, T r) {
  if (!is_valid_division(x, y, q, r)) return false;
  if constexpr (std::is_signed_v<T>) {
    return r == 0 || __sgn2(r) == __sgn2(y);
  } else {
    return r < y;
  }
}

template<class T>
constexpr bool is_valid_division_to_odd(T x, T y, T q, T r) {
  if (!is_valid_division(x, y, q, r)) return false;
  return r == 0 || q % 2 != 0;
}

template<class T>
constexpr bool is_valid_division_to_even(T x, T y, T q, T r) {
  if (!is_valid_division(x, y, q, r)) return false;
  return r == 0 || q % 2 == 0;
}

// The tie-rounding functions should always:
// - round away from zero when abs(quotient) > 0.5,
// - round towards zero when abs(quotient) < 0.5, otherwise
// - check if the exact tie was rounded correctly

template<class T>
constexpr bool is_valid_division_to_nearest(T x, T y, T q, T r, bool is_tie_valid) {
  if (!is_valid_division(x, y, q, r)) return false;
  if (r == 0) return true;
  const big_int comp = std::abs(big_int(x) % big_int(y) * 2) - std::abs(big_int(y));
  if constexpr (std::is_signed_v<T>) {
    if (comp > 0) return __sgn2(r) != __sgn2(x);
    if (comp < 0) return __sgn2(r) == __sgn2(x);
  } else {
    if (comp != 0) return big_int(q) == div_ties_to_zero(big_int(x), big_int(y));
  }
  return is_tie_valid;
}

static_assert(div_rem_away_zero(72'777'531u, 3'405'476'348u).quotient == 1);
static_assert(div_rem_away_zero(72'777'531u, 3'405'476'348u).quotient == 1);

//            dividend    == quotient * divisor + remainder
static_assert(72'777'531u == 1u * 3'405'476'348u + 962'268'479u);
static_assert(div_rem_away_zero(72'777'531u, 3'405'476'348u).remainder == 962'268'479u);
static_assert(div_rem_to_pos_inf(72'777'531u, 3'405'476'348u).remainder == 962'268'479u);

template<class T>
constexpr bool is_valid_division_ties_to_zero(T x, T y, T q, T r) {
  if constexpr (std::is_signed_v<T>) {
    return is_valid_division_to_nearest(x, y, q, r, __sgn2(r) == __sgn2(x));
  } else {
    return is_valid_division_to_nearest(x, y, q, r, r < y);
  }
}

template<class T>
constexpr bool is_valid_division_ties_away_zero(T x, T y, T q, T r) {
  if constexpr (std::is_signed_v<T>) {
    return is_valid_division_to_nearest(x, y, q, r, __sgn2(r) != __sgn2(x));
  } else {
    bool quotient_like_signed = big_int(q) == div_away_zero(big_int(x), big_int(y));
    return is_valid_division_to_nearest(x, y, q, r, quotient_like_signed);
  }
}

template<class T>
constexpr bool is_valid_division_ties_to_pos_inf(T x, T y, T q, T r) {
  if constexpr (std::is_signed_v<T>) {
    return is_valid_division_to_nearest(x, y, q, r, __sgn2(r) != __sgn2(y));
  } else {
    bool quotient_like_signed = big_int(q) == div_away_zero(big_int(x), big_int(y));
    return is_valid_division_to_nearest(x, y, q, r, quotient_like_signed);
  }
}

template<class T>
constexpr bool is_valid_division_ties_to_neg_inf(T x, T y, T q, T r) {
  if constexpr (std::is_signed_v<T>) {
    return is_valid_division_to_nearest(x, y, q, r, __sgn2(r) == __sgn2(y));
  } else {
    return is_valid_division_to_nearest(x, y, q, r, r < y);
  }
}

template<class T>
constexpr bool is_valid_division_ties_to_odd(T x, T y, T q, T r) {
  return is_valid_division_to_nearest(x, y, q, r, q % 2 != 0);
}

template<class T>
constexpr bool is_valid_division_ties_to_even(T x, T y, T q, T r) {
  return is_valid_division_to_nearest(x, y, q, r, q % 2 == 0);
}

using rng_type = std::default_random_engine;

template<class T>
inline auto interesting_values = [] {
  if constexpr (std::is_signed_v<T>) {
    return std::array{
        T{std::numeric_limits<T>::min() + 2},
        T{std::numeric_limits<T>::min() + 1},
        T{std::numeric_limits<T>::min()},
        T{-2},
        T{-1},
        T{0},
        T{1},
        T{2},
        T{std::numeric_limits<T>::max() - 2},
        T{std::numeric_limits<T>::max() - 1},
        T{std::numeric_limits<T>::max()},
    };
  } else {
    return std::array{
        T{0},
        T{1},
        T{2},
        T{3},
        T{4},
        T{5},
        T{6},
        T{7},
        T{std::numeric_limits<T>::max() - 2},
        T{std::numeric_limits<T>::max() - 1},
        T{std::numeric_limits<T>::max()},
    };
  }
}();

template<class T, div_result<T> (&div_rem)(T, T), bool (&verify)(T, T, T, T)>
void check_interesting() {
  for (const T& x : interesting_values<T>) {
    for (const T& y : interesting_values<T>) {
      if (!is_div_defined(x, y)) continue;
      const auto [q, r] = div_rem(x, y);
      if (!verify(x, y, q, r)) {
        std::cout << "failed for (" << x << " / " << y << ") = " << q << " R " << r << "\n";
        std::exit(1);
      }
    }
  }
}

template<class T, div_result<T> (&div_rem)(T, T), bool (&verify)(T, T, T, T)>
void sample(rng_type& rng, std::uniform_int_distribution<T>& distr, int samples) {
  for (int i = 0; i < samples; ++i) {
    const T x = distr(rng);
    const T y = distr(rng);
    if (!is_div_defined(x, y)) continue;
    const auto [q, r] = div_rem(x, y);
    if (!verify(x, y, q, r)) {
      std::cout << "failed for (" << x << " / " << y << ") = " << q << " R " << r << "\n";
      std::exit(1);
    }
  }
}

constexpr int full_samples =
#ifndef NDEBUG
    1'000'000;
#else
    10'000'000;
#endif

template<class T, div_result<T> (&div_rem)(T, T), bool (&verify)(T, T, T, T)>
void fuzz_test(std::string_view name) {
  std::cout << name << " ... " << std::flush;

  check_interesting<T, div_rem, verify>();

  std::default_random_engine rng{12345};

  std::uniform_int_distribution<T> distr_tiny{std::is_signed_v<T> ? -4 : 0, 4};
  sample<T, div_rem, verify>(rng, distr_tiny, 100);

  std::uniform_int_distribution<T> distr_small{std::is_signed_v<T> ? -100 : 0, 100};
  sample<T, div_rem, verify>(rng, distr_small, 100'000);

  std::uniform_int_distribution<T> distr_full;
  sample<T, div_rem, verify>(rng, distr_full, full_samples);
  std::cout << "OK" << std::endl;
}

template<class T>
void fuzz_test_mod(std::string_view name) {
  std::cout << name << " ... ";
  std::default_random_engine rng{12345};
  std::uniform_int_distribution<T> distr_full;

  for (int i = 0; i < full_samples; ++i) {
    T x = distr_full(rng);
    T y = distr_full(rng);
    if (!is_div_defined(x, y)) continue;
    T r = mod(x, y);
    if (r != div_rem_to_neg_inf(x, y).remainder) {
      std::cout << "failure for (" << x << " mod " << y << ") = " << r << '\n';
      std::exit(1);
    }
  }
  std::cout << "OK" << std::endl;
}

#define RUN_TEST(type, div_rem, verify) fuzz_test<type, div_rem, verify>(#div_rem "<" #type ">")

int main() {
  RUN_TEST(int, div_rem_to_zero, is_valid_division_to_zero);
  RUN_TEST(int, div_rem_away_zero, is_valid_division_away_zero);
  RUN_TEST(int, div_rem_to_pos_inf, is_valid_division_to_pos_inf);
  RUN_TEST(int, div_rem_to_neg_inf, is_valid_division_to_neg_inf);
  RUN_TEST(int, div_rem_to_odd, is_valid_division_to_odd);
  RUN_TEST(int, div_rem_to_even, is_valid_division_to_even);

  RUN_TEST(int, div_rem_ties_to_zero, is_valid_division_ties_to_zero);
  RUN_TEST(int, div_rem_ties_away_zero, is_valid_division_ties_away_zero);
  RUN_TEST(int, div_rem_ties_to_pos_inf, is_valid_division_ties_to_pos_inf);
  RUN_TEST(int, div_rem_ties_to_neg_inf, is_valid_division_ties_to_neg_inf);
  RUN_TEST(int, div_rem_ties_to_odd, is_valid_division_ties_to_odd);
  RUN_TEST(int, div_rem_ties_to_even, is_valid_division_ties_to_even);

  RUN_TEST(unsigned, div_rem_to_zero, is_valid_division_to_zero);
  RUN_TEST(unsigned, div_rem_away_zero, is_valid_division_away_zero);
  RUN_TEST(unsigned, div_rem_to_pos_inf, is_valid_division_to_pos_inf);
  RUN_TEST(unsigned, div_rem_to_neg_inf, is_valid_division_to_neg_inf);
  RUN_TEST(unsigned, div_rem_to_odd, is_valid_division_to_odd);
  RUN_TEST(unsigned, div_rem_to_even, is_valid_division_to_even);

  RUN_TEST(unsigned, div_rem_ties_to_zero, is_valid_division_ties_to_zero);
  RUN_TEST(unsigned, div_rem_ties_away_zero, is_valid_division_ties_away_zero);
  RUN_TEST(unsigned, div_rem_ties_to_pos_inf, is_valid_division_ties_to_pos_inf);
  RUN_TEST(unsigned, div_rem_ties_to_neg_inf, is_valid_division_ties_to_neg_inf);
  RUN_TEST(unsigned, div_rem_ties_to_odd, is_valid_division_ties_to_odd);
  RUN_TEST(unsigned, div_rem_ties_to_even, is_valid_division_ties_to_even);

  fuzz_test_mod<int>("mod<int>");
  fuzz_test_mod<unsigned>("mod<unsigned>");
}
