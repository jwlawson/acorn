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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "container/slot_map.h"

#include <type_traits>

static_assert(!std::is_copy_constructible<acorn::SlotMap<size_t>>::value,
              "SlotMap should not be copiable, as that would require copying "
              "all data stored in multiple chunks.");

static_assert(!std::is_copy_assignable<acorn::SlotMap<size_t>>::value,
              "SlotMap should not be copiable, as that would require copying "
              "all data stored in multiple chunks.");

static_assert(std::is_move_constructible<acorn::SlotMap<size_t>>::value,
              "SlotMap should be move constructible.");

static_assert(std::is_move_assignable<acorn::SlotMap<size_t>>::value,
              "SlotMap should be move assignable.");

TEST(SlotMap, InsertAndFetchElements) {
  acorn::SlotMap<size_t> map;

  for (size_t i = 0; i < 100; ++i) {
    map.insert(i);
  }
  for (size_t i = 0; i < 100; ++i) {
    ASSERT_EQ(map[i], i);
  }
}

TEST(SlotMap, RemoveKeepsIndicesConsistent) {
  acorn::SlotMap<size_t> map;

  for (size_t i = 0; i < 100; ++i) {
    map.insert(i);
  }
  for (size_t i = 0; i < 50; ++i) {
    map.erase(i);
  }
  for (size_t i = 50; i < 100; ++i) {
    ASSERT_EQ(map[i], i);
  }
}
