#include "komoperm/komoperm.hpp"

#include <gtest/gtest.h>

#include <iostream>

using namespace komoperm::detail;
using namespace komoperm;

namespace {
enum class Hoge {
  kA,
  kB,
  kC,
  kD,
};
}  // namespace

TEST(Komoperm, choose_test) {
  constexpr Choose<int, 4, 4> kC1;
  constexpr Choose<int, 5, 2> kC2;

  // 4 choose 2 == 6
  EXPECT_EQ(kC1.Get(4, 2), 6);
  EXPECT_EQ(kC2.Get(4, 2), 6);
  EXPECT_EQ((ChooseMetaFunc<4, 2>::value), 6);

  // 4 choose 3 == 4
  EXPECT_EQ(kC1.Get(4, 3), 4);
  EXPECT_THROW(kC2.Get(4, 3), std::runtime_error);
  EXPECT_EQ((ChooseMetaFunc<4, 3>::value), 4);

  // 1 choose 2 == 0
  EXPECT_EQ(kC1.Get(1, 2), 0);
  EXPECT_EQ(kC2.Get(1, 2), 0);
  EXPECT_EQ((ChooseMetaFunc<1, 2>::value), 0);
}

TEST(Komoperm, copy_test) {
  int a[3] = {2, 6, 4};
  int b[3] = {};
  Copy(std::begin(a), std::end(a), std::begin(b));

  EXPECT_EQ(b[0], 2);
  EXPECT_EQ(b[1], 6);
  EXPECT_EQ(b[2], 4);
}

TEST(Komoperm, merge_sort_test) {
  int a[7] = {3, 4, 5, 1, 2, 9, 2};
  int b[7] = {};
  MergeSort(std::begin(a), std::end(a), std::begin(b));

  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(a[1], 2);
  EXPECT_EQ(a[2], 2);
  EXPECT_EQ(a[3], 3);
  EXPECT_EQ(a[4], 4);
  EXPECT_EQ(a[5], 5);
  EXPECT_EQ(a[6], 9);
}

TEST(Komoperm, any_of_test) {
  EXPECT_TRUE(AnyOf({true, false, true}));
  EXPECT_TRUE(AnyOf({false, false, true}));
  EXPECT_FALSE(AnyOf({false, false, false}));
}

TEST(Komoperm, unique_count_test) {
  EXPECT_EQ((UniqueCount<int, 3, 3, 4, 3, 3, 4>()), 2);
  EXPECT_EQ((UniqueCount<Hoge, Hoge::kA, Hoge::kB, Hoge::kA, Hoge::kC, Hoge::kD,
                         Hoge::kA>()),
            4);
}

TEST(Komoperm, item_count_index_test) {
  ItemCount<Hoge, Hoge::kA, 5, 2> ic1;
  constexpr Choose<std::size_t, 10, 10> kChoose;

  EXPECT_EQ(ic1.Size(), 10);

  std::array<Hoge, 5> vals{Hoge::kA, Hoge::kB, Hoge::kA, Hoge::kC, Hoge::kB};
  EXPECT_EQ(ic1.IndexImpl(kChoose, vals.begin()), 1);
  EXPECT_EQ(vals[0], Hoge::kB);
  EXPECT_EQ(vals[1], Hoge::kC);
  EXPECT_EQ(vals[2], Hoge::kB);

  std::array<Hoge, 5> vals2{Hoge::kB, Hoge::kB, Hoge::kA, Hoge::kA, Hoge::kB};
  EXPECT_EQ(ic1.IndexImpl(kChoose, vals2.begin()), 7);
}

TEST(Komoperm, item_count_operator_test) {
  ItemCount<Hoge, Hoge::kA, 3, 2> ic1;
  constexpr Choose<std::size_t, 10, 10> kChoose;

  Array<Hoge, 5> x{Hoge::kC, Hoge::kC, Hoge::kC, Hoge::kC, Hoge::kC};
  Array<bool, 5> f{false, true, false, true, false};

  ic1.Get(kChoose, 1, x, f);

  Array<Hoge, 5> ax{Hoge::kA, Hoge::kC, Hoge::kC, Hoge::kC, Hoge::kA};
  Array<bool, 5> af{true, true, false, true, true};
  for (std::size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(ax[i], x[i]) << "i=" << i;
    EXPECT_EQ(af[i], f[i]) << "i=" << i;
  }
}

TEST(Komoperm, item_count_is_ok_test) {
  ItemCount<Hoge, Hoge::kA, 3, 2> ic1;

  Array<Hoge, 5> ok{Hoge::kC, Hoge::kA, Hoge::kA, Hoge::kC, Hoge::kC};
  Array<Hoge, 3> ng1{Hoge::kA, Hoge::kC, Hoge::kC};
  Array<Hoge, 4> ng2{Hoge::kA, Hoge::kA, Hoge::kA};

  EXPECT_TRUE(ic1.IsOk(ok.begin(), ok.end()));
  EXPECT_FALSE(ic1.IsOk(ng1.begin(), ng1.end()));
  EXPECT_FALSE(ic1.IsOk(ng2.begin(), ng2.end()));
}

TEST(Komoperm, permutation_index_test) {
  constexpr Permutation<Hoge, Hoge::kA, Hoge::kA, Hoge::kA, Hoge::kB, Hoge::kB,
                        Hoge::kC>
      c;

  EXPECT_EQ(
      c.Index({Hoge::kA, Hoge::kA, Hoge::kA, Hoge::kB, Hoge::kB, Hoge::kC}), 0);
  EXPECT_EQ(
      c.Index({Hoge::kB, Hoge::kA, Hoge::kA, Hoge::kA, Hoge::kB, Hoge::kC}),
      10);

  EXPECT_THROW(c.Index({Hoge::kA, Hoge::kA}), std::runtime_error);
  EXPECT_THROW(
      c.Index({Hoge::kA, Hoge::kA, Hoge::kA, Hoge::kA, Hoge::kB, Hoge::kC}),
      std::runtime_error);
}

TEST(Komb, permutation_operator_test) {
  constexpr Permutation<Hoge, Hoge::kA, Hoge::kA, Hoge::kA, Hoge::kB, Hoge::kB,
                        Hoge::kC>
      c;

  auto x1 = c[0];
  std::array<Hoge, 6> ans1 = {Hoge::kA, Hoge::kA, Hoge::kA,
                              Hoge::kB, Hoge::kB, Hoge::kC};
  for (std::size_t i = 0; i < 6; ++i) {
    EXPECT_EQ(ans1[i], x1[i]);
  }

  auto x2 = c[10];
  std::array<Hoge, 6> ans2 = {Hoge::kB, Hoge::kA, Hoge::kA,
                              Hoge::kA, Hoge::kB, Hoge::kC};
  for (std::size_t i = 0; i < 6; ++i) {
    EXPECT_EQ(ans2[i], x2[i]);
  }
  EXPECT_THROW(c[c.Size()], std::runtime_error);
}

TEST(Komb, permutation_identity_test) {
  constexpr Permutation<Hoge, Hoge::kA, Hoge::kA, Hoge::kA, Hoge::kB, Hoge::kB,
                        Hoge::kC>
      c;

  for (std::size_t i = 0; i < c.Size(); ++i) {
    EXPECT_EQ(c.Index(c[i]), i);
  }
}

#if __cplusplus >= 201703L
TEST(Komoperm, permutation_auto_test) {
  ::testing::StaticAssertTypeEq<
      PermutationAuto<Hoge::kA, Hoge::kB, Hoge::kA>,
      Permutation<Hoge, Hoge::kA, Hoge::kB, Hoge::kA> >();
}
#endif  // __cplusplus >= 201703L
