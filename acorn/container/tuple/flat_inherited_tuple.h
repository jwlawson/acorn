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
#ifndef ACORN_CONTAINER_TUPLE_FLAT_INHERITED_TUPLE_H_
#define ACORN_CONTAINER_TUPLE_FLAT_INHERITED_TUPLE_H_

#include "acorn/container/tuple/tuple_element.h"

#include "acorn/traits/remove_const_ref.h"
#include "acorn/traits/with_ref_matching.h"

#include "absl/utility/utility.h"

#include <cstddef>
#include <utility>

namespace acorn {

template <std::size_t I, typename Arg>
struct TupleHolder {
  Arg value_;
};

template <typename Indices, typename... Args>
struct FlatInheritedTupleImpl;

template <std::size_t... Is, typename... Args>
struct FlatInheritedTupleImpl<absl::index_sequence<Is...>, Args...>
    : TupleHolder<Is, Args>... {
  static constexpr bool is_flat_inherited_tuple = true;

  template <std::size_t... IsA, typename... ArgsA>
  constexpr FlatInheritedTupleImpl(absl::index_sequence<IsA...>,
                                   ArgsA&&... args)
      : TupleHolder<IsA, ArgsA>{std::forward<ArgsA>(args)}... {}

  constexpr FlatInheritedTupleImpl() = default;
};

template <typename... Args>
struct FlatInheritedTuple
    : FlatInheritedTupleImpl<absl::make_index_sequence<sizeof...(Args)>,
                             Args...> {
  using IndexSequence = absl::make_index_sequence<sizeof...(Args)>;
  using Base = FlatInheritedTupleImpl<IndexSequence, Args...>;

  template <typename... ArgsA>
  constexpr FlatInheritedTuple(ArgsA&&... args)
      : Base{IndexSequence{}, std::forward<ArgsA>(args)...} {}

  constexpr FlatInheritedTuple() = default;
  constexpr FlatInheritedTuple(FlatInheritedTuple&) = default;
  constexpr FlatInheritedTuple(FlatInheritedTuple const&) = default;
  constexpr FlatInheritedTuple(FlatInheritedTuple&&) = default;
  FlatInheritedTuple& operator=(FlatInheritedTuple const&) = default;
  FlatInheritedTuple& operator=(FlatInheritedTuple&&) = default;
};

template <std::size_t I, typename Tuple,
          typename std::enable_if<
              RemoveConstRef<Tuple>::is_flat_inherited_tuple, int>::type = 0>
constexpr WithRefMatching<TupleElementType<I, RemoveConstRef<Tuple>>, Tuple>
get(Tuple&& tuple) noexcept {
  using TupleType = TupleHolder<I, TupleElementType<I, RemoveConstRef<Tuple>>>;
  using TupleRefType = WithRefMatching<TupleType, Tuple>;
  return static_cast<TupleRefType>(tuple).value_;
}

}  // namespace acorn

#endif  // ACORN_CONTAINER_TUPLE_FLAT_INHERITED_TUPLE_H_
