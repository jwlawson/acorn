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
#ifndef ACORN_CONTAINER_TUPLE_H_
#define ACORN_CONTAINER_TUPLE_H_

#include "acorn/traits/remove_const_ref.h"
#include "acorn/traits/with_ref_matching.h"

#include <cstddef>
#include <utility>

namespace acorn {

template <std::size_t I, template <typename...> class Tuple, typename... Args>
struct TupleElementImpl;

template <std::size_t I, template <typename...> class Tuple, typename First,
          typename... Rest>
struct TupleElementImpl<I, Tuple, First, Rest...>
    : TupleElementImpl<I - 1, Tuple, Rest...> {};

template <template <typename...> class Tuple, typename First, typename... Rest>
struct TupleElementImpl<0, Tuple, First, Rest...> {
  using type = First;
  using TupleType = Tuple<First, Rest...>;
};

template <std::size_t I, typename Tuple>
struct TupleElement;

template <std::size_t I, template <typename...> class Tuple, typename... Args>
struct TupleElement<I, Tuple<Args...>> : TupleElementImpl<I, Tuple, Args...> {};

template <std::size_t I, typename Tuple>
using TupleElementType = typename TupleElement<I, Tuple>::type;

template <typename... Args>
struct MemberTuple {};

template <typename First, typename... Rest>
struct MemberTuple<First, Rest...> {
  constexpr static bool is_member_tuple = true;

  template <typename FirstA, typename... RestA>
  constexpr MemberTuple(FirstA&& arg1, RestA&&... args)
      : arg_{std::forward<FirstA>(arg1)}, rest_{std::forward<RestA>(args)...} {}

  constexpr MemberTuple() = default;
  constexpr MemberTuple(MemberTuple&) = default;
  constexpr MemberTuple(MemberTuple const&) = default;
  constexpr MemberTuple(MemberTuple&&) = default;
  MemberTuple& operator=(MemberTuple const&) = default;
  MemberTuple& operator=(MemberTuple&&) = default;

  First arg_;
  MemberTuple<Rest...> rest_;
};

template <std::size_t I>
struct MemberTupleGetHelper {
  template <typename Tuple>
  static constexpr WithRefMatching<TupleElementType<I, RemoveConstRef<Tuple>>,
                                   Tuple>
  get(Tuple&& tuple) noexcept {
    return MemberTupleGetHelper<I - 1>::get(tuple.rest_);
  }
};

template <>
struct MemberTupleGetHelper<0> {
  template <typename Tuple>
  static constexpr WithRefMatching<TupleElementType<0, RemoveConstRef<Tuple>>,
                                   Tuple>
  get(Tuple&& tuple) noexcept {
    return tuple.arg_;
  }
};

template <std::size_t I, typename Tuple,
          typename std::enable_if<Tuple::is_member_tuple, int>::type = 0>
constexpr WithRefMatching<TupleElementType<I, RemoveConstRef<Tuple>>, Tuple>
get(Tuple&& tuple) noexcept {
  return MemberTupleGetHelper<I>::get(tuple);
}

template <typename... Args>
struct InheritedTuple {};

template <typename First, typename... Rest>
struct InheritedTuple<First, Rest...> : InheritedTuple<Rest...> {
  template <typename FirstA, typename... RestA>
  constexpr InheritedTuple(FirstA&& arg1, RestA&&... args)
      : InheritedTuple<Rest...>{std::forward<RestA>(args)...},
        arg_{std::forward<FirstA>(arg1)} {}

  constexpr InheritedTuple() = default;
  constexpr InheritedTuple(InheritedTuple&) = default;
  constexpr InheritedTuple(InheritedTuple const&) = default;
  constexpr InheritedTuple(InheritedTuple&&) = default;
  InheritedTuple& operator=(InheritedTuple const&) = default;
  InheritedTuple& operator=(InheritedTuple&&) = default;

  First arg_;
};

template <std::size_t I, typename Tuple,
          typename std::enable_if<
              std::is_base_of<InheritedTuple<>, RemoveConstRef<Tuple>>::value,
              int>::type = 0>
constexpr WithRefMatching<TupleElementType<I, RemoveConstRef<Tuple>>, Tuple>
get(Tuple&& tuple) noexcept {
  using TupleType = typename TupleElement<I, RemoveConstRef<Tuple>>::TupleType;
  using TupleRefType = WithRefMatching<TupleType, Tuple>;
  return static_cast<TupleRefType>(tuple).arg_;
}

}  // namespace acorn

#endif  // ACORN_CONTAINER_TUPLE_H_
