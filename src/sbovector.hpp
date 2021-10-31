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
struct is_iterator<T,
                   typename std::enable_if<!std::is_same<
                       typename std::iterator_traits<T>::value_type,
                       void>::value>::type> {
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template <typename DataType,
          size_t BufferSize,
          typename Allocator,
          bool = is_compactable_v<Allocator>>
struct SBOVectorBase : private Allocator {
  using AlignedStorage =
      std::aligned_storage_t<sizeof(DataType), alignof(DataType)>;
  size_t count_;
  union {
    std::array<AlignedStorage, BufferSize> inline_;
    struct {
      DataType* data_;
      size_t capacity_;
    } external_;
  };

  SBOVectorBase() : Allocator(), count_(0), inline_() {}
  SBOVectorBase(const Allocator& alloc)
      : Allocator(alloc), count_(0), inline_() {}

  ~SBOVectorBase() { /*intentionally empty*/
  }

  Allocator& access_allocator() { return *this; }
  Allocator get_allocator() const { return *this; }
};

template <typename DataType, size_t BufferSize, typename Allocator>
struct SBOVectorBase<DataType, BufferSize, Allocator, false> {
  using AlignedStorage =
      std::aligned_storage_t<sizeof(DataType), alignof(DataType)>;
  Allocator alloc_;
  size_t count_;
  union {
    std::array<AlignedStorage, BufferSize> inline_;
    struct {
      DataType* data_;
      size_t capacity_;
    } external_;
  };

  SBOVectorBase() : alloc_(), count_(0), inline_() {}
  SBOVectorBase(const Allocator& alloc) : alloc_(alloc), count_(0), inline_() {}

  ~SBOVectorBase() { /*intentionally empty*/
  }

  Allocator& access_allocator() { return alloc_; }
  Allocator get_allocator() const { return alloc_; }
};

template <typename DataType, size_t BufferSize, typename Allocator>
struct VectorImpl final
    : public SBOVectorBase<DataType, BufferSize, Allocator> {
  using BaseType = SBOVectorBase<DataType, BufferSize, Allocator>;

  using BaseType::access_allocator;
  using BaseType::count_;
  using BaseType::external_;
  using BaseType::get_allocator;
  using BaseType::inline_;

  VectorImpl() : BaseType() {}
  VectorImpl(const Allocator& alloc) : BaseType(alloc) {}

  ~VectorImpl() { clear(); }

  DataType* begin() {
    if (count_ <= BufferSize) {
      return inline_as_datatype();
    }
    return external_.data_;
  }

  const DataType* begin() const {
    if (count_ <= BufferSize) {
      return reinterpret_cast<const DataType*>(inline_.data());
    }
    return external_.data_;
  }

  DataType* end() { return begin() + count_; }
  const DataType* end() const { return begin() + count_; }

  void clear() noexcept {
    std::destroy(begin(), end());
    if (count_ > BufferSize) {
      get_allocator().deallocate(external_.data_, external_.capacity_);
      new (&inline_) decltype(inline_)();
    }
    count_ = 0;
  }

  size_t capacity() const noexcept {
    if (count_ <= BufferSize)
      return BufferSize;
    return external_.capacity_;
  }

 private:
  void insert_unninitialized_in_cap(size_t pos, size_t insert_count) noexcept {
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);
    static_assert(relax_except ||
                  std::is_nothrow_move_constructible_v<DataType>);

    auto new_size = count_ + insert_count;
    auto uninit_count = std::min(count_ - pos, insert_count);
    auto assign_count = count_ - (pos + uninit_count);
    std::uninitialized_move_n(end() - uninit_count, uninit_count,
                              begin() + new_size - uninit_count);

    std::move_backward(begin() + pos, begin() + pos + assign_count, end());

    std::destroy_n(begin() + pos, count_ - (pos + assign_count));
    count_ += insert_count;
  }

  void insert_unninitialized_with_growth(size_t pos, size_t insert_count)
      SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except ||
                  std::is_nothrow_move_constructible_v<DataType>);
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    const size_t new_size = count_ + insert_count;
    const size_t new_cap = std::max(new_size, SuggestGrowth(count_));

    auto new_buffer = get_allocator().allocate(new_cap);

    if (!new_buffer) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }

    std::uninitialized_move_n(begin(), pos, new_buffer);
    std::uninitialized_move_n(begin() + pos, count_ - pos,
                              new_buffer + pos + insert_count);

    clear();

    std::destroy_at(&inline_);
    count_ = new_size;
    external_.data_ = new_buffer;
    external_.capacity_ = new_cap;
  }

 public:
  // Create Uninitialized Space x insert_count at begin() + pos
  // eg: if X is a value and U uninitialized space
  // { X, X, X, X }.insert_uniinitialized(2,3) -> { X, X, U, U, U, X, X }
  void insert_unninitialized(size_t pos,
                             size_t insert_count) SBOVECTOR_THROW_ALLOC {
    if (count_ + insert_count <= capacity()) {
      insert_unninitialized_in_cap(pos, insert_count);
    } else {
      insert_unninitialized_with_growth(pos, insert_count);
    }
  }

  // bring external_ -> inline_
  // assumes count_ <= BufferSize
  // assumes external_ is in use
  void internalize() noexcept {
    static_assert(!relax_except ||
                  std::is_nothrow_move_constructible_v<DataType>);

    auto external_ptr_copy = external_.data_;
    auto external_capacity_copy = external_.capacity_;

    new (&inline_) decltype(inline_)();

    std::uninitialized_move_n(external_ptr_copy, count_, inline_as_datatype());

    std::destroy_n(external_ptr_copy, count_);
    get_allocator().deallocate(external_ptr_copy, external_capacity_copy);
  }

 private:
  template <size_t Size1,
            typename Allocator1,
            size_t Size2,
            typename Allocator2>
  inline static void no_alloc_swap(
      VectorImpl<DataType, Size1, Allocator1>& A,
      VectorImpl<DataType, Size2, Allocator2>& B) noexcept {
    static_assert(relax_except ||
                  std::is_nothrow_move_constructible_v<DataType>);

    auto small_size = std::min(A.count_, B.count_);
    auto size_diff = std::max(A.count_, B.count_) - small_size;

    DataType* source = A.begin() + small_size;
    DataType* sink = B.begin() + small_size;

    std::swap_ranges(A.begin(), source, B.begin());

    if (A.count_ == small_size) {
      std::swap(source, sink);
    }

    std::uninitialized_move_n(source, size_diff, sink);
    std::destroy_n(source, size_diff);
    std::swap(A.count_, B.count_);
  }

  template <size_t Size1,
            typename Allocator1,
            size_t Size2,
            typename Allocator2>
  inline static void one_alloc_swap(
      VectorImpl<DataType, Size1, Allocator1>& allocating,
      VectorImpl<DataType, Size2, Allocator2>& remaining)
      SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except ||
                  std::is_nothrow_move_constructible_v<DataType>);

    auto new_data_size = remaining.count_;
    auto new_data = allocating.access_allocator().allocate(new_data_size);

    if (!new_data) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }
    std::uninitialized_move_n(remaining.begin(), new_data_size, new_data);
    std::destroy(remaining.begin(), remaining.end());
    std::uninitialized_move_n(allocating.begin(), allocating.count_,
                              remaining.begin());

    if (remaining.count_ > Size2 && allocating.count_ <= Size2) {
      remaining.count_ = allocating.count_;
      remaining.internalize();
    } else {
      remaining.count_ = allocating.count_;
    }

    allocating.clear();
    std::destroy_at(&allocating.inline_);
    allocating.count_ = allocating.external_.capacity_ = new_data_size;
    allocating.external_.data_ = new_data;
  }

 public:
  template <size_t OtherSize, typename OtherAllocator>
  inline void swap_cross(VectorImpl<DataType, OtherSize, OtherAllocator>& that)
      SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    const auto this_is_sufficient = (capacity() >= that.count_);
    const auto that_is_sufficient = (that.capacity() >= count_);

    if (this_is_sufficient && that_is_sufficient) {
      no_alloc_swap(*this, that);
      if (count_ <= BufferSize && that.count_ > BufferSize)
        internalize();
      if (that.count_ <= OtherSize && count_ > OtherSize)
        that.internalize();
    } else if (this_is_sufficient) {
      one_alloc_swap(that, *this);
    } else if (that_is_sufficient) {
      one_alloc_swap(*this, that);
    } else {
      auto new_this_size = that.count_;
      auto new_this = access_allocator().allocate(that.count_);

      if (!new_this) {
        SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
        SBOVECTOR_DO_BAD_ALLOC_THROW;
      }

      auto new_that_size = count_;
      auto new_that = that.access_allocator().allocate(count_);

      if (!new_that) {
        SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
        access_allocator().deallocate(new_that, new_that_size);
        SBOVECTOR_DO_BAD_ALLOC_THROW;
      }

      std::uninitialized_move_n(that.begin(), that.count_, new_this);
      std::uninitialized_move_n(begin(), count_, new_that);

      clear();
      that.clear();

      std::destroy_at(&inline_);
      std::destroy_at(&that.inline_);
      that.external_.capacity_ = that.count_ = new_that_size;
      that.external_.data_ = new_that;
      external_.capacity_ = count_ = new_this_size;
      external_.data_ = new_this;
      // No internalizations are possible here as capacity has a lower bound
      // and this and that are both insufficiant
    }
  }

  template <size_t OtherSize>
  void swap(VectorImpl<DataType, OtherSize, Allocator>& that)
      SBOVECTOR_THROW_ALLOC {
    const auto this_is_inline = (count_ <= BufferSize);
    const auto that_is_inline = (that.count_ <= OtherSize);
    const auto this_will_be_inline = (that.count_ <= BufferSize);
    const auto that_will_be_inline = (count_ <= OtherSize);
    const auto can_swap_external =
        std::allocator_traits<Allocator>::is_always_equal::value ||
        (that.access_allocator() == access_allocator());

    if (!(this_is_inline || that_is_inline) && can_swap_external) {
      std::swap(count_, that.count_);
      std::swap(external_.data_, that.external_.data_);
      std::swap(external_.capacity_, that.external_.capacity_);
      if (this_will_be_inline)
        internalize();
      if (that_will_be_inline)
        that.internalize();
    } else if (this_is_inline && that_is_inline && this_will_be_inline &&
               that_will_be_inline) {
      no_alloc_swap(*this, that);
    } else {
      swap_cross(that);
    }
  }

  inline DataType* inline_as_datatype() noexcept {
    return reinterpret_cast<DataType*>(inline_.data());
  }

  void reserve(size_t capacity) SBOVECTOR_THROW_ALLOC {
    // If calling reserve with count_ <= BufferSize
    // reserve will leave this in an invalid state
    // care must be taken to resolve this
    // (increase count_ + construct required elements)
    // immediately.
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    capacity = std::max(capacity, SuggestGrowth(count_));

    auto new_data = access_allocator().allocate(capacity);
    if (!new_data) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }

    std::uninitialized_move_n(begin(), count_, new_data);
    std::destroy(begin(), end());

    if (count_ > BufferSize) {
      access_allocator().deallocate(external_.data_, external_.capacity_);
    } else {
      std::destroy_at(&inline_);
    }

    external_.data_ = new_data;
    external_.capacity_ = capacity;
  }

  void shrink_to_fit() SBOVECTOR_THROW_ALLOC {
    // requires count_ > BufferSize
    // requires count_ < external_.capacity_
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    auto new_data = get_allocator().allocate(count_);
    if (!new_data) {
      SBOVECTOR_ASSERT(!SBOVECTOR_THROW_BAD_ALLOC, SBOVEC_OOM);
      SBOVECTOR_DO_BAD_ALLOC_THROW;
    }

    std::uninitialized_move_n(begin(), count_, new_data);
    std::destroy(begin(), end());
    get_allocator().deallocate(external_.data_, external_.capacity_);

    external_.data_ = new_data;
    external_.capacity_ = count_;
  }

  DataType* erase(const DataType* pos, size_t count) noexcept {
    static_assert(relax_except || std::is_nothrow_move_assignable_v<DataType>);

    const auto must_internalize =
        (count_ > BufferSize) && (count_ - count) <= BufferSize;
    auto i_pos = std::distance(const_cast<const DataType*>(begin()), pos);
    std::move(begin() + i_pos + count, end(), begin() + i_pos);
    std::destroy_n(begin() + (count_ - count), count);
    count_ -= count;
    if (must_internalize)
      internalize();
    return begin() + i_pos;
  }

  template <typename... Args>
  DataType& emplace_back(Args&&... args) SBOVECTOR_THROW_ALLOC {
    static_assert(relax_except ||
                  std::is_nothrow_constructible_v<DataType, Args...>);
    if (count_ == capacity())
      reserve(count_ + 1);
    ++count_;
    DataType* out = begin() + count_ - 1;
    new (out) DataType(std::forward<Args>(args)...);
    return *out;
  }
};

}  // namespace details_

template <typename DataType,
          size_t BufferSize,
          typename Allocator = std::allocator<DataType>>
class SBOVector {
  static_assert(std::is_move_assignable_v<DataType>);
  static_assert(BufferSize > 0);
  static_assert(
      std::is_convertible_v<typename std::allocator_traits<Allocator>::pointer,
                            DataType*>);
  static_assert(std::is_convertible_v<
                typename std::allocator_traits<Allocator>::const_pointer,
                const DataType*>);

 public:
  details_::VectorImpl<DataType, BufferSize, Allocator> impl_;

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

  SBOVector(size_t count,
            const DataType& value,
            const Allocator& alloc = Allocator()) SBOVECTOR_THROW_ALLOC
      : impl_(alloc) {
    resize(count, value);
  }

  explicit SBOVector(size_t count,
                     const Allocator& alloc = Allocator()) SBOVECTOR_THROW_ALLOC
      : SBOVector(alloc) {
    resize(count);
  }

  template <typename InputIter,
            typename = std::enable_if_t<details_::is_iterator_v<InputIter>>>
  SBOVector(InputIter p_begin,
            InputIter p_end,
            const Allocator& alloc = Allocator()) SBOVECTOR_THROW_ALLOC
      : impl_(alloc) {
    insert(begin(), p_begin, p_end);
  }

  SBOVector(std::initializer_list<DataType> init_list,
            const Allocator& alloc = Allocator()) SBOVECTOR_THROW_ALLOC
      : SBOVector(init_list.begin(), init_list.end(), alloc) {}

  SBOVector(const SBOVector& copy) SBOVECTOR_THROW_ALLOC
      : SBOVector(copy.begin(), copy.end(), copy.get_allocator()) {}

  template <size_t OtherSize>
  SBOVector(const SBOVector<DataType, OtherSize, Allocator>& copy)
      SBOVECTOR_THROW_ALLOC
      : SBOVector(copy.begin(), copy.end(), copy.get_allocator()) {}

  template <size_t OtherSize, typename AllocatorType>
  SBOVector(const SBOVector<DataType, OtherSize, AllocatorType>& copy,
            const Allocator& alloc = Allocator()) SBOVECTOR_THROW_ALLOC
      : SBOVector(copy.begin(), copy.end(), alloc) {}

  SBOVector(SBOVector&& move_from) SBOVECTOR_THROW_ALLOC
      : SBOVector(move_from.get_allocator()) {
    swap(move_from);
  }

  template <size_t OtherSize>
  SBOVector(SBOVector<DataType, OtherSize, Allocator>&& move_from) noexcept
      : SBOVector(move_from.get_allocator()) {
    swap(move_from);
  }

  template <size_t OtherSize, typename AllocatorType>
  SBOVector(SBOVector<DataType, OtherSize, AllocatorType>&& move_from,
            const Allocator& alloc = Allocator()) SBOVECTOR_THROW_ALLOC
      : SBOVector(alloc) {
    swap(move_from);
  }

  ~SBOVector() { clear(); }

  SBOVector& operator=(const SBOVector& other) SBOVECTOR_THROW_ALLOC {
    assign(other.begin(), other.end());
    return *this;
  }

  template <size_t OtherSize, typename AllocatorType>
  SBOVector& operator=(const SBOVector<DataType, OtherSize, AllocatorType>&
                           other) SBOVECTOR_THROW_ALLOC {
    assign(other.begin(), other.end());
    return *this;
  }

  SBOVector& operator=(SBOVector&& that) noexcept {
    swap(that);
    return *this;
  }

  template <size_t OtherSize, typename AllocatorType>
  SBOVector& operator=(SBOVector<DataType, OtherSize, AllocatorType>&& that)
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

  template <typename InputIt,
            typename = std::enable_if_t<details_::is_iterator_v<InputIt>>>
  void assign(InputIt p_begin, InputIt p_end) SBOVECTOR_THROW_ALLOC {
    auto new_size = (size_t)std::distance(p_begin, p_end);
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
  [[nodiscard]] reverse_iterator rend() noexcept { return rbegin() + size(); }
  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return rbegin() + size();
  }
  [[nodiscard]] const_reverse_iterator crend() const noexcept {
    return crbegin() + size();
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
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, 1);
    auto out = begin() + i_pos;
    new (out) DataType(std::move(mv));
    return out;
  }

  iterator insert(const_iterator pos,
                  size_t count,
                  const DataType& v) SBOVECTOR_THROW_ALLOC {
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, count);
    std::uninitialized_fill_n(begin() + i_pos, count, v);
    return begin() + i_pos;
  }

  template <typename InputIt,
            typename = std::enable_if_t<details_::is_iterator_v<InputIt>>>
  iterator insert(const_iterator pos,
                  InputIt p_begin,
                  InputIt p_end) SBOVECTOR_THROW_ALLOC {
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, std::distance(p_begin, p_end));
    for (auto iter = begin() + i_pos; p_begin != p_end; ++p_begin, ++iter) {
      new (iter) DataType(*p_begin);
    }
    return begin() + i_pos;
  }

  iterator insert(const_iterator pos,
                  std::initializer_list<DataType> list) SBOVECTOR_THROW_ALLOC {
    return insert(pos, list.begin(), list.end());
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) SBOVECTOR_THROW_ALLOC {
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, 1);
    auto out = begin() + i_pos;
    new (out) DataType(std::forward<Args>(args)...);
    return out;
  }

  iterator erase(const_iterator pos) noexcept { return impl_.erase(pos, 1); }

  iterator erase(const_iterator p_begin, const_iterator p_end) noexcept {
    return impl_.erase(p_begin, std::distance(p_begin, p_end));
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
      typename = std::enable_if_t<!std::is_same_v<Allocator, OtherAllocator>>>
  void swap(SBOVector<DataType, OtherSize, OtherAllocator>& that)
      SBOVECTOR_THROW_ALLOC {
    impl_.swap_cross(that.impl_);
  }

  template <size_t OtherSize>
  void swap(SBOVector<DataType, OtherSize, Allocator>& that)
      SBOVECTOR_THROW_ALLOC {
    impl_.swap(that.impl_);
  }
};

#endif  // SBOVECTOR_HPP