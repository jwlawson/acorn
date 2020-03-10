
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "container/slot_map.h"

#include <type_traits>

static_assert(!std::is_copy_constructible<SlotMap<size_t>>::value, "SlotMap should not be copiable, as that would require copying all data stored in multiple chunks.");

static_assert(!std::is_copy_assignable<SlotMap<size_t>>::value, "SlotMap should not be copiable, as that would require copying all data stored in multiple chunks.");

static_assert(std::is_move_constructible<SlotMap<size_t>>::value, "SlotMap should be move constructible.");

static_assert(std::is_move_assignable<SlotMap<size_t>>::value, "SlotMap should be move assignable.");

TEST(SlotMap, InsertAndFetchElements) {
  SlotMap<size_t> map;

  for (size_t i = 0; i < 100; ++i) {
    map.insert(i);
  }
  for (size_t i = 0; i < 100; ++i) {
    ASSERT_EQ(map[i], i);
  }
}

TEST(SlotMap, RemoveKeepsIndicesConsistent) {
  SlotMap<size_t> map;

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
