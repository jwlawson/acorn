/*
 * Copyright (c) 2020, John Lawson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "acorn/container/tuple.h"

#include <gtest/gtest.h>

namespace acorn {
template<typename... Args>
using Tuple = InheritedTuple<Args...>;
//using Tuple = MemberTuple<Args...>;
}  // namespace acorn

static_assert(
    std::is_same<acorn::TupleElement<0, acorn::Tuple<int>>::type, int>::value,
    "First type of Tuple<int> should be int");

static_assert(
    std::is_same<acorn::TupleElement<0, acorn::Tuple<float, int>>::type,
                 float>::value,
    "First type of Tuple<float, int> should be float");

static_assert(
    std::is_same<acorn::TupleElement<1, acorn::Tuple<float, int>>::type,
                 int>::value,
    "Second type of Tuple<float, int> should be int");

#if 0
static_assert(std::is_standard_layout<acorn::Tuple<int>>::value,
              "Tuple<int> should be standard layout");
static_assert(std::is_standard_layout<acorn::Tuple<int, float>>::value,
              "Tuple<int, float> should be standard layout");

struct StructX {};
static_assert(std::is_standard_layout<acorn::Tuple<StructX>>::value,
              "Tuple<struct X> should be standard layout");
static_assert(std::is_standard_layout<acorn::Tuple<StructX, int>>::value,
              "Tuple<struct X, int> should be standard layout");
#endif

TEST(Tuple, ConstructEmptyTuple) {
  acorn::Tuple<> a;
  acorn::Tuple<> b{};

  (void)a;
  (void)b;
}

TEST(Tuple, ConstructFromSingleInts) {
  acorn::Tuple<int> a(0);
  acorn::Tuple<int> b{0};
  acorn::Tuple<int> c = {0};
  acorn::Tuple<int> d = acorn::Tuple<int>(0);
  acorn::Tuple<int> e = acorn::Tuple<int>{0};

  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
}

TEST(Tuple, AssignIntTuple) {
  acorn::Tuple<int> a{1};
  acorn::Tuple<int> const b = a;
  acorn::Tuple<int> c = b;
  acorn::Tuple<int> const d = std::move(a);

  (void)a;
  (void)b;
  (void)c;
  (void)d;
}

TEST(Tuple, ExtractFromIntTuple) {
  acorn::Tuple<int> a(0);

  EXPECT_EQ(acorn::get<0>(a), 0);
  acorn::get<0>(a) = 1;
  EXPECT_EQ(acorn::get<0>(a), 1);
}

TEST(Tuple, ExtractFromConstIntTuple) {
  acorn::Tuple<int, int> const a(0, 1);

  EXPECT_EQ(acorn::get<0>(a), 0);
  EXPECT_EQ(acorn::get<1>(a), 1);
}

TEST(Tuple, ConstructFromMultipleInts) {
  acorn::Tuple<int, int> a(0, 1);
  acorn::Tuple<int, int> b{0, 1};
  acorn::Tuple<int, int> c = {0, 1};
  acorn::Tuple<int, int> d = acorn::Tuple<int, int>(0, 1);
  acorn::Tuple<int, int> e = acorn::Tuple<int, int>{0, 1};

  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
}

struct NonCopiable {
  NonCopiable() = default;
  NonCopiable(int a) : val{a} {}
  NonCopiable(NonCopiable const&) = delete;
  NonCopiable(NonCopiable&&) = default;
  int val = 0;
};

TEST(Tuple, ConstructFromNonCopiable) {
  NonCopiable a{};
  acorn::Tuple<NonCopiable> b{std::move(a)};

  acorn::get<0>(b).val = 1;
  EXPECT_EQ(acorn::get<0>(b).val, 1);
}

TEST(Tuple, MoveFromNonCopiable) {
  NonCopiable a{1};
  acorn::Tuple<NonCopiable> b{std::move(a)};
  acorn::Tuple<NonCopiable> c{std::move(b)};
  EXPECT_EQ(acorn::get<0>(c).val, 1);
}

TEST(Tuple, MoveFromMultipleNonCopiable) {
  NonCopiable a{1};
  NonCopiable b{2};
  acorn::Tuple<NonCopiable, NonCopiable, int> c{std::move(a), std::move(b), 3};
  acorn::Tuple<NonCopiable, NonCopiable, int> d{std::move(c)};
  EXPECT_EQ(acorn::get<0>(d).val, 1);
  EXPECT_EQ(acorn::get<1>(d).val, 2);
  EXPECT_EQ(acorn::get<2>(d), 3);
}
