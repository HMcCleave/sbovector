#ifndef SBOVECTOR_HPP
#define SBOVECTOR_HPP

#include <algorithm>
#include <array>
#ifndef SBOVECTOR_ASSERT
#include <cassert>
#endif
#include <exception>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

// if true: nullptr == allocator.allocate() will throw std::bad_alloc exception
// if false: assert and terminate
#ifndef SBOVECTOR_THROW_BAD_ALLOC
#define SBOVECTOR_THROW_BAD_ALLOC false
#endif

#define SBOVECTOR_THROW_ALLOC noexcept(!SBOVECTOR_THROW_BAD_ALLOC)

namespace details_ {

// throw within a noexcept is a call to terminate
// but this will surpress a cavalcade of warnings
#if SBO_VECTOR_THROW_BAD_ALLOC
#define SBOVECTOR_DO_BAD_ALLOC_THROW throw std::bad_alloc()
#else 
#define SBOVECTOR_DO_BAD_ALLOC_THROW std::terminate()
#endif

#ifndef SBOVECTOR_ASSERT
#define SBOVECTOR_ASSERT(cond, message) assert(cond&& message)
#endif

#define SBOVEC_OOM "SBOVector allocation failure (likely OOM)!"

// if true will supress static_assertions related to DataType being
// nothrow constructable/movable/moveconstructable as applicable,
// note, excpetions thrown that violate those requirements will terminate
#ifndef SBOVECTOR_RELAX_EXCEPTION_REQUIREMENTS
#define SBOVECTOR_RELAX_EXCEPTION_REQUIREMENTS false
#endif

constexpr bool relax_except = SBOVECTOR_RELAX_EXCEPTION_REQUIREMENTS;

constexpr size_t SuggestGrowth(size_t old_size) {
  constexpr size_t kSBOVectorGrowthFactor = 2;
  return old_size * kSBOVectorGrowthFactor;
}

template <typename T>
struct is_compactable {
  static constexpr bool value = (std::is_empty_v<T> && !std::is_final_v<T>);
};

template <typename T>
constexpr bool is_compactable_v = is_compactable<T>::value;

template <typename T, typename = void>
struct is_iterator {
  static constexpr bool value = false;
};

template <typename T>
struct is_iterator<
  T,
  typename std::enable_if_t<
    !std::is_same_v<typename std::iterator_traits<T>::value_type, void>
  >
> {
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template<typename T>
using AlignedStorage = std::aligned_storage_t<sizeof(T), alignof(T)>;


template<typename DataType, size_t Capacity, bool compact>
struct SBOVectorStorage;

// Do Not Use this class directly, it is an incomplete helper class
template<typename DataType, size_t Capacity>
struct SBOVectorStorage<DataType, Capacity, true> {
  struct External {
    DataType* data_;
    size_t capacity_;
  };
  size_t count_;
  union {
    std::array<AlignedStorage<DataType>, Capacity> inline_;
    External external_;
  };

  void set_count(size_t count) { count_ = count; }

  size_t count() const { return count_; }

  size_t capacity() const {
    if (count_ > Capacity) {
      return external_.capacity_;
    }
    return Capacity;
  }

  DataType* data() { 
    if (count_ > Capacity) {
      return external_.data_;
    }
    return reinterpret_cast<DataType*>(inline_.data());
  }

  const DataType* data() const {
    if (count_ > Capacity) {
      return external_.data_;
    }
    return reinterpret_cast<const DataType*>(inline_.data());
  }

  void set_external(DataType* pData, size_t cap) {
    external_ = {pData, cap};
  }

  DataType* external_data() const {
    // expectation external_ must still be valid
    return external_.data_;
  }

  size_t external_capacity() const {
    // expectation external_ must still be valid
    return external_.capacity_;
  }

  void prep_change_to_inline() { 
    // compiler/optimizer should optimize this away
    new (&inline_) decltype(inline_)(); 
  }

  void prep_change_to_external() {
    // compiler/optimizer should optimize this away
    std::destroy_at(&inline_);
  }
};

// Do Not Use this class directly, it is an incomplete helper class
template <typename DataType, size_t Capacity>
struct SBOVectorStorage<DataType, Capacity, false> {
  size_t count_;
  DataType* data_;
  union {
    std::array<AlignedStorage<DataType>, Capacity> inline_;
    size_t capacity_;
  };

  void set_count(size_t count) { count_ = count; }

  size_t count() const { return count_; }

  size_t capacity() const {
    if (count_ > Capacity) {
      return capacity_;
    }
    return Capacity;
  }

  DataType* data() { return data_; }

  const DataType* data() const { return data_; }

  void set_external(DataType* pData, size_t cap) { 
    data_ = pData;
    capacity_ = cap;
  }

  DataType* external_data() const {
    // expectation external_ must still be valid
    return data_;
  }

  size_t external_capacity() const {
    // expectation external_ must still be valid
    return capacity_;
  }

  void prep_change_to_inline() {
    // compiler/optimizer should optimize this away
    new (&inline_) decltype(inline_)();
    data_ = reinterpret_cast<DataType*>(inline_.data());
  }

  void prep_change_to_external() {
    // compiler/optimizer should optimize this away
    std::destroy_at(&inline_);
  }
};

// Do Not Use this class directly, it is an incomplete helper class
template <
  typename DataType,
  size_t BufferSize,
  typename Allocator,
  bool Compact,
  bool = is_compactable_v<Allocator>
>
struct SBOVectorBase
    : private Allocator,
      public SBOVectorStorage<DataType, BufferSize, Compact> {

  SBOVectorBase() : Allocator() {
    this->set_count(0);
    this->prep_change_to_inline();
  }

  SBOVectorBase(const Allocator& alloc) : Allocator(alloc) { 
    this->set_count(0);
    this->prep_change_to_inline();
  }

  ~SBOVectorBase() { /*intentionally empty*/
  }

  Allocator& access_allocator() { return *this; }
  Allocator get_allocator() const { return *this; }

  using StorageBase = SBOVectorStorage<DataType, BufferSize, Compact>;
  using StorageBase::capacity;
  using StorageBase::count;
  using StorageBase::data;
  using StorageBase::set_external;
  using StorageBase::external_capacity;
  using StorageBase::prep_change_to_inline;
  using StorageBase::prep_change_to_external;
};

template <
  typename DataType,
  size_t BufferSize,
  typename Allocator,
  bool Compact
>
struct SBOVectorBase<DataType, BufferSize, Allocator, Compact, false>
    : public SBOVectorStorage<DataType, BufferSize, Compact> {
  Allocator alloc_;

  SBOVectorBase() : alloc_() {
    this->set_count(0);
    this->prep_change_to_inline();
  }

  SBOVectorBase(const Allocator& alloc) : alloc_(alloc) {
    this->set_count(0);
    this->prep_change_to_inline();
  }

  ~SBOVectorBase() { /*intentionally empty*/
  }

  Allocator& access_allocator() { return alloc_; }
  Allocator get_allocator() const { return alloc_; }

  using StorageBase = SBOVectorStorage<DataType, BufferSize, Compact>;
  using StorageBase::capacity;
  using StorageBase::count;
  using StorageBase::data;
};

// Do Not Use this class directly, it is an incomplete helper class
template <typename DataType, size_t BufferSize, typename Allocator, bool Compact>
struct VectorImpl final
    : public SBOVectorBase<DataType, BufferSize, Allocator, Compact> {
  using BaseType = SBOVectorBase<DataType, BufferSize, Allocator, Compact>;

  using BaseType::get_allocator;
  using BaseType::capacity;
  using BaseType::count;
  using BaseType::data;

  VectorImpl() : BaseType() {}
  VectorImpl(const Allocator& alloc) : BaseType(alloc) {}

  ~VectorImpl() { clear(); }

  DataType* begin() {
    return data();
  }

  const DataType* begin() const {
    return const_cast<const DataType*>(data());
  }

  DataType* end() { return begin() + count(); }
  const DataType* end() const { return begin() + count(); }

  void clear() noexcept {
    std::destroy(begin(), end());
    if (count() > BufferSize) {
      get_allocator().deallocate(data(), capacity());
      this->prep_change_to_inline();
    }
    this->set_count(0);
  }

 private:
  void insert_unninitialized_in_cap(size_t pos, size_t insert_count) noexcept {
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);
    static_assert(relax_except || std::is_nothrow_move_constructible_v<DataType>);

    auto new_size = count() + insert_count;
    auto uninit_count = std::min(count() - pos, insert_count);
    auto assign_count = count() - (pos + uninit_count);
    std::uninitialized_move_n(
      end() - uninit_count,
      uninit_count,
      begin() + new_size - uninit_count
    );

    std::move_backward(begin() + pos, begin() + pos + assign_count, end());

    std::destroy_n(begin() + pos, count() - (pos + assign_count));
    this->set_count(count() + insert_count);
  }

  void insert_unninitialized_with_growth(size_t pos, size_t insert_count)
      SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except || std::is_nothrow_move_constructible_v<DataType>);
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    const size_t new_size = count() + insert_count;
    const size_t new_cap = std::max(new_size, SuggestGrowth(count()));

    auto new_buffer = get_allocator().allocate(new_cap);

    if (!new_buffer) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }

    std::uninitialized_move_n(begin(), pos, new_buffer);
    std::uninitialized_move_n(
      begin() + pos,
      count() - pos,
      new_buffer + pos + insert_count
    );

    clear();

    this->prep_change_to_external();
    this->set_count(new_size);
    this->set_external(new_buffer, new_cap);
  }

 public:
  // Create Uninitialized Space x insert_count at begin() + pos
  // eg: if X is a value and U uninitialized space
  // { X, X, X, X }.insert_uniinitialized(2,3) -> { X, X, U, U, U, X, X }
  void insert_unninitialized(
        size_t pos,
        size_t insert_count
      ) SBOVECTOR_THROW_ALLOC {
    if (count() + insert_count <= capacity()) {
      insert_unninitialized_in_cap(pos, insert_count);
    } else {
      insert_unninitialized_with_growth(pos, insert_count);
    }
  }

  // bring external_ -> inline_
  // assumes count() <= BufferSize
  // assumes external_ is in use
  void internalize() noexcept {
    static_assert(!relax_except || std::is_nothrow_move_constructible_v<DataType>);

    auto external_ptr_copy = this->external_data();
    auto external_capacity_copy = this->external_capacity();

    this->prep_change_to_inline();

    std::uninitialized_move_n(external_ptr_copy, count(), data());

    std::destroy_n(external_ptr_copy, count());
    get_allocator().deallocate(external_ptr_copy, external_capacity_copy);
  }

 private:
  template <
    size_t Size1,
    typename Allocator1,
    bool Compact1,
    size_t Size2,
    typename Allocator2,
    bool Compact2
  >
  inline static void no_alloc_swap(
      VectorImpl<DataType, Size1, Allocator1, Compact1>& A,
      VectorImpl<DataType, Size2, Allocator2, Compact2>& B) noexcept {
    static_assert(relax_except || std::is_nothrow_move_constructible_v<DataType>);

    auto small_size = std::min(A.count(), B.count());
    auto size_diff = std::max(A.count(), B.count()) - small_size;

    DataType* source = A.begin() + small_size;
    DataType* sink = B.begin() + small_size;

    std::swap_ranges(A.begin(), source, B.begin());

    if (A.count() == small_size) {
      std::swap(source, sink);
    }

    std::uninitialized_move_n(source, size_diff, sink);
    std::destroy_n(source, size_diff);
    auto a_count = A.count();
    A.set_count(B.count());
    B.set_count(a_count);
  }

  template <
    size_t Size1,
    typename Allocator1,
    bool Compact1,
    size_t Size2,
    typename Allocator2,
    bool Compact2
  >
  inline static void one_alloc_swap(
        VectorImpl<DataType, Size1, Allocator1, Compact1>& allocating,
        VectorImpl<DataType, Size2, Allocator2, Compact2>& remaining
      ) SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except || std::is_nothrow_move_constructible_v<DataType>);

    auto new_data_size = remaining.count();
    auto new_data = allocating.access_allocator().allocate(new_data_size);

    if (!new_data) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }
    std::uninitialized_move_n(remaining.begin(), new_data_size, new_data);
    std::destroy(remaining.begin(), remaining.end());
    std::uninitialized_move_n(
      allocating.begin(),
      allocating.count(),
      remaining.begin()
    );

    if (remaining.count() > Size2 && allocating.count() <= Size2) {
      remaining.count_ = allocating.count();
      remaining.internalize();
    } else {
      remaining.count_ = allocating.count();
    }

    allocating.clear();
    std::destroy_at(&allocating.inline_);
    allocating.count_ = new_data_size;
    allocating.set_external(new_data, new_data_size);
  }

 public:
  template <size_t OtherSize, typename OtherAllocator, bool OtherCompact>
  inline void swap_cross(
        VectorImpl<DataType, OtherSize, OtherAllocator, OtherCompact>& that
      ) SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    const auto this_is_sufficient = (capacity() >= that.count());
    const auto that_is_sufficient = (that.capacity() >= count());

    if (this_is_sufficient && that_is_sufficient) {
      no_alloc_swap(*this, that);
      if (count() <= BufferSize && that.count() > BufferSize)
        internalize();
      if (that.count() <= OtherSize && count() > OtherSize)
        that.internalize();
    } else if (this_is_sufficient) {
      one_alloc_swap(that, *this);
    } else if (that_is_sufficient) {
      one_alloc_swap(*this, that);
    } else {
      auto new_this_size = that.count();
      auto new_this = this->access_allocator().allocate(that.count());

      if (!new_this) {
        SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
        SBOVECTOR_DO_BAD_ALLOC_THROW;
      }

      auto new_that_size = count();
      auto new_that = that.access_allocator().allocate(count());

      if (!new_that) {
        SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
        this->access_allocator().deallocate(new_that, new_that_size);
        SBOVECTOR_DO_BAD_ALLOC_THROW;
      }

      std::uninitialized_move_n(that.begin(), that.count(), new_this);
      std::uninitialized_move_n(begin(), count(), new_that);

      clear();
      that.clear();

      this->prep_change_to_external();
      that.prep_change_to_external();
      that.count_ = new_that_size;
      that.set_external(new_that, new_that_size);
      this->set_count(new_this_size);
      this->set_external(new_this, new_this_size);
      // No internalizations are possible here as capacity has a lower bound
      // and this and that are both insufficiant
    }
  }

  template <size_t OtherSize, bool OtherCompact>
  void swap(VectorImpl<DataType, OtherSize, Allocator, OtherCompact>& that)
      SBOVECTOR_THROW_ALLOC {
    const auto this_is_inline = (count() <= BufferSize);
    const auto that_is_inline = (that.count() <= OtherSize);
    const auto this_will_be_inline = (that.count() <= BufferSize);
    const auto that_will_be_inline = (count() <= OtherSize);
    const auto can_swap_external =
        std::allocator_traits<Allocator>::is_always_equal::value ||
        (that.access_allocator() == this->access_allocator());

    if (!(this_is_inline || that_is_inline) && can_swap_external) {
      auto this_count = this->count();
      this->set_count(that.count());
      that.set_count(this_count);
      auto this_data = data();
      auto this_cap = capacity();
      auto that_data = that.data();
      auto that_cap = that.capacity();
      this->set_external(that_data, that_cap);
      that.set_external(this_data, this_cap);
      if (this_will_be_inline) {
        internalize();
      }
      if (that_will_be_inline) {
        that.internalize();
      }
    } else if (this_is_inline && that_is_inline 
            && this_will_be_inline && that_will_be_inline) {
      no_alloc_swap(*this, that);
    } else {
      swap_cross(that);
    }
  }

  void reserve(size_t new_capacity) SBOVECTOR_THROW_ALLOC {
    // If calling reserve with count() <= BufferSize
    // reserve will leave this in an invalid state
    // care must be taken to resolve this
    // (increase count() + construct required elements)
    // immediately.
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    new_capacity = std::max(new_capacity, SuggestGrowth(count()));

    auto new_data = this->access_allocator().allocate(new_capacity);
    if (!new_data) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }

    std::uninitialized_move_n(begin(), count(), new_data);
    std::destroy(begin(), end());

    if (count() > BufferSize) {
      this->access_allocator().deallocate(data(), capacity());
    } else {
      this->prep_change_to_external();
    }

    this->set_external(new_data, new_capacity);
  }

  void shrink_to_fit() SBOVECTOR_THROW_ALLOC {
    // requires count() > BufferSize
    // requires count() < capacity()
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    auto new_data = get_allocator().allocate(count());
    if (!new_data) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }

    std::uninitialized_move_n(begin(), count(), new_data);
    std::destroy(begin(), end());
    get_allocator().deallocate(data(), capacity());

    this->set_external(new_data, count());
  }

  DataType* erase(const DataType* pos, size_t p_count) noexcept {
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    const auto must_internalize =
        (count() > BufferSize) && ((count() - p_count) <= BufferSize);
    auto i_pos = std::distance(const_cast<const DataType*>(begin()), pos);
    std::move(begin() + i_pos + p_count, end(), begin() + i_pos);
    std::destroy_n(begin() + (count() - p_count), p_count);
    this->set_count(count() - p_count);
    if (must_internalize)
      internalize();
    return begin() + i_pos;
  }

  template <typename... Args>
  DataType& emplace_back(Args&&... args) SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except || std::is_nothrow_constructible_v<DataType, Args...>);
    if (count() == capacity()) {
      reserve(count() + 1);
    }
    this->set_count(count() + 1);
    // have to increment then offset by count() - 1
    // as reserve(count() + 1) will leave an invalid state
    // if count() == BufferSize and begin() is thus an invalid call
    DataType* out = begin() + count() - 1;
    new (out) DataType(std::forward<Args>(args)...);
    return *out;
  }
};

}  // namespace details_

template <
  typename DataType,
  size_t BufferSize,
  typename Allocator = std::allocator<DataType>,
  bool UseCompactStorage = false
>
class SBOVector {
  static_assert(std::is_move_assignable_v<DataType>);
  static_assert(BufferSize > 0);
  static_assert(
    std::is_convertible_v<
      typename std::allocator_traits<Allocator>::pointer,
      DataType*
    >
  );
  static_assert(
    std::is_convertible_v<
      typename std::allocator_traits<Allocator>::const_pointer,
      const DataType*
    >
  );

 private:
  details_::VectorImpl<
    DataType,
    BufferSize,
    Allocator,
    UseCompactStorage
  > impl_;

 public:
  using value_type = DataType;
  using allocator_type = Allocator;
  using size_type = size_t;
  using pointer = DataType*;
  using const_pointer = const DataType*;
  using iterator = DataType*;
  using const_iterator = const DataType*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reference = DataType&;
  using const_reference = const DataType&;

  SBOVector() noexcept : impl_() {}

  explicit SBOVector(const Allocator& alloc) noexcept : impl_(alloc) {}

  SBOVector(
        size_t count,
        const DataType& value,
        const Allocator& alloc = Allocator()
      ) SBOVECTOR_THROW_ALLOC : impl_(alloc) {
    resize(count, value);
  }

  explicit SBOVector(
        size_t count,
        const Allocator& alloc = Allocator()
      ) SBOVECTOR_THROW_ALLOC : SBOVector(alloc) {
    resize(count);
  }

  template <
    typename InputIter,
    typename = std::enable_if_t<details_::is_iterator_v<InputIter>>
  >
  SBOVector(
        InputIter p_begin,
        InputIter p_end,
        const Allocator& alloc = Allocator()
      ) SBOVECTOR_THROW_ALLOC : impl_(alloc) {
    insert(begin(), p_begin, p_end);
  }

  SBOVector(
        std::initializer_list<DataType> init_list,
        const Allocator& alloc = Allocator()
      ) SBOVECTOR_THROW_ALLOC : SBOVector(init_list.begin(), init_list.end(), alloc) {}

  SBOVector(const SBOVector& copy) SBOVECTOR_THROW_ALLOC
      : SBOVector(copy.begin(), copy.end(), copy.get_allocator()) {}

  template <size_t OtherSize>
  SBOVector(const SBOVector<DataType, OtherSize, Allocator>& copy) SBOVECTOR_THROW_ALLOC
      : SBOVector(copy.begin(), copy.end(), copy.get_allocator()) {}

  template <size_t OtherSize, typename AllocatorType>
  SBOVector(
        const SBOVector<DataType, OtherSize, AllocatorType>& copy,
        const Allocator& alloc = Allocator()
      ) SBOVECTOR_THROW_ALLOC : SBOVector(copy.begin(), copy.end(), alloc) {}

  SBOVector(SBOVector&& move_from) SBOVECTOR_THROW_ALLOC
      : SBOVector(move_from.get_allocator()) {
    swap(move_from);
  }

  template <size_t OtherSize, bool OtherCompact>
  SBOVector(SBOVector<DataType, OtherSize, Allocator, OtherCompact>&& move_from) noexcept
      : SBOVector(move_from.get_allocator()) {
    swap(move_from);
  }

  template <size_t OtherSize, typename AllocatorType, bool OtherCompact>
  SBOVector(
        SBOVector<DataType, OtherSize, AllocatorType, OtherCompact>&& move_from,
        const Allocator& alloc = Allocator()
      ) SBOVECTOR_THROW_ALLOC : SBOVector(alloc) {
    swap(move_from);
  }

  ~SBOVector() { clear(); }

  SBOVector& operator=(const SBOVector& other) SBOVECTOR_THROW_ALLOC {
    assign(other.begin(), other.end());
    return *this;
  }

  template <size_t OtherSize, typename AllocatorType, bool OtherCompact>
  SBOVector& operator=(
        const SBOVector<DataType, OtherSize, AllocatorType, OtherCompact>& other
      ) SBOVECTOR_THROW_ALLOC {
    assign(other.begin(), other.end());
    return *this;
  }

  SBOVector& operator=(SBOVector&& that) noexcept {
    swap(that);
    return *this;
  }

  template <size_t OtherSize, typename AllocatorType, bool OtherCompact>
  SBOVector& operator=(SBOVector<DataType, OtherSize, AllocatorType, OtherCompact>&& that)
      SBOVECTOR_THROW_ALLOC {
    swap(that);
    return *this;
  }

  SBOVector& operator=(std::initializer_list<DataType> init)
      SBOVECTOR_THROW_ALLOC {
    assign(init.begin(), init.end());
    return *this;
  }

  void assign(size_t count, const DataType& value) SBOVECTOR_THROW_ALLOC {
    for (auto iter = begin(), end_ = begin() + std::min(size(), count);
         iter != end_; ++iter) {
      *iter = value;
    }
    resize(count, value);
  }

  template <
    typename InputIt,
    typename = std::enable_if_t<details_::is_iterator_v<InputIt>>
  >
  void assign(InputIt p_begin, InputIt p_end) SBOVECTOR_THROW_ALLOC {
    auto new_size = static_cast<size_t>(std::distance(p_begin, p_end));
    for (auto iter = begin(), end_ = end(); iter != end_ && p_begin != p_end;
         ++iter, ++p_begin) {
      *iter = *p_begin;
    }
    if (new_size > size()) {
      insert(end(), p_begin, p_end);
    } else {
      resize(new_size);
    }
  }

  void assign(std::initializer_list<DataType> list) SBOVECTOR_THROW_ALLOC {
    assign(list.begin(), list.end());
  }

  [[nodiscard]] Allocator get_allocator() const noexcept {
    return impl_.get_allocator();
  }

  [[nodiscard]] reference at(size_t index) noexcept {
    return *(begin() + index);
  }
  [[nodiscard]] const_reference at(size_t index) const noexcept {
    return *(cbegin() + index);
  }

  [[nodiscard]] reference operator[](size_t index) noexcept {
    return at(index);
  }
  [[nodiscard]] const_reference operator[](size_t index) const noexcept {
    return at(index);
  }

  [[nodiscard]] reference front() noexcept { return at(0); }
  [[nodiscard]] const_reference front() const noexcept { return at(0); }

  [[nodiscard]] reference back() noexcept { return at(size() - 1); }
  [[nodiscard]] const_reference back() const noexcept { return at(size() - 1); }

  [[nodiscard]] pointer data() noexcept { return impl_.begin(); }
  [[nodiscard]] const_pointer data() const noexcept { return impl_.begin(); }
  [[nodiscard]] const_pointer cdata() const noexcept { return data(); }

  [[nodiscard]] iterator begin() noexcept { return data(); }
  [[nodiscard]] const_iterator begin() const noexcept { return data(); }
  [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

  [[nodiscard]] iterator end() noexcept { return begin() + size(); }
  [[nodiscard]] const_iterator end() const noexcept {
    return cbegin() + size();
  }
  [[nodiscard]] const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }
  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return std::make_reverse_iterator(cend());
  }
  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }
  [[nodiscard]] reverse_iterator rend() noexcept {
    return std::make_reverse_iterator(begin());
  }
  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return std::make_reverse_iterator(cbegin());
  }
  [[nodiscard]] const_reverse_iterator crend() const noexcept {
    return rend();
  }

  [[nodiscard]] bool empty() const noexcept { return 0 == size(); }
  [[nodiscard]] size_t size() const noexcept { return impl_.count_; }
  [[nodiscard]] size_t max_size() const noexcept {
    return std::allocator_traits<Allocator>::max_size();
  }

  void reserve_if_external(size_t requested_capacity) SBOVECTOR_THROW_ALLOC {
    if (requested_capacity <= capacity() || size() <= BufferSize)
      return;
    impl_.reserve(requested_capacity);
  }

  [[nodiscard]] size_t capacity() const noexcept { return impl_.capacity(); }

  void shrink_to_fit_if_external() SBOVECTOR_THROW_ALLOC {
    if (size() <= BufferSize || size() == capacity())
      return;
    impl_.shrink_to_fit();
  }

  void clear() noexcept { impl_.clear(); }

  iterator insert(const_iterator pos, const DataType& v) SBOVECTOR_THROW_ALLOC {
    return insert(pos, 1, v);
  }

  iterator insert(const_iterator pos, DataType&& mv) SBOVECTOR_THROW_ALLOC {
    const auto i_pos = static_cast<size_t>(std::distance(cbegin(), pos));
    impl_.insert_unninitialized(i_pos, 1);
    auto out = begin() + i_pos;
    new (out) DataType(std::move(mv));
    return out;
  }

  iterator insert(
        const_iterator pos,
        size_t count,
        const DataType& v
      ) SBOVECTOR_THROW_ALLOC {
    const auto i_pos = static_cast<size_t>(std::distance(cbegin(), pos));
    impl_.insert_unninitialized(i_pos, count);
    std::uninitialized_fill_n(begin() + i_pos, count, v);
    return begin() + i_pos;
  }

  template <
    typename InputIt,
    typename = std::enable_if_t<details_::is_iterator_v<InputIt>>
  >
  iterator insert(
        const_iterator pos,
        InputIt p_begin,
        InputIt p_end
      ) SBOVECTOR_THROW_ALLOC {
    const auto i_pos = static_cast<size_t>(std::distance(cbegin(), pos));
    impl_.insert_unninitialized(i_pos, static_cast<size_t>(std::distance(p_begin, p_end)));
    for (auto iter = begin() + i_pos; p_begin != p_end; ++p_begin, ++iter) {
      new (iter) DataType(*p_begin);
    }
    return begin() + i_pos;
  }

  iterator insert(
        const_iterator pos,
        std::initializer_list<DataType> list
      ) SBOVECTOR_THROW_ALLOC {
    return insert(pos, list.begin(), list.end());
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) SBOVECTOR_THROW_ALLOC {
    auto i_pos = static_cast<size_t>(std::distance(cbegin(), pos));
    impl_.insert_unninitialized(i_pos, 1);
    auto out = begin() + i_pos;
    new (out) DataType(std::forward<Args>(args)...);
    return out;
  }

  iterator erase(const_iterator pos) noexcept { return impl_.erase(pos, 1); }

  iterator erase(const_iterator p_begin, const_iterator p_end) noexcept {
    return impl_.erase(p_begin, static_cast<size_t>(std::distance(p_begin, p_end)));
  }

  void push_back(const DataType& value) SBOVECTOR_THROW_ALLOC {
    emplace_back(value);
  }

  void push_back(DataType&& value) SBOVECTOR_THROW_ALLOC {
    emplace_back(std::forward<DataType>(value));
  }

  template <typename... Args>
  reference emplace_back(Args&&... args) SBOVECTOR_THROW_ALLOC {
    return impl_.emplace_back(std::forward<Args>(args)...);
  }

  void pop_back() noexcept { erase(begin() + size() - 1); }

  void resize(size_t count) SBOVECTOR_THROW_ALLOC {
    static_assert(std::is_nothrow_default_constructible_v<DataType>);
    auto old_size = size();
    if (old_size > count) {
      erase(end() - (old_size - count), end());
    } else if (old_size < count) {
      impl_.insert_unninitialized(old_size, count - old_size);
      for (auto iter = begin() + old_size, end_ = end(); iter != end_; ++iter) {
        new (iter) DataType();
      }
    }
  }

  void resize(size_t count, const DataType& v) SBOVECTOR_THROW_ALLOC {
    if (size() > count) {
      erase(end() - (size() - count), end());
    } else if (size() < count) {
      insert(end(), count - size(), v);
    }
  }

  template <
    size_t OtherSize,
    typename OtherAllocator,
    bool OtherCompact
  >
  void swap(SBOVector<DataType, OtherSize, OtherAllocator, OtherCompact>& that)
      SBOVECTOR_THROW_ALLOC {
    impl_.swap_cross(that.impl_);
  }

  template <size_t OtherSize, bool OtherCompact>
  void swap(SBOVector<DataType, OtherSize, Allocator, OtherCompact>& that)
      SBOVECTOR_THROW_ALLOC {
    impl_.swap(that.impl_);
  }

  template<typename DataType_, size_t BufferSize_, typename Allocator_, bool Compact_>
  friend class SBOVector;
};

template<typename DataType, size_t BufferSize, typename Allocator = std::allocator<DataType>>
using CompactSBOVector = SBOVector<DataType, BufferSize, Allocator, true>;

#endif  // SBOVECTOR_HPP