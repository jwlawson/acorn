/*
 * Copyright (c) 2020, John Lawson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
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
#ifndef ACORN_CONTAINER_SLOT_MAP_H_
#define ACORN_CONTAINER_SLOT_MAP_H_

#include <array>
#include <memory>
#include <vector>

#ifndef NDEBUG
#include <type_traits>
#endif

namespace acorn {

template <typename T>
struct SlotMap {
 private:
  enum class CleanStrategyOpts {
    prefer_fast_insert,
    prefer_fast_erase,
  };
  static constexpr auto CleanStrategy = CleanStrategyOpts::prefer_fast_insert;

  static constexpr size_t ChunkSize = 64;
  using ChunkMask = uint64_t;
  using Chunk = std::array<T, ChunkSize>;

  struct DataHolder {
    // It would be nice if this was stored as a unique_ptr, instead of a raw
    // pointer, so that its memory is managed automatically and the destructor
    // of the SlotMap can be trivial. However that prevents the DataHolder being
    // trivially move assignable and hence prevents optimisations in the
    // std::rotate used to free up chunks of memory.
    Chunk* chunk_;
    ChunkMask free_mask_;

    DataHolder(Chunk* chunk_ptr, ChunkMask free_mask) noexcept
        : chunk_{chunk_ptr}, free_mask_{free_mask} {}

    DataHolder(DataHolder const&) = delete;
    DataHolder& operator=(DataHolder const&) = delete;

    DataHolder(DataHolder&&) noexcept = default;
    DataHolder& operator=(DataHolder&&) noexcept = default;
  };
#ifndef NDEBUG
  static_assert(std::is_move_constructible<DataHolder>::value,
                "DataHolder must be move constructible.");
  static_assert(std::is_trivially_move_constructible<DataHolder>::value,
                "DataHolder should be trivially move constructible to enable "
                "rotate optimisations.");
  static_assert(std::is_nothrow_move_constructible<DataHolder>::value,
                "DataHolder should not throw in move constructor.");
  static_assert(std::is_move_assignable<DataHolder>::value,
                "DataHolder must be move assignable.");
  static_assert(std::is_trivially_move_assignable<DataHolder>::value,
                "DataHolder should be trivially move assignable to enable "
                "rotate optimisations.");
  static_assert(std::is_nothrow_move_assignable<DataHolder>::value,
                "DataHolder should not throw in move assignment.");
#endif

  using Data = std::vector<DataHolder>;

 public:
  SlotMap() = default;

  SlotMap(SlotMap const&) = delete;
  SlotMap& operator=(SlotMap const&) = delete;

  SlotMap(SlotMap&&) noexcept = default;
  SlotMap& operator=(SlotMap&&) noexcept = default;

  /**
   * Insert a value into the slot map.
   *
   * @tparam Type Universal reference type that should atch the underlying data
   *         type stored in the slot map.
   *
   * @param value [in] The value to insert in the map.
   * @return The index at which the value was inserted.
   */
  template <typename Type>
  size_t insert(Type&& value) {
    if (++insert_index_ >= ChunkSize) {
      insert_chunk_ = get_new_chunk();
      insert_index_ = 0;
    }
    assert(insert_chunk_ != nullptr);
    insert_chunk_->chunk_->operator[](insert_index_) =
        std::forward<Type>(value);

    if (CleanStrategy == CleanStrategyOpts::prefer_fast_erase) {
      clean_any_empty_chunks();
    }

    size_t chunk_index = insert_chunk_ - data_.data();
    return compute_flat_index(chunk_index, insert_index_);
  }

  /**
   * Remove the value at a given index from the slot map.
   *
   * @param index [in] The index to remove from the map.
   */
  void erase(size_t index) {
    constexpr ChunkMask mask_one = 1u;
    auto c_idx = compute_chunk_index(index);
    auto& holder = data_[c_idx.first];
    size_t bit_to_set = mask_one << c_idx.second;
    holder.free_mask_ |= bit_to_set;

    if (CleanStrategy == CleanStrategyOpts::prefer_fast_insert) {
      clean_any_empty_chunks();
    }
  }

  /**
   * Access an element in the map.
   *
   * @param The index at which the element is stored.
   * @return A reference to the sotred element.
   */
  T& operator[](size_t index) noexcept {
    auto c_idx = compute_chunk_index(index);
    return data_[c_idx.first].chunk_->operator[](c_idx.second);
  }

  /** @copydoc operator[](size_t) */
  T const& operator[](size_t index) const noexcept {
    auto c_idx = compute_chunk_index(index);
    return data_[c_idx.first].chunk_->operator[](c_idx.second);
  }

  /** Destructor to free up the map's memory. */
  ~SlotMap() {
    for (auto&& holder : data_) {
      delete holder.chunk_;
    }
  }

 private:
  size_t compute_flat_index(size_t chunk_idx, size_t index_in_chunk) const
      noexcept {
    return chunk_idx * ChunkSize + index_in_chunk + first_chunk_offset_;
  }

  std::pair<size_t, size_t> compute_chunk_index(size_t index) const noexcept {
    assert(index >= first_chunk_offset_);
    size_t adjusted_idx = index - first_chunk_offset_;
    size_t chunk_index = adjusted_idx / ChunkSize;
    size_t index_in_chunk = adjusted_idx % ChunkSize;
    return {chunk_index, index_in_chunk};
  }

  DataHolder* get_new_chunk() {
    if (num_chunks_used_++ == data_.size()) {
      // All current chunks in use, so construct a new one.
      data_.emplace_back(new Chunk{}, 0u);
      return &data_.back();
    } else {
      // Reuse an existing but unused chunk
      return &data_[num_chunks_used_];
    }
  }

  void clean_any_empty_chunks() noexcept {
    static constexpr auto full_free_mask = static_cast<ChunkMask>(-1);
    size_t num_to_clean = 0;
    for (auto&& holder : data_) {
      if (holder.free_mask_ != full_free_mask) {
        break;
      }
      ++num_to_clean;
    }
    if (num_to_clean > 0) {
      mark_front_chunks_unused(num_to_clean);
    }
  }

  void mark_front_chunks_unused(size_t num_to_mark) noexcept {
    auto chunk_iter = begin(data_);
    for (size_t i = 0; i < num_to_mark; ++i) {
      (chunk_iter + i)->free_mask_ = 0u;
    }
    auto next_chunk_iter = chunk_iter + num_to_mark;
    std::rotate(chunk_iter, next_chunk_iter, end(data_));
    num_chunks_used_ -= num_to_mark;
    insert_chunk_ -= num_to_mark;
    first_chunk_offset_ += ChunkSize * num_to_mark;
  }

  /** Vector of DataHolders pointing to the data chunks. */
  Data data_;
  /** Number of data chunks in use at this point in time. */
  size_t num_chunks_used_ = 0;
  /** Index offset of the first data chunk. */
  size_t first_chunk_offset_ = 0;
  /** A pointer to the chunk to insert the next value into. */
  DataHolder* insert_chunk_ = nullptr;
  /** Index into the chunk to insert the next value into. */
  size_t insert_index_ = ChunkSize;

};

}  // namespace acorn

#endif  // ACORN_CONTAINER_SLOT_MAP_H_
