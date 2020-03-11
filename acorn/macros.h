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
#ifndef ACORN_MACROS_H_
#define ACORN_MACROS_H_

/**
 * @def ACORN_HAS_ATTRIBUTE(X)
 * Check whether the compiler has access to the given c++ attribute.
 */
#if defined __has_cpp_attribute
#define ACORN_HAS_ATTRIBUTE(X) __has_cpp_attribute(X)
#else
#define ACORN_HAS_ATTRIBUTE(X) 0
#endif

/**
 * @def ACORN_MAYBE_UNUSED
 * Attribute to specify that an entity may be unused and no warning should be
 * raised if it is not used.
 */
#if ACORN_HAS_ATTRIBUTE(maybe_unused)
#define ACORN_MAYBE_UNUSED [[maybe_unused]]
#elif ACORN_HAS_ATTRIBUTE(gnu::unused)
#define ACORN_MAYBE_UNUSED [[gnu::unused]]
#else
#define ACORN_MAYBE_UNUSED
#endif

#endif  // ACORN_MACROS_H_
