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

#include "acorn/container/tuple/flat_inherited_tuple.h"
#include "acorn/container/tuple/member_tuple.h"
#include "acorn/container/tuple/nested_inherited_tuple.h"

#include "acorn/traits/remove_const_ref.h"
#include "acorn/traits/with_ref_matching.h"

#include <cstddef>
#include <utility>

namespace acorn {

#ifdef ACORN_STANDARD_LAYOUT_TUPLE
#define INLINE_MEMBER inline
#define INLINE_NESTED_INHERITED
#define INLINE_FLAT_INHERITED
#elif ACORN_NESTED_LAYOUT_TUPLE
#define INLINE_MEMBER
#define INLINE_NESTED_INHERITED inline
#define INLINE_FLAT_INHERITED
#else
#define INLINE_MEMBER
#define INLINE_NESTED_INHERITED
#define INLINE_FLAT_INHERITED inline
#endif

INLINE_MEMBER namespace member {
  template <typename... Args>
  using Tuple = MemberTuple<Args...>;
}

INLINE_NESTED_INHERITED namespace nested_inherited {
  template <typename... Args>
  using Tuple = NestedInheritedTuple<Args...>;
}

INLINE_FLAT_INHERITED namespace flat_inherited {
  template <typename... Args>
  using Tuple = FlatInheritedTuple<Args...>;
}

#undef INLINE_MEMBER
#undef INLINE_NESTED_INHERITED
#undef INLINE_FLAT_INHERITED

}  // namespace acorn

#endif  // ACORN_CONTAINER_TUPLE_H_
