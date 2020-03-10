#include "threads/logger.h"

#include <array>
#include <memory>
#include <vector>

#ifndef NDEBUG
#include <type_traits>
#endif

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

  SlotMap(SlotMap &&) noexcept = default;
  SlotMap& operator=(SlotMap &&) noexcept = default;

  /**
   * Insert a value into the slot map.
   *
   * @param value [in] The value to insert in the map.
   * @return The index at which the value was inserted.
   */
  size_t insert(T const& value) {
    if (++insert_index_ >= ChunkSize) {
      insert_chunk_ = get_new_chunk();
      insert_index_ = 0;
    }
    assert(insert_chunk_ != nullptr);
    insert_chunk_->chunk_->operator[](insert_index_) = value;

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
  Data data_;
  size_t num_chunks_used_ = 0;
  size_t first_chunk_offset_ = 0;
  DataHolder* insert_chunk_ = nullptr;
  size_t insert_index_ = ChunkSize;

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
};
