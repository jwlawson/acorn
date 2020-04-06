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
#ifndef ACORN_CONTAINER_TUPLE_NESTED_INHERITED_TUPLE_H_
#define ACORN_CONTAINER_TUPLE_NESTED_INHERITED_TUPLE_H_

#include "acorn/container/tuple/tuple_element.h"

#include "acorn/traits/remove_const_ref.h"
#include "acorn/traits/with_ref_matching.h"

#include <cstddef>
#include <utility>

namespace acorn {

template <typename... Args>
struct NestedInheritedTuple {};

template <typename First, typename... Rest>
struct NestedInheritedTuple<First, Rest...> : NestedInheritedTuple<Rest...> {
  template <typename FirstA, typename... RestA>
  constexpr NestedInheritedTuple(FirstA&& arg1, RestA&&... args)
      : NestedInheritedTuple<Rest...>{std::forward<RestA>(args)...},
        arg_{std::forward<FirstA>(arg1)} {}

  constexpr NestedInheritedTuple() = default;
  constexpr NestedInheritedTuple(NestedInheritedTuple&) = default;
  constexpr NestedInheritedTuple(NestedInheritedTuple const&) = default;
  constexpr NestedInheritedTuple(NestedInheritedTuple&&) = default;
  NestedInheritedTuple& operator=(NestedInheritedTuple const&) = default;
  NestedInheritedTuple& operator=(NestedInheritedTuple&&) = default;

  First arg_;
};

template <std::size_t I, typename Tuple,
          typename std::enable_if<std::is_base_of<NestedInheritedTuple<>,
                                                  RemoveConstRef<Tuple>>::value,
                                  int>::type = 0>
constexpr WithRefMatching<TupleElementType<I, RemoveConstRef<Tuple>>, Tuple>
get(Tuple&& tuple) noexcept {
  using TupleType = typename TupleElement<I, RemoveConstRef<Tuple>>::TupleType;
  using TupleRefType = WithRefMatching<TupleType, Tuple>;
  return static_cast<TupleRefType>(tuple).arg_;
}

}  // namespace acorn

#endif  // ACORN_CONTAINER_TUPLE_NESTED_INHERITED_TUPLE_H_
