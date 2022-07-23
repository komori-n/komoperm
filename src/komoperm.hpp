#ifndef KOMORI_KOMOPERM_HPP_
#define KOMORI_KOMOPERM_HPP_

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace komoperm {
namespace detail {
/**
 * @brief A utility template type for SFINAE
 *
 * By using this idiom, you can make SFINAE codes a little easier to understand
 *
 * # Example
 *
 * An example that defines a template function if `T` is a integer type.
 *
 * ```
 * template <typename T,
 *           Constraints<std::enable_if_t<std::is_integral_v<T>>> = nullptr>
 * void Func() {
 *    ...
 * ```
 */
template <typename...>
using Constraints = std::nullptr_t;

/**
 * @brief A type that can be constructed by any type
 *
 * This type is only used for the implementation of `ConsumeValues()`.
 */
struct Any {
  template <typename Arg>
  // NOLINTNEXTLINE
  constexpr Any(Arg&&) noexcept {}
};

/**
 * @brief A dummy function which consumes all passed expressions.
 *
 * In c++14, the order of evaluation of function arguments is not defined by the
 * standard. Therefore, this dummy function provides a feature that it evaluates
 * the given expressions (typically template parameter pack) by the defined
 * order.
 *
 * # Example
 *
 * The following example definitely calls `OtherFunc(Seq_1)`,
 * `OtherFunc(Seq_2)`, ...
 *
 * ```
 * template <std::size_t... Seq>
 * void Func(std::index_sequence<Seq...>) noexcept {
 *     ConsumeValues({OtherFunc(Seq)...});
 * }
 * ```
 */
constexpr inline void ConsumeValues(std::initializer_list<Any>) noexcept {}

/**
 * We would like to use `std::array<T, N>` at compile time, but actually some of
 * its methods are not constexpr in C++14. Therefore, we define an alternative
 * class if c++ version is old.
 */
#if __cplusplus < 201703L
/**
 * @brief A class like `std::array<T, N>` which supports constexpr manipulation.
 */
template <typename T, std::size_t N>
struct MyArray {
  /// The actual data region for the array. This is a public member to make
  /// `MyArray` an aggregate class.
  T _vals[N];

  constexpr auto& operator[](std::size_t index) { return _vals[index]; }

  constexpr const auto& operator[](std::size_t index) const {
    return _vals[index];
  }

  constexpr auto& front() { return _vals[0]; }
  constexpr const auto& front() const { return _vals[0]; }
  constexpr auto& back() { return _vals[N - 1]; }
  constexpr const auto& back() const { return _vals[N - 1]; }
  constexpr T* data() noexcept { return _vals; }
  constexpr const T* data() const noexcept { return _vals; }

  constexpr auto begin() { return std::begin(_vals); }
  constexpr auto begin() const { return std::begin(_vals); }
  constexpr auto end() { return std::end(_vals); }
  constexpr auto end() const { return std::end(_vals); }
  constexpr auto cbegin() const { return std::cbegin(_vals); }
  constexpr auto cend() const { return std::cend(_vals); }
  constexpr auto rbegin() { return std::rbegin(_vals); }
  constexpr auto rbegin() const { return std::rbegin(_vals); }
  constexpr auto rend() { return std::rend(_vals); }
  constexpr auto rend() const { return std::rend(_vals); }
  constexpr auto crbegin() const { return std::crbegin(_vals); }
  constexpr auto crend() const { return std::crend(_vals); }

  constexpr bool empty() const noexcept { return N != 0; }
  constexpr std::size_t size() const noexcept { return N; }
  constexpr std::size_t max_size() const noexcept { return N; }
};

template <typename T, std::size_t N>
using Array = MyArray<T, N>;
#else
template <typename T, std::size_t N>
using Array = std::array<T, N>;
#endif  // __cplusplus < 201703L

/**
 * @brief A class which precalculates the results of the choose function.
 *
 * This function holds a NxM array, and provides the result of
 * choose function from (1 choose 0) to (N choose M) in O(1) time complexity.
 *
 * # Example
 *
 * ```
 * constexpr Choose<std::size_t, 4, 2> choose{};
 * EXPECT_EQ(choose.Get(4, 2), 6);
 * ```
 */
template <typename T, std::size_t N, std::size_t M = N>
class Choose {
  static_assert(std::is_integral<T>::value, "T must be an integer type");
  static_assert(M <= N, "M must be equal to or less than N");

 public:
  constexpr Choose() noexcept {
    for (std::size_t i = 0; i < N; ++i) {
      vals_[i][0] = 1;

      for (std::size_t j = 1; j <= std::min(i, M); ++j) {
        vals_[i][j] = vals_[i - 1][j] + vals_[i - 1][j - 1];
      }

      if (i < M) {
        vals_[i][i + 1] = 1;
      }
    }
  }

  /**
   * @brief Calculate (n choose m). precondition: (n <= N && m <= M).
   */
  constexpr T Get(std::size_t n, std::size_t m) const {
    if (m > n) {
      return 0;
    } else if (n > N || m > M) {
      throw std::runtime_error("index out of range");
    }

    return vals_[n - 1][m];
  }

 private:
  /// vals_[n-1][m] = nCm
  T vals_[N][M + 1]{};
};

/**
 * @brief Calculate (N choose M) at compile time
 *
 * Whereas `Choose` class may calculate the results at runtime, this meta
 * function definitely calculates it at compile time.
 */
template <std::size_t N, std::size_t M>
struct ChooseMetaFunc
    : std::integral_constant<std::size_t,
                             ChooseMetaFunc<N - 1, M>::value +
                                 ChooseMetaFunc<N - 1, M - 1>::value> {};

template <std::size_t N>
struct ChooseMetaFunc<N, 0> : std::integral_constant<std::size_t, 1> {};

template <std::size_t M>
struct ChooseMetaFunc<0, M> : std::integral_constant<std::size_t, 0> {};
template <std::size_t N>
struct ChooseMetaFunc<N, N> : std::integral_constant<std::size_t, 1> {};

/**
 * @brief Copy [in_begin, in_end)
 *             to [out_begin, out_begin + (in_end - in_begin))
 *
 * As `std::copy()` is not constexpr in C++14, we prepare a similar function.
 */
template <typename InputIterator, typename OutputIterator>
inline constexpr void Copy(InputIterator in_begin, InputIterator in_end,
                           OutputIterator out_begin) noexcept {
  while (in_begin != in_end) {
    *(out_begin++) = *(in_begin++);
  }
}

/**
 * @brief `true` iff `list` has at least one `true`
 *
 * As `std::any_of()` is not constexpr in C++14, we prepare a similar function.
 */
inline constexpr bool AnyOf(std::initializer_list<bool> list) noexcept {
  for (auto x : list) {  // NOLINT
    if (x) {
      return true;
    }
  }
  return false;
}

/**
 * @brief A constexpr sort function.
 *
 * Due to the constraints of merge sorts, this function requires a temporary
 * region which is greater than or equal to `end - begin`.
 */
template <typename Iterator>
inline constexpr void MergeSort(Iterator begin, Iterator end,
                                Iterator tmp_begin) noexcept {
  const auto len = end - begin;
  if (len <= 1) {
    return;
  }

  Iterator mid = begin + len / 2;

  MergeSort(begin, mid, tmp_begin);
  MergeSort(mid, end, tmp_begin);

  Iterator li = begin;
  Iterator ri = mid;
  Iterator oi = tmp_begin;
  while (li < mid && ri < end) {
    if (*li < *ri) {
      *(oi++) = *(li++);
    } else {
      *(oi++) = *(ri++);
    }
  }

  if (li == mid) {
    while (ri < end) {
      *(oi++) = *(ri++);
    }
  } else {
    while (li < mid) {
      *(oi++) = *(li++);
    }
  }

  auto tmp_end = tmp_begin + len;
  Copy(tmp_begin, tmp_end, begin);
}

/**
 * @brief This function counts # of unique values in `Vals...`
 */
template <typename T, T... Vals>
inline constexpr std::size_t UniqueCount() noexcept {
  if (sizeof...(Vals) == 0) {
    return 0;
  }

  T vals[sizeof...(Vals)]{Vals...};
  T tmp[sizeof...(Vals)]{};

  // Sort the input values to reduce time complexity
  MergeSort(std::begin(vals), std::end(vals), std::begin(tmp));

  T prev = vals[0];
  std::size_t count = 1;
  for (std::size_t i = 1; i < sizeof...(Vals); ++i) {
    if (prev != vals[i]) {
      prev = vals[i];
      count++;
    }
  }
  return count;
}

/**
 * @brief A helper class for placement of 'C' of  `Val` in `N` spaces.
 */
template <typename T, T Val, std::size_t N, std::size_t C>
struct ItemCount {
  /**
   * @brief The number of possible placements
   */
  static constexpr std::size_t Size() noexcept {
    return ChooseMetaFunc<N, C>::value;
  }

  /**
   * @brief Get `index` for the given 'Val' placement, and remove it from the
   * sequence.
   */
  template <std::size_t N2, std::size_t M2, typename Iterator,
            Constraints<std::enable_if_t<N2 >= N && M2 >= C>> = nullptr>
  static constexpr std::size_t IndexImpl(
      const Choose<std::size_t, N2, M2>& choose, Iterator buffer) noexcept {
    std::size_t ret = 0;
    std::size_t remain_cnt = C;
    Iterator out_itr = buffer;
    for (std::size_t i = 0; i < N; ++i, ++buffer) {
      if (*buffer == Val) {
        remain_cnt--;
      } else {
        if (remain_cnt > 0) {
          ret += choose.Get(N - i - 1, remain_cnt - 1);
        }
        *(out_itr++) = *buffer;
      }
    }
    return ret;
  }

  /**
   * @brief Get `index`'th placement.
   *
   * @param array   The output region for placement.
   * @param filled  If `filled[i]` is true, the slot of `array` (`array[i]`) is
   * just ignored.
   */
  template <std::size_t N2, std::size_t M2, std::size_t L>
  static constexpr void Get(const Choose<std::size_t, N2, M2>& choose,
                            std::size_t index, Array<T, L>& array,
                            Array<bool, L>& filled) noexcept {
    std::size_t remain_cnt = C;
    for (std::size_t i = 0, j = 0; j < L; ++j) {
      if (filled[j]) {
        continue;
      }

      if (remain_cnt > 0) {
        if (remain_cnt >= N - i ||
            index < choose.Get(N - i - 1, remain_cnt - 1)) {
          array[j] = Val;
          filled[j] = true;
          remain_cnt--;
        } else {
          index -= choose.Get(N - i - 1, remain_cnt - 1);
        }
      }

      ++i;
    }

    assert(remain_cnt == 0);
  }

  /**
   * @brief Check if the input sequence is a possible placement.
   */
  template <typename Iterator>
  static constexpr bool IsOk(Iterator begin, Iterator end) noexcept {
    std::size_t cnt = 0;
    for (auto itr = begin; itr != end; ++itr) {
      if (*itr == Val) {
        cnt++;
      }
    }

    return cnt == C;
  }
};

/**
 * @brief A class that realizes the main features for permutation with
 * duplicates
 *
 * @tparam T    The type to be placed. It should be an integer or an enum type
 * @tparam N    The number of spaces
 * @tparam M    The maximum number of same values
 * @tparam ICs  The parameter pack of ItemCount
 */
template <typename T, std::size_t N, std::size_t M, typename... ICs>
class PermutationImpl {
 public:
  /**
   * @brief The number of possible placements
   */
  constexpr std::size_t Size() const noexcept { return SizeImpl(); }
  constexpr std::size_t Index(const T (&vals)[N]) const {
    T tmp_vals[N]{};
    Copy(std::begin(vals), std::end(vals), std::begin(tmp_vals));
    return IndexImpl(tmp_vals);
  }

  /**
   * @brief Get `index` for the given placement
   */
  template <typename Container>
  constexpr std::size_t Index(const Container& vals) const {
    if (vals.size() != N) {
      throw std::runtime_error("The size of `vals` is illegal");
    }

    T tmp_vals[N]{};
    Copy(vals.begin(), vals.end(), std::begin(tmp_vals));
    return IndexImpl(tmp_vals);
  }

  /**
   * @brief Get `index`'th placement.
   */
  constexpr Array<T, N> operator[](std::size_t index) const {
    if (index >= Size()) {
      throw std::runtime_error("Index out of range");
    }

    Array<T, N> ret{};
    Array<bool, N> filled{};
    ConsumeValues({(ICs::Get(choose_, index % ICs::Size(), ret, filled),
                    index /= ICs::Size())...});

    return ret;
  }

 private:
  static constexpr std::size_t SizeImpl() noexcept {
    std::size_t ret = 1;

    // Calculate all multiplication of ICs::Size().
    //
    // [Notes]
    // - As fold expression (ICs::Size() * ...) is not available in C++14, we
    //   adopts `ConsumeValues()` to realize a loop without recursion.
    // - In order to prevent overflow, we check it at every iteration.
    ConsumeValues(
        {(assert(std::numeric_limits<std::size_t>::max() / ret > ICs::Size()),
          ret *= ICs::Size())...});
    return ret;
  }

  constexpr std::size_t IndexImpl(T (&tmp_vals)[N]) const {
    if (AnyOf({!ICs::IsOk(std::begin(tmp_vals), std::end(tmp_vals))...})) {
      throw std::runtime_error("Input is illegal");
    }

    // The output index can be splitted as follows.
    //
    //     index = (index of ICs[0]) x (index of ICs[1]) x ...
    //                   ∈                   ∈             ...
    //             [0, ICs[0]::Size())  [0, ICs[1]::Size())   ...
    //       x: Cartesian Product
    std::size_t index = 0;
    std::size_t base = 1;
    ConsumeValues(
        {(index += base * ICs::IndexImpl(choose_, std::begin(tmp_vals)),
          base *= ICs::Size())...});
    return index;
  }

  Choose<std::size_t, N, M> choose_{};

  static_assert(SizeImpl() < std::numeric_limits<std::size_t>::max(),
                "Total size must be finite");
};

/**
 * @brief An array that summarize the input sequence `Vals...`
 */
template <typename T, std::size_t N>
struct ItemArray {
  T values[N];
  std::size_t remains[N];
  std::size_t counts[N];
};

/**
 * @brief Summarize the input sequence `Vals...`
 *
 * # Example
 *
 * MakeItemCountsImplCalc<int, 3, 3, 4, 2, 6, 4>
 * => ItemArray{
 *   values[] = {3, 4, 2, 6},
 *   remains[] = {4, 2, 1, 0},   // The sum of counts[i+1]..counts[Max]
 *   counts[] = {2, 2, 1, 0},    // The number of values[i]
 * }
 */
template <typename T, T... Vals>
inline constexpr auto MakeItemCountsImplCalc() noexcept {
  ItemArray<T, UniqueCount<T, Vals...>()> ret{};
  T vals[sizeof...(Vals)]{Vals...};
  bool visited[sizeof...(Vals)]{};

  std::size_t remains = sizeof...(Vals);
  std::size_t max_idx = 0;
  for (std::size_t i = 0; i < sizeof...(Vals); ++i) {
    if (!visited[i]) {
      visited[i] = true;
      ret.values[max_idx] = vals[i];
      std::size_t count = 1;
      for (std::size_t j = i + 1; j < sizeof...(Vals); ++j) {
        if (vals[i] == vals[j]) {
          count++;
          visited[j] = true;
        }
      }
      ret.remains[max_idx] = remains;
      ret.counts[max_idx] = count;

      remains -= count;
      max_idx++;
    }
  }

  assert(max_idx == (UniqueCount<T, Vals...>()));
  assert(remains == 0);

  return ret;
}

/**
 * @brief A class that holds the input sequence as a template parameter pack
 */
template <typename T, T... Vals>
struct ValueSet {};

/**
 * @brief Create a proper permutation implementation at compile time (See below)
 */
template <typename V, typename I>
struct MakePermutationImpl;

/**
 * @brief Create a proper permutation implementation at compile time
 *
 * This class checks the input sequence, and deduces a proper implementation
 * class for that, which is done at compile time (!).
 *
 * # Example
 *
 * MakePermutationImpl<ValueSet<int, 3, 3, 4, 2, 6, 4>,
 *                     std::index_sequence<0, 1, 2, 3, 4, 5, 6>>::type
 * => PermutationImpl<int, // The type of `Vals...`
 *        6,   // The number of `Vals...`
 *        2,   // The maximum number of symbols for `Vals...`
 *        //       <type, symbol, remain, count>
 *        ItemCount<int, 3, 4, 2>,
 *        ItemCount<int, 4, 2, 2>,
 *        ItemCount<int, 2, 1, 1>,
 *        ItemCount<int, 6, 0, 1>
 * >
 */
template <typename T, T... Vals, std::size_t... Indices>
struct MakePermutationImpl<ValueSet<T, Vals...>,
                           std::index_sequence<Indices...>> {
 private:
  /**
   * @brief An implementation method for type deduction. This function is never
   * called at runtime.
   */
  static constexpr auto Impl() noexcept {
    // In C++14, `static constexpr` member might need to be defined in .cpp
    // file. Therefore, it is not suitable for us, header only template
    // libraries.
    //
    // So we hide the constexpr instance into static method, deduce the proper
    // type, and extract it by `decltype()`.
    constexpr auto kValue = MakeItemCountsImplCalc<T, Vals...>();
    using type = PermutationImpl<
        T, sizeof...(Vals), std::max({kValue.counts[Indices]...}),
        ItemCount<T, kValue.values[Indices], kValue.remains[Indices],
                  kValue.counts[Indices]>...>;
    return type{};
  }

 public:
  using type = decltype(Impl());
};
}  // namespace detail

/**
 * @brief A class that handles permutation of duplicates
 *
 * - `Permutation::Size()` returns the number of possible placements.
 * - `Permutation::Index({...})` returns an unique number on [0, Size()) for any
 *   placements.
 * - `Permutation::operator[index]` returns the `index`'th placement for the
 *   input sequence. Note that `permutation.Index(permutation[index])` is always
 *   equals to `index`.
 *
 * All methods are marked as constexpr, and can be invoked at compile time.
 *
 * # Example
 *
 * ```
 * enum Kind {
 *      A, B, C,
 * };
 *
 * // Consider the placement of the set {A, A, A, B, B, C}
 * constexpr Permutation<Kind, A, A, A, B, B, C> permutation;
 *
 * static_assert(permutation.Size() == 60);
 *
 * constexpr auto seq = permutation[10];  // {B, A, A, A, B, C}
 * constexpr auto index = permutation.Index({B, A, A, A, B, C});  // 10
 * ```
 *
 * @tparam T     An integer or enum
 * @tparam Vals  A sequence of type `T`. (duplication of values are permitted)
 */
template <typename T, T... Vals>
using Permutation = typename detail::MakePermutationImpl<
    detail::ValueSet<T, Vals...>,
    std::make_index_sequence<detail::UniqueCount<T, Vals...>()>>::type;

#if __cplusplus >= 201703L
/**
 * @brief A class that handles permutation of duplicates
 *
 * This function is an alternative of `Permutation` in which you can omit the
 * type parameter. For more detail, see the description of `Permutation`.
 */
template <auto Val, decltype(Val)... Vals>
using PermutationAuto = typename detail::MakePermutationImpl<
    detail::ValueSet<decltype(Val), Val, Vals...>,
    std::make_index_sequence<
        detail::UniqueCount<decltype(Val), Val, Vals...>()>>::type;
#endif  // __cplusplus >= 201703L
}  // namespace komoperm

#endif  // KOMORI_KOMOPERM_HPP_