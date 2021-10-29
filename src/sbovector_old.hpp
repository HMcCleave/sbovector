// Reference copy of SBOVector_h
// used to provide comparative benchmarking

#ifndef SBOVECTOR_OLD_HPP
#define SBOVECTOR_OLD_HPP

#include <array>
#include <algorithm>
#include <memory>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace old {

namespace details_ {

constexpr size_t kSBOVectorGrowthFactor = 2;

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
  size_t count_;
  union {
    std::array<std::aligned_storage_t<sizeof(DataType), alignof(DataType)>,
               BufferSize>
        inline_;
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
  Allocator alloc_;
  size_t count_;
  union {
    std::array<std::aligned_storage_t<sizeof(DataType), alignof(DataType)>,
               BufferSize>
        inline_;
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
struct VectorImpl : public SBOVectorBase<DataType, BufferSize, Allocator> {
  using BaseType = SBOVectorBase<DataType, BufferSize, Allocator>;
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

  void clear() {
    std::destroy(begin(), end());
    if (count_ > BufferSize) {
      get_allocator().deallocate(external_.data_, external_.capacity_);
      new (&inline_) decltype(inline_)();
    }
    count_ = 0;
  }

  void insert_unninitialized_in_cap(size_t pos, size_t insert_count) {
    auto new_size = count_ + insert_count;
    auto uninit_count = std::min(count_ - pos, insert_count);
    auto assign_count = count_ - (pos + uninit_count);
    std::uninitialized_move_n(end() - uninit_count, uninit_count,
                              begin() + new_size - uninit_count);
    std::move_backward(begin() + pos, begin() + pos + assign_count, end());
    std::destroy_n(begin() + pos, count_ - (pos + assign_count));
    count_ += insert_count;
  }

  void insert_unninitialized_with_growth(size_t pos, size_t insert_count) {
    size_t new_size = count_ + insert_count;
    DataType* new_buffer = get_allocator().allocate(new_size);
    // TODO: throw bad_alloc on nullptr if using exceptions
    std::uninitialized_move_n(begin(), pos, new_buffer);
    std::uninitialized_move_n(begin() + pos, count_ - pos,
                              new_buffer + pos + insert_count);
    clear();
    count_ = new_size;
    external_.data_ = new_buffer;
    external_.capacity_ = new_size;
  }

  size_t capacity() const {
    if (count_ <= BufferSize)
      return BufferSize;
    return external_.capacity_;
  }

  // Create Uninitialized Space x insert_count at begin() + pos
  // eg: if X is a value and U uninitialized space
  // { X, X, X, X }.insert_uniinitialized(2,3) -> { X, X, U, U, U, X, X }
  void insert_unninitialized(size_t pos, size_t insert_count) {
    if (count_ + insert_count <= capacity()) {
      insert_unninitialized_in_cap(pos, insert_count);
    } else {
      insert_unninitialized_with_growth(pos, insert_count);
    }
  }

  void internalize() {
    // Helper function, must be called when decreasing in size such that the
    // count is <= BufferSize but an external buffer still exists
    auto external_ptr_copy = external_.data_;
    auto external_capacity_copy = external_.capacity_;
    new (&inline_) decltype(inline_)();
    std::uninitialized_move_n(external_ptr_copy, count_, inline_as_datatype());
    std::destroy_n(external_ptr_copy, count_);
    get_allocator().deallocate(external_ptr_copy, external_capacity_copy);
  }

  template <size_t OtherSize, typename OA>
  void inline_swap(VectorImpl<DataType, OtherSize, OA>& smaller) {
    // requirement count_ >= smaller.count_
    // requirement count_ <= min(OtherSize, BufferSize)
    auto& large_impl = *this;
    auto& small_impl = smaller;
    std::swap_ranges(small_impl.begin(), small_impl.end(), large_impl.begin());
    {
      auto start = small_impl.count_;
      auto count = large_impl.count_ - small_impl.count_;
      std::uninitialized_move_n(large_impl.inline_as_datatype() + start, count,
                                small_impl.inline_as_datatype() + start);
      std::destroy_n(large_impl.inline_as_datatype() + start, count);
    }
    std::swap(count_, smaller.count_);
  }

  template <size_t OtherSize>
  void external_swap(VectorImpl<DataType, OtherSize, Allocator>& other) {
    std::swap(count_, other.count_);
    std::swap(external_.data_, other.external_.data_);
    std::swap(external_.capacity_, other.external_.capacity_);
  }

  template <size_t OtherSize, typename OtherAllocator>
  void swap_cross(VectorImpl<DataType, OtherSize, OtherAllocator>& that) {
    const auto this_will_be_inline = (that.count_ <= BufferSize);
    const auto that_will_be_inline = (count_ <= OtherSize);

    // todo still optimization available here
    auto new_this = access_allocator().allocate(that.count_);
    auto new_this_size = that.count_;
    auto new_that = that.access_allocator().allocate(count_);
    auto new_that_size = count_;
    std::uninitialized_move_n(that.begin(), that.count_, new_this);
    std::uninitialized_move_n(begin(), count_, new_that);
    clear();
    that.clear();
    std::destroy_at(&inline_);
    std::destroy_at(&that.inline_);
    that.external_.capacity_ = that.count_ = new_that_size;
    that.external_.data_ = new_that;
    if (that_will_be_inline)
      that.internalize();
    external_.capacity_ = count_ = new_this_size;
    external_.data_ = new_this;
    if (this_will_be_inline)
      internalize();
  }

  template <size_t OtherSize>
  void swap(VectorImpl<DataType, OtherSize, Allocator>& that) {
    const auto this_is_inline = (count_ <= BufferSize);
    const auto that_is_inline = (that.count_ <= OtherSize);
    const auto this_will_be_inline = (that.count_ <= BufferSize);
    const auto that_will_be_inline = (count_ <= OtherSize);

    const auto min_size = std::min(count_, that.count_);
    const auto max_size = std::max(count_, that.count_);

    if (!(this_is_inline || that_is_inline)) {
      external_swap(that);
      if (this_will_be_inline)
        internalize();
      if (that_will_be_inline)
        that.internalize();
    } else if (this_is_inline && that_is_inline && this_will_be_inline &&
               that_will_be_inline) {
      if (count_ < that.count_) {
        that.inline_swap(*this);
      } else {
        inline_swap(that);
      }
    } else {
      swap_cross(that);
    }
  }

  inline DataType* inline_as_datatype() {
    return reinterpret_cast<DataType*>(inline_.data());
  }

  void reserve(size_t capacity) {
    // If calling reserve with count_ <= BufferSize
    // reserve will leave this in an invalid state
    // care must be taken to resolve this
    // (increase count_ + construct required elements)
    // immediately.

    capacity = std::max(capacity, count_ * kSBOVectorGrowthFactor);
    DataType* new_data = access_allocator().allocate(capacity);
    // TODO if using exceptions, throw bad_alloc on nullptr
    std::uninitialized_move_n(begin(), count_, new_data);
    std::destroy(begin(), end());
    if (count_ > BufferSize) {
      access_allocator().deallocate(external_.data_, external_.capacity_);
    }
    external_.data_ = new_data;
    external_.capacity_ = capacity;
  }

  void shrink_to_fit() {
    // requires count_ > BufferSize
    // requires count_ < external_.capacity_
    DataType* new_data = get_allocator().allocate(count_);
    // TODO if using exceptions, throw bad_alloc on nullptr
    std::uninitialized_move_n(begin(), count_, new_data);
    std::destroy(begin(), end());
    get_allocator().deallocate(external_.data_, external_.capacity_);
    external_.data_ = new_data;
    external_.capacity_ = count_;
  }

  DataType* erase(const DataType* pos, size_t count) {
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
};

}  // namespace details_

template <typename DataType,
          size_t BufferSize,
          typename Allocator = std::allocator<DataType>>
class SBOVector {
 private:
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

  SBOVector() : impl_() {}

  explicit SBOVector(const Allocator& alloc) noexcept : impl_(alloc) {}

  SBOVector(size_t count,
            const DataType& value,
            const Allocator& alloc = Allocator())
      : impl_(alloc) {
    resize(count, value);
  }

  explicit SBOVector(size_t count, const Allocator& alloc = Allocator())
      : SBOVector(alloc) {
    resize(count);
  }

  template <typename InputIter,
            typename = std::enable_if_t<details_::is_iterator_v<InputIter>>>
  SBOVector(InputIter p_begin,
            InputIter p_end,
            const Allocator& alloc = Allocator())
      : impl_(alloc) {
    insert(begin(), p_begin, p_end);
  }

  SBOVector(std::initializer_list<DataType> init_list,
            const Allocator& alloc = Allocator())
      : SBOVector(init_list.begin(), init_list.end(), alloc) {}

  SBOVector(const SBOVector& copy)
      : SBOVector(copy.begin(), copy.end(), copy.get_allocator()) {}

  template <int OtherSize>
  SBOVector(const SBOVector<DataType, OtherSize, Allocator>& copy)
      : SBOVector(copy.begin(), copy.end(), copy.get_allocator()) {}

  template <int OtherSize, typename AllocatorType>
  SBOVector(const SBOVector<DataType, OtherSize, AllocatorType>& copy,
            const Allocator& alloc = Allocator())
      : SBOVector(copy.begin(), copy.end(), alloc) {}

  SBOVector(SBOVector&& move_from) : SBOVector(move_from.get_allocator()) {
    swap(move_from);
  }

  template <int OtherSize>
  SBOVector(SBOVector<DataType, OtherSize, Allocator>&& move_from)
      : SBOVector(move_from.get_allocator()) {
    swap(move_from);
  }

  template <int OtherSize, typename AllocatorType>
  SBOVector(SBOVector<DataType, OtherSize, AllocatorType>&& move_from,
            const Allocator& alloc = Allocator()) noexcept
      : SBOVector(alloc) {
    swap(move_from);
  }

  ~SBOVector() { clear(); }

  SBOVector& operator=(const SBOVector& other) {
    assign(other.begin(), other.end());
    return *this;
  }

  template <int OtherSize, typename AllocatorType>
  SBOVector& operator=(
      const SBOVector<DataType, OtherSize, AllocatorType>& other) {
    assign(other.begin(), other.end());
    return *this;
  }

  SBOVector& operator=(SBOVector&& that) {
    swap(that);
    return *this;
  }

  template <int OtherSize, typename AllocatorType>
  SBOVector& operator=(SBOVector<DataType, OtherSize, AllocatorType>&& that) {
    swap(that);
    return *this;
  }

  SBOVector& operator=(std::initializer_list<DataType> init) {
    assign(init.begin(), init.end());
    return *this;
  }

  void assign(size_t count, const DataType& value) {
    SBOVector temp(count, value, get_allocator());
    swap(temp);
  }

  template <typename InputIt>
  void assign(InputIt begin, InputIt end) {
    SBOVector temp(begin, end, get_allocator());
    swap(temp);
  }

  void assign(std::initializer_list<DataType> list) {
    assign(list.begin(), list.end());
  }

  [[nodiscard]] Allocator get_allocator() const noexcept {
    return impl_.get_allocator();
  }

  [[nodiscard]] reference at(size_t index) { return *(begin() + index); }
  [[nodiscard]] const_reference at(size_t) const { return *(cbegin() + index); }

  [[nodiscard]] reference operator[](size_t index) { return at(index); }
  [[nodiscard]] const_reference operator[](size_t index) const {
    return at(index);
  }

  [[nodiscard]] reference front() { return at(0); }
  [[nodiscard]] const_reference front() const { return at(0); }

  [[nodiscard]] reference back() { return at(size() - 1); }
  [[nodiscard]] const_reference back() const { return at(size() - 1); }

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

  void reserve_if_external(size_t requested_capacity) {
    if (requested_capacity <= capacity() || size() <= BufferSize)
      return;
    impl_.reserve(requested_capacity);
  }

  [[nodiscard]] size_t capacity() const noexcept { return impl_.capacity(); }

  void shrink_to_fit_if_external() {
    if (size() <= BufferSize || size() == capacity())
      return;
    impl_.shrink_to_fit();
  }

  void clear() noexcept { impl_.clear(); }

  iterator insert(const_iterator pos, const DataType& v) {
    return insert(pos, 1, v);
  }

  iterator insert(const_iterator pos, DataType&& mv) {
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, 1);
    auto out = begin() + i_pos;
    new (out) DataType(std::move(mv));
    return out;
  }

  iterator insert(const_iterator pos, size_t count, const DataType& v) {
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, count);
    std::uninitialized_fill_n(begin() + i_pos, count, v);
    return begin() + i_pos;
  }

  template <typename InputIt,
            typename = std::enable_if_t<details_::is_iterator_v<InputIt>>>
  iterator insert(const_iterator pos, InputIt p_begin, InputIt p_end) {
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, std::distance(p_begin, p_end));
    for (auto iter = begin() + i_pos; p_begin != p_end; ++p_begin, ++iter) {
      new (iter) DataType(*p_begin);
    }
    return begin() + i_pos;
  }

  iterator insert(const_iterator pos, std::initializer_list<DataType> list) {
    return insert(pos, list.begin(), list.end());
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    size_t i_pos = std::distance(cbegin(), pos);
    impl_.insert_unninitialized(i_pos, 1);
    auto out = begin() + i_pos;
    new (out) DataType(std::forward<Args>(args)...);
    return out;
  }

  iterator erase(const_iterator pos) { return impl_.erase(pos, 1); }

  iterator erase(const_iterator p_begin, const_iterator p_end) {
    return impl_.erase(p_begin, std::distance(p_begin, p_end));
  }

  void push_back(const DataType& value) {
    if (size() == capacity())
      impl_.reserve(size() + 1);
    ++impl_.count_;
    new (&back()) DataType(value);
  }

  void push_back(DataType&& value) {
    if (size() == capacity())
      impl_.reserve(size() + 1);
    ++impl_.count_;
    new (&back()) DataType(std::move(value));
  }

  template <typename... Args>
  reference emplace_back(Args&&... args) {
    if (size() == capacity())
      impl_.reserve(size() + 1);
    ++impl_.count_;
    new (&back()) DataType(std::forward<Args>(args)...);
    return back();
  }

  void pop_back() { erase(begin() + size() - 1); }

  void resize(size_t count) {
    auto old_size = size();
    if (old_size > count)
      erase(end() - (old_size - count), end());
    else if (old_size < count) {
      impl_.insert_unninitialized(old_size, count - old_size);
      for (auto iter = begin() + old_size; iter != end(); ++iter) {
        new (iter) DataType();
      }
    }
  }

  void resize(size_t count, const DataType& v) {
    if (size() > count)
      erase(end() - (size() - count), end());
    else if (size() < count) {
      insert(end(), count - size(), v);
    }
  }

  template <
      int OtherSize,
      typename OtherAllocator,
      typename = std::enable_if_t<!std::is_same_v<Allocator, OtherAllocator>>>
  void swap(SBOVector<DataType, OtherSize, OtherAllocator>& that) {
    impl_.swap_cross(that.impl_);
  }

  template <size_t OtherSize,
            typename = std::enable_if_t<OtherSize != BufferSize>>
  void swap(SBOVector<DataType, OtherSize, Allocator>& that) {
    impl_.swap(that.impl_);
  }

  void swap(SBOVector& that) { impl_.swap(that.impl_); }

  friend class SBOVector;
};

} // namespace old

#endif  // SBOVECTOR_OLD_HPP