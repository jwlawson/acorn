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
#ifndef ACORN_TRAITS_WITH_REF_MATCHING_H_
#define ACORN_TRAITS_WITH_REF_MATCHING_H_

#include <type_traits>

namespace acorn {

template <typename Type, typename Target>
struct WithRefMatchingImpl {
  using type = Type;
};

template <typename Type, typename Target>
struct WithRefMatchingImpl<Type, Target&> {
  using type = typename std::add_lvalue_reference<Type>::type;
};

template <typename Type, typename Target>
struct WithRefMatchingImpl<Type, Target const&> {
  using type = typename std::add_lvalue_reference<
      typename std::add_const<Type>::type>::type;
};

template <typename Type, typename Target>
struct WithRefMatchingImpl<Type, Target&&> {
  using type = typename std::add_rvalue_reference<Type>::type;
};

template <typename Type, typename Target>
struct WithRefMatchingImpl<Type, Target const&&> {
  using type = typename std::add_rvalue_reference<
      typename std::add_const<Type>::type>::type;
};

template <typename Type, typename Target>
using WithRefMatching = typename WithRefMatchingImpl<Type, Target>::type;

}  // namespace acorn

#endif  // ACORN_TRAITS_WITH_REF_MATCHING_H_
