#ifndef SBOVECTOR_HPP
#define SBOVECTOR_HPP

#include <array>
#include <algorithm>
#include <memory>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace details_ {

template<typename T>
struct is_compactable {
  static constexpr bool value = (std::is_empty_v<T> && !std::is_final_v<T>);
};

template<typename T>
constexpr bool is_compactable_v = is_compactable<T>::value;

template<typename T, typename = void>
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

template<typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template<typename DataType, size_t BufferSize, typename Allocator, bool = is_compactable_v<Allocator>>
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

  ~SBOVectorBase() { /*intentionally empty*/ }

  Allocator get_allocator() const { return *this; }
};

template<typename DataType, size_t BufferSize, typename Allocator>
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
  SBOVectorBase(const Allocator& alloc)
      : alloc_(alloc), count_(0), inline_() {}

  ~SBOVectorBase() { /*intentionally empty*/ }


  Allocator get_allocator() const { return alloc_; }
};

template<typename DataType, size_t BufferSize, typename Allocator>
struct VectorImpl final : public SBOVectorBase<DataType, BufferSize, Allocator> {
  using BaseType = SBOVectorBase<DataType, BufferSize, Allocator>;
  VectorImpl() : BaseType() {}
  VectorImpl(const Allocator& alloc) : BaseType(alloc) {}

  void clean_assign(Allocator& alloc, size_t count, const DataType& value) {
    // assumes clean [count_ == 0]
    count_ = count;
    DataType* init_ptr = reinterpret_cast<DataType*>(inline_.data());
    if (count_ > BufferSize) {
      external_.data_ = alloc.allocate(count);
      external_.capacity_ = count_;
      init_ptr = external_.data_;
    }
    while (count--)
      new (init_ptr++) DataType(value);
  }

  ~VectorImpl() { 
    clear();
  }

  DataType* begin() {
    return (count_ <= BufferSize ? reinterpret_cast<DataType*>(inline_.data())
                                 : external_.data_);
  }

  const DataType* begin() const {
    return (count_ <= BufferSize ? reinterpret_cast<const DataType*>(inline_.data())
                                 : external_.data_);
  }

  DataType* end() { return begin() + count_; }
  const DataType* end() const { return begin() + count_; }

  void clear() {
    std::destroy(begin(), end());
    if (count_ > BufferSize) {
      get_allocator().deallocate(external_.data_, external_.capacity_);
    }
    count_ = 0;
  }

  template <size_t SmallerSize, typename OA>
  void inline_swap(VectorImpl<DataType, SmallerSize, OA> &smaller) {
    // Both Inline
    auto& large_impl = *this;
    auto& small_impl = smaller;
    for (auto i = 0u; i < small_impl.count_; ++i) {
      if constexpr (std::is_swappable_v<DataType>) {
        std::swap<DataType>(*reinterpret_cast<DataType*>(large_impl.inline_.data() + i), *reinterpret_cast<DataType*>(small_impl.inline_.data() + i));
      } else if constexpr (std::is_move_constructible_v<DataType> &&
                           std::is_move_assignable_v<DataType>) {
        auto p_large_at_i =
            reinterpret_cast<DataType*>(large_impl.inline_.data() + i);
        auto p_small_at_i =
            reinterpret_cast<DataType*>(small_impl.inline_.data() + i);
        DataType temp(std::move(*p_large_at_i));
        *p_large_at_i = std::move(*p_small_at_i);
        *p_small_at_i = std::move(temp);
      } else if constexpr (std::is_copy_constructible_v<DataType> &&
                           std::is_copy_assignable_v<DataType>) {
        auto p_large_at_i =
            reinterpret_cast<DataType*>(large_impl.inline_.data() + i);
        auto p_small_at_i =
            reinterpret_cast<DataType*>(small_impl.inline_.data() + i);
        DataType temp(*p_large_at_i);
        *p_large_at_i = *p_small_at_i;
        *p_small_at_i = temp;
      } else {
        static_assert(std::is_copy_constructible_v<DataType> &&
                          std::is_copy_assignable_v<DataType>,
                      "DataType is unswappable!");
      }
    }
    {
      auto start = small_impl.count_;
      auto count = large_impl.count_ - small_impl.count_;
      if constexpr (std::is_move_constructible_v<DataType>) {
        std::uninitialized_move_n(&large_impl.inline_[start], count,
                                  &small_impl.inline_[start]);
      } else {
        std::uninitialized_copy_n(&large_impl.inline_[start], count,
                                  &small_impl.inline_[start]);
      }
    }
    std::swap(count_, smaller.count_);
  }
};

} // namespace details_

template<typename DataType, size_t BufferSize, typename Allocator = std::allocator<DataType>>
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

   explicit SBOVector(const Allocator& alloc) noexcept
       : impl_(alloc) {}

   SBOVector(size_t count, const DataType& value, const Allocator& alloc = Allocator())
       : impl_(alloc) {
     impl_.clean_assign(get_allocator(), count, value);
   }

   explicit SBOVector(size_t count, const Allocator& alloc = Allocator()) : SBOVector(count, DataType(), alloc) {}

   template <typename InputIter, typename = std::enable_if_t<details_::is_iterator_v<InputIter>>>
   SBOVector(InputIter begin, InputIter end, const Allocator& alloc = Allocator()) : impl_(alloc) {
     auto count = (size_t)std::distance(begin, end);
     impl_.count_ = count;
     auto data_ptr = reinterpret_cast<pointer>(impl_.inline_.data());
     if (count > BufferSize) {
       data_ptr = impl_.external_.data_ = get_allocator().allocate(count);
       impl_.external_.capacity_ = count;
     }
     if constexpr (!std::is_trivial_v<DataType> && std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin, end, data_ptr);
     } else {
       std::uninitialized_copy(begin, end, data_ptr);
     }
   }

   template<int OtherSize, typename AllocatorType>
   SBOVector(const SBOVector<DataType, OtherSize, AllocatorType>& copy,
             const Allocator& alloc = Allocator()) : SBOVector((size_t)0, alloc) {
     decltype(copy) temp(copy);
     this->swap(temp);
   }

   template<int OtherSize, typename AllocatorType>
   SBOVector(SBOVector<DataType, OtherSize, AllocatorType>&& move_from,
             const Allocator& alloc = Allocator()) noexcept : SBOVector((size_t)0, alloc) {
     this->swap(move_from);
   }

   SBOVector(std::initializer_list<DataType> init_list, const Allocator& alloc = Allocator()) 
     : SBOVector(init_list.begin(), init_list.end(), alloc) {}

   ~SBOVector() {
     clear();
   }

   template<int OtherSize, typename AllocatorType>
   SBOVector& operator=(
       const SBOVector<DataType, OtherSize, AllocatorType>& other) {
     decltype(other) temp(other);
     this->swap(temp);
     return *this;
   }
   template<int OtherSize, typename AllocatorType>
   SBOVector& operator=(SBOVector<DataType, OtherSize, AllocatorType>&& that) {
     this->swap(that);
     return *this;
   }

   SBOVector& operator=(std::initializer_list<DataType> init) {
     SBOVector temp(list.begin(), list.end(), get_allocator());
     swap(temp);
     return *this;
   }
   
   void assign(size_t count, const DataType& value) {
     SBOVector temp(count, value, get_allocator());
     swap(temp);
   }

   template<typename InputIt>
   void assign(InputIt begin, InputIt end) {
     SBOVector temp(begin, end, get_allocator());
     swap(temp);
   }

   void assign(std::initializer_list<DataType> list) {
     assign(list.begin(), list.end());
   }

   Allocator get_allocator() const noexcept { return impl_.get_allocator(); }

   reference at(size_t index) { return *(begin() + index); }
   const_reference at(size_t) const { return *(cbegin() + index); }

   reference operator[](size_t index) { return at(index); }
   const_reference operator[](size_t index) const { return at(index); }

   reference front() { return at(0); }
   const_reference front() const { return at(0); }

   reference back() { return at(size() - 1); }
   const_reference back() const { return at(size() - 1); }

   pointer data() noexcept {
     return impl_.begin();
   }
   const_pointer data() const noexcept { 
     return impl_.begin();
   }
   const_pointer cdata() const noexcept { return data(); }

   iterator begin() noexcept {
     return data();
   }
   const_iterator begin() const noexcept { 
     return data();
   }

   const_iterator cbegin() const noexcept { 
     return begin();
   }

   iterator end() noexcept { return begin() + size(); }
   const_iterator end() const noexcept { return cbegin() + size(); }
   const_iterator cend() const noexcept { return end(); }

   bool empty() const noexcept { return 0 == size(); }
   size_t size() const noexcept { return impl_.count_; }
   size_t max_size() const noexcept { return 0; }
   void reserve_if_external(size_t requested_capacity) {
     if (requested_capacity <= capacity() || size() <= BufferSize)
       return;
     DataType* new_data = get_allocator().allocate(requested_capacity);
     // TODO if using exceptions, throw bad_alloc on nullptr
     if constexpr (std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin(), end(), new_data);
     } else {
       std::uninitialized_copy(begin(), end(), new_data);
     }
     std::destroy(begin(), end());
     get_allocator().deallocate(impl_.external_.data_,
                              impl_.external_.capacity_);
     impl_.external_.data_ = new_data;
     impl_.external_.capacity_ = requested_capacity;
   }

   size_t capacity() const noexcept {
     if (size() <= BufferSize) {
       return BufferSize;
     }
     return impl_.external_.capacity_;
   }

   void shrink_to_fit_if_external() {
     if (size() <= BufferSize || size() == capacity())
       return;
     DataType* new_data = get_allocator().allocate(size());
     // TODO if using exceptions, throw bad_alloc on nullptr
     if constexpr (std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin(), end(), new_data);
     } else {
       std::uninitialized_copy(begin(), end(), new_data);
     }
     std::destroy(begin(), end());
     get_allocator().deallocate(impl_.external_.data_, impl_.external_.capacity_);
     impl_.external_.data_ = new_data;
     impl_.external_.capacity_ = impl_.count_;
   }

   void clear() noexcept {
     std::destroy(begin(), end());
     if (impl_.count_ > BufferSize) {
       get_allocator().deallocate(impl_.external_.data_,
                                impl_.external_.capacity_);
     }
     impl_.count_ = 0;
   }

   iterator insert(const_iterator, const DataType&) { return begin(); }
   iterator insert(const_iterator, DataType&&) { return begin(); }
   iterator insert(const_iterator, size_t, const DataType&) { return begin(); }

   template<typename InputIt>
   iterator insert(const_iterator pos, InputIt p_begin, InputIt p_end) {
     size_t out_pos = std::distance(cbegin(), pos);
     auto shift_count = std::distance(pos, cend());
     auto insert_count = std::distance(p_begin, p_end);
     if (size() + insert_count <= capacity()) {
       auto uninit_count = std::min(shift_count, insert_count);
       auto assign_count = shift_count - uninit_count;
       auto old_end = end();
       if constexpr (std::is_move_constructible_v<DataType>) {
         std::uninitialized_move_n(old_end - uninit_count, uninit_count, old_end);
         std::move_backward(pos, pos + assign_count, old_end);
         std::move(p_begin, p_end, begin() + out_pos);
       } else {
         std::uninitialized_copy_n(old_end - uninit_count, uninit_count,
                                   old_end);
         std::copy_backward(pos, pos + assign_count, old_end);
         std::copy(p_begin, p_end, begin() + out_pos);
       }
       impl_.count_ += insert_count;
     } else {
       size_t new_size = size() + insert_count;
       DataType* new_buffer = get_allocator().allocate(new_size);
       auto preceeding = std::distance(cbegin(), pos);
       // TODO: throw bad_alloc on nullptr if using exceptions
       if constexpr (std::is_move_constructible_v<DataType>) {
         std::uninitialized_move_n(begin(), preceeding, new_buffer);
         std::uninitialized_move_n(p_begin, insert_count,
                                   new_buffer + preceeding);
         std::uninitialized_move_n(pos, size() - preceeding,
                                   new_buffer + preceeding + insert_count);
       } else {
         std::uninitialized_copy_n(begin(), preceeding, new_buffer);
         std::uninitialized_copy_n(p_begin, insert_count,
                                   new_buffer + preceeding);
         std::uninitialized_copy_n(pos, size() - preceeding,
                                   new_buffer + preceeding + insert_count);
       }
       clear();
       impl_.count_ = new_size;
       impl_.external_.data_ = new_buffer;
       impl_.external_.capacity_ = new_size;
     }
     return begin() + out_pos;
   }

   iterator insert(const_iterator pos, std::initializer_list<DataType> list) {
     return insert(pos, list.begin(), list.end());
   }
   template <typename... Args>
   iterator emplace(const_iterator pos, Args&&... args) {
     return begin();
   }

   iterator erase(const_iterator) { return begin(); }
   iterator erase(const_iterator, const_iterator) { return begin(); }

   void push_back(const DataType& value) {
     if (size() == capacity())
       grow();
     ++impl_.count_;
     new (&back()) DataType(value);
   }

   void push_back(DataType&& value) {
     if (size() == capacity())
       grow();
     ++impl_.count_;
     new (&back()) DataType(value);
   }

   template <typename... Args>
   reference emplace_back(Args&&... args) {
     if (size() == capacity())
       grow();
     ++impl_.count_;
     new (&back()) DataType(std::forward<Args>(args)...);
     return back();
   }

   void pop_back() {
     std::destroy_at<DataType>(&(back()));
     --impl_.count_;
     if (impl_.count_ == BufferSize) {
       auto external_ptr_copy = impl_.external_.data_;
       auto external_capacity_copy = impl_.external_.capacity_;
       new (&impl_.inline_) decltype(impl_.inline_)();  // std::array is trivial
                                                      // constructable so this
                                                      // will optimze out
       if constexpr (std::is_move_constructible_v<DataType>) {
         std::uninitialized_move_n(external_ptr_copy, BufferSize, reinterpret_cast<pointer>(impl_.inline_.data()));
       } else {
         std::uninitialized_copy_n(external_ptr_copy, BufferSize,
                                   reinterpret_cast<pointer>(impl_.inline_.data()));
       }
       std::destroy_n(external_ptr_copy, impl_.count_);
       get_allocator().deallocate(external_ptr_copy, external_capacity_copy);
     }
   }

   void resize(size_t) {}
   void resize(size_t, const DataType&) {}

   template<int OtherSize, typename OtherAllocator, typename = std::enable_if_t<!std::is_same_v<Allocator, OtherAllocator>>>
   void swap(SBOVector<DataType, OtherSize, OtherAllocator>& that) {
     // TODO: Optimize Considerably
     SBOVector<DataType, OtherSize, OtherAllocator> copy_that(that);
     that.clear();
     that.insert(that.begin(), begin(), end());
     clear();
     insert(begin(), copy_that.begin(), copy_that.end());
   }

   template<size_t OtherSize, typename = std::enable_if_t<OtherSize != BufferSize>>
   void swap(SBOVector<DataType, OtherSize, Allocator>& that) {
     auto& this_impl = impl_;
     auto& that_impl = that.impl_;
     auto largest_buffer = std::max(BufferSize, OtherSize);
     auto smallest_buffer = std::min(BufferSize, OtherSize);
     if (this_impl.count_ > largest_buffer &&
         that_impl.count_ > largest_buffer) {
       swap_external_buffers(that);
     } else if (this_impl.count_ <= smallest_buffer &&
                that_impl.count_ <= smallest_buffer) {
       swap_inline_buffers(that);
     } else {
       // TODO this can be optimized considerably
       SBOVector copy_that(that.begin(), that.end(), get_allocator());
       that.clear();
       that.insert(that.begin(), begin(), end());
       clear();
       insert(begin(), copy_that.begin(), copy_that.end());
     }
   }

   void swap(SBOVector& that) {
     auto& this_impl = impl_;
     auto& that_impl = that.impl_;
     if (this_impl.count_ > BufferSize && that_impl.count_ > BufferSize) {
       swap_external_buffers(that);
     } else if (this_impl.count_ <= BufferSize && that_impl.count_ <= BufferSize) {
       swap_inline_buffers(that);
     } else {
       // Cross Inline/External
       auto& large_impl =
           (this_impl.count_ > that_impl.count_ ? this_impl : that_impl);
       auto& small_impl =
           (this_impl.count_ > that_impl.count_ ? that_impl : this_impl);
       auto large_ptr = large_impl.external_.data_;
       auto large_cap = large_impl.external_.capacity_;
       if constexpr (std::is_move_constructible_v<DataType>) {
         std::uninitialized_move_n(small_impl.inline_.data(), small_impl.count_,
                                   large_impl.inline_.data());
       } else {
         std::uninitialized_copy_n(small_impl.inline_.data(), small_impl.count_,
                                   large_impl.inline_.data());
       }
       small_impl.external_.data_ = large_ptr;
       small_impl.external_.capacity_ = large_cap;
       std::swap(large_impl.count_, small_impl.count_);
     }
   }

  private:
   void grow() {
     // Note: if calling grown with count_ <= BufferSize, you must immediately insert sufficient elements
     // as grow will change to an external_buffer, and count_ <= BufferSize implies internal_buffer is being used
     size_t requested = size() * 2;
     DataType* new_data = get_allocator().allocate(requested);
     // TODO if using exceptions, throw bad_alloc on nullptr
     if constexpr (std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin(), end(), new_data);
     } else {
       std::uninitialized_copy(begin(), end(), new_data);
     }
     std::destroy(begin(), end());
     if (size() > BufferSize) {
       get_allocator().deallocate(impl_.external_.data_,
                                impl_.external_.capacity_);
     }
     impl_.external_.data_ = new_data;
     impl_.external_.capacity_ = requested;
   }

   template<typename Other>
   inline void swap_inline_buffers(Other& that) {
     auto& this_impl = impl_;
     auto& that_impl = that.impl_;
     if (this_impl.count_ > that_impl.count_) {
       this_impl.inline_swap(that_impl);
     } else {
       that_impl.inline_swap(this_impl);
     }
   }

   template<size_t OtherSize>
   inline void swap_external_buffers(
       SBOVector<DataType, OtherSize, Allocator>& that) {
     auto& this_impl = this->impl_;
     auto& that_impl = that.impl_;
     std::swap(this_impl.count_, that_impl.count_);
     std::swap(this_impl.external_.data_, that_impl.external_.data_);
     std::swap(this_impl.external_.capacity_, that_impl.external_.capacity_);
   }

   template<typename OtherData, size_t OtherSize, typename OtherAllocator>
   friend class SBOVector;
};

#endif // SBOVECTOR_HPP