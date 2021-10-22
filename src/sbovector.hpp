#ifndef SBOVECTOR_HPP
#define SBOVECTOR_HPP

#include <array>
#include <algorithm>
#include <memory>
#include <initializer_list>
#include <type_traits>

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

template<typename A, typename B, bool = is_compactable_v<A>>
struct CompactPair final : private A {
  B b_;

  template<typename A_Arg, typename... B_Args>
  CompactPair(std::true_type, A_Arg&& a_arg, B_Args&&... b_args) 
    : A(std::forward<A_Arg>(a_arg)),
      b_(std::forward<B_Args>(b_args)...) {}

  template<typename... B_Args>
  CompactPair(std::false_type, B_Args&&... b_args)
    : A(), b_(std::forward<B_Args>(b_args)...) {}

  ~CompactPair() {}


  A& first() { return *this; }
  const A& first() const { return *this; }
  B& second() { return b_; }
  const B& second() const { return b_; }
};

template<typename A, typename B>
struct CompactPair<A, B, false> final {
  A a_;
  B b_;
  template <typename A_Arg, typename... B_Args>
  CompactPair(std::true_type, A_Arg&& a_arg, B_Args&&... b_args)
      : a_(std::forward<A_Arg>(a_arg)), b_(std::forward<B_Args>(b_args)...) {}

  template <typename... B_Args>
  CompactPair(std::false_type, B_Args&&... b_args)
      : a_(), b_(std::forward<B_Args>(b_args)...) {}

  ~CompactPair() {}

  A& first() { return a_; }
  const A& first() const { return a_; }
  B& second() { return b_; }
  const B& second() const { return b_; }
};

} // namespace details_

template<typename DataType, size_t BufferSize, typename Allocator = std::allocator<DataType>>
class SBOVector {
 private:

  struct VectorImpl {
    size_t count_;
    union {
      std::array<DataType, BufferSize> inline_;
      struct {
        DataType* data_;
        size_t capacity_;
      } external_;
    };
    VectorImpl() : count_(0) {}

    void clean_assign(Allocator& alloc, size_t count, const DataType& value) {
      // assumes clean [count_ == 0]
      count_ = count;
      DataType* init_ptr = inline_.data();
      if (count_ > BufferSize) {
        external_.data_ = alloc.allocate(count);
        external_.capacity_ = count_;
        init_ptr = external_.data_;
      }
      std::uninitialized_fill_n(init_ptr, count_, value);
    }
    ~VectorImpl() { /*Destruction of external_ must be done by SBOVector class to facilitate custom allocator*/ }

    void clear(Allocator& alloc) { 
      if (count_ > BufferSize) {
        alloc.deallocate(external_.data_, external_.capacity_);
      }
      count_ = 0;
    }
  };

  details_::CompactPair<Allocator, VectorImpl> data_;

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

   SBOVector() : data_(std::false_type{}) {}

   explicit SBOVector(const Allocator& a) noexcept
       : data_(std::true_type{}, a) {}

   SBOVector(size_t count, const DataType& value, const Allocator& alloc = Allocator())
       : data_(std::true_type{}, alloc) {
     data_.second().clean_assign(data_.first(), count, value);
   }

   explicit SBOVector(size_t count, const Allocator& alloc = Allocator()) : SBOVector(count, DataType(), alloc) {}

   template <typename InputIter, typename = std::enable_if_t<details_::is_iterator_v<InputIter>>>
   SBOVector(InputIter begin, InputIter end, const Allocator& alloc = Allocator()) : data_(std::true_type{}, alloc) {
     auto count = (size_t)std::distance(begin, end);
     auto& ralloc = data_.first();
     auto& impl = data_.second();
     impl.count_ = count;
     DataType* data_ptr = impl.inline_.data();
     if (count > BufferSize) {
       data_ptr = impl.external_.data_ = ralloc.allocate(count);
       impl.external_.capacity_ = count;
     }
     if constexpr (!std::is_trivial_v<DataType> && std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin, end, data_ptr);
     } else {
       std::uninitialized_copy(begin, end, data_ptr);
     }
   }

   template<int OtherSize>
   SBOVector(const SBOVector<DataType, OtherSize, Allocator>&,
             const Allocator& = Allocator()) {} // TODO

   template<int OtherSize>
   SBOVector(SBOVector<DataType, OtherSize, Allocator>&&,
             const Allocator& = Allocator()) noexcept {} // TODO

   SBOVector(std::initializer_list<DataType> init_list, const Allocator& alloc = Allocator()) 
     : SBOVector(init_list.begin(), init_list.end(), alloc) {}

   ~SBOVector() {
     std::destroy(begin(), end());
     data_.second().clear(data_.first());
   }

   template<int OtherSize>
   SBOVector& operator=(
       const SBOVector<DataType, OtherSize, Allocator>& other) { // TODO
     return *this;
   }
   template<int OtherSize>
   SBOVector& operator=(SBOVector<DataType, OtherSize, Allocator>&&) { // TODO
     return *this;
   }

   SBOVector& operator=(std::initializer_list<DataType>) {
     return *this;
   }
   
   void assign(size_t, const DataType&) {}

   template<typename InputIt>
   void assign(InputIt, InputIt) {}

   void assign(std::initializer_list<DataType> list) {
     assign(list.begin(), list.end());
   }

   Allocator get_allocator() const noexcept { return Allocator(); }

   reference at(size_t index) { return *(begin() + index); }
   const_reference at(size_t) const { return *(cbegin() + index); }

   reference operator[](size_t index) { return at(index); }
   const_reference operator[](size_t index) const { return at(index); }

   reference front() { return at(0); }
   const_reference front() const { return at(0); }

   reference back() { return at(size() - 1); }
   const_reference back() const { return at(size() - 1); }

   pointer data() noexcept {
     auto& impl = data_.second();
     if (impl.count_ <= BufferSize) {
       return impl.inline_.data();
     }
     return impl.external_.data_;
   }
   const_pointer data() const noexcept {
     auto& impl = data_.second();
     if (impl.count_ <= BufferSize) {
       return impl.inline_.data();
     }
     return impl.external_.data_;
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
   size_t size() const noexcept { return data_.second().count_; }
   size_t max_size() const noexcept { return 0; }
   void reserve_if_external(size_t requested_capacity) {
     if (requested_capacity <= capacity() || size() <= BufferSize)
       return;
     DataType* new_data = data_.first().allocate(requested_capacity);
     // TODO if using exceptions, throw bad_alloc on nullptr
     if constexpr (std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin(), end(), new_data);
     } else {
       std::uninitialized_copy(begin(), end(), new_data);
     }
     std::destroy(begin(), end());
     data_.first().deallocate(data_.second().external_.data_,
                              data_.second().external_.capacity_);
     data_.second().external_.data_ = new_data;
     data_.second().external_.capacity_ = requested_capacity;
   }

   size_t capacity() const noexcept {
     if (size() <= BufferSize) {
       return BufferSize;
     }
     return data_.second().external_.capacity_;
   }

   void shrink_to_fit_if_external() {
     if (size() <= BufferSize || size() == capacity())
       return;
     DataType* new_data = data_.first().allocate(size());
     // TODO if using exceptions, throw bad_alloc on nullptr
     if constexpr (std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin(), end(), new_data);
     } else {
       std::uninitialized_copy(begin(), end(), new_data);
     }
     std::destroy(begin(), end());
     data_.first().deallocate(data_.second().external_.data_,
                              data_.second().external_.capacity_);
     data_.second().external_.data_ = new_data;
     data_.second().external_.capacity_ = data_.second().count_;
   }

   void clear() noexcept {
     std::destroy(begin(), end());
     if (data_.second().count_ > BufferSize) {
       data_.first().deallocate(data_.second().external_.data_,
                                data_.second().external_.capacity_);
     }
     data_.second().count_ = 0;
   }

   iterator insert(const_iterator, const DataType&) { return begin(); }
   iterator insert(const_iterator, DataType&&) { return begin(); }
   iterator insert(const_iterator, size_t, const DataType&) { return begin(); }
   template<typename InputIt>
   iterator insert(const_iterator, InputIt, InputIt) {
     return begin();
   }
   iterator insert(const_iterator, std::initializer_list<DataType>) {
     return begin();
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
     ++data_.second().count_;
     new (&back()) DataType(value);
   }

   void push_back(DataType&& value) {
     if (size() == capacity())
       grow();
     ++data_.second().count_;
     new (&back()) DataType(value);
   }

   template <typename... Args>
   reference emplace_back(Args&&... args) {
     if (size() == capacity())
       grow();
     ++data_.second().count_;
     new (&back()) DataType(std::forward<Args>(args)...);
     return back();
   }

   void pop_back() {
     auto& impl = data_.second();
     std::destroy_at<DataType>(&(back()));
     --impl.count_;
     if (impl.count_ == BufferSize) {
       auto external_ptr_copy = impl.external_.data_;
       auto external_capacity_copy = impl.external_.capacity_;
       new (&impl.inline_) decltype(impl.inline_)();  // std::array is trivial
                                                      // constructable so this
                                                      // will optimze out
       if constexpr (std::is_move_constructible_v<DataType>) {
         std::uninitialized_move_n(external_ptr_copy, BufferSize, impl.inline_.data());
       } else {
         std::uninitialized_copy_n(external_ptr_copy, BufferSize,
                                   impl.inline_.data());
       }
       std::destroy_n(external_ptr_copy, impl.count_);
       data_.first().deallocate(external_ptr_copy, external_capacity_copy);
     }
   }

   void resize(size_t) {}
   void resize(size_t, const DataType&) {}

   template<int OtherSize, typename OtherAllocator, typename = std::enable_if_t<!std::is_same_v<Allocator, OtherAllocator>>>
   void swap(SBOVector<DataType, OtherSize, OtherAllocator>& that) {

   }

   template<int OtherSize, typename = std::enable_if_t<OtherSize != BufferSize>>
   void swap(SBOVector<DataType, OtherSize, Allocator>& that) {

   }

   void swap(SBOVector& that) {
     auto& this_impl = this->data_.second();
     auto& that_impl = that.data_.second();
     if (this_impl.count_ > BufferSize && that_impl.count_ > BufferSize) {
       // Both External
       std::swap(this_impl.count_, that_impl.count_);
       std::swap(this_impl.external_.data_, that_impl.external_.data_);
       std::swap(this_impl.external_.capacity_, that_impl.external_.capacity_);
     } else if (this_impl.count_ <= BufferSize && that_impl.count_ <= BufferSize) {
       // Both Inline
       auto& large_impl = (this_impl.count_ > that_impl.count_ ? this_impl : that_impl);
       auto& small_impl = (this_impl.count_ > that_impl.count_ ? that_impl : this_impl);
       for (auto i = 0u; i < small_impl.count_; ++i) {
         if constexpr (std::is_swappable_v<DataType>) {
           std::swap<DataType>(large_impl.inline_[i], small_impl.inline_[i]);
         } else if constexpr(std::is_move_constructible_v<DataType> && std::is_move_assignable_v<DataType>) {
           DataType temp(std::move(large_impl.inline_[i]));
           large_impl.inline_[i] = std::move(small_impl.inline_[i]);
           small_impl.inline_[i] = std::move(temp);
         } else if constexpr (std::is_copy_constructible_v<DataType> &&
                              std::is_copy_assignable_v<DataType>) {
           DataType temp(large_impl.inline_[i]);
           large_impl.inline_[i] = small_impl.inline_[i];
           small_impl.inline_[i] = temp;
         } else {
           static_assert(std::is_copy_constructible_v<DataType> &&
                         std::is_copy_assignable_v<DataType>, "DataType is unswappable!");
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
       std::swap(this_impl.count_, that_impl.count_);
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
     DataType* new_data = data_.first().allocate(requested);
     // TODO if using exceptions, throw bad_alloc on nullptr
     if constexpr (std::is_move_assignable_v<DataType>) {
       std::uninitialized_move(begin(), end(), new_data);
     } else {
       std::uninitialized_copy(begin(), end(), new_data);
     }
     std::destroy(begin(), end());
     if (size() > BufferSize) {
       data_.first().deallocate(data_.second().external_.data_,
                                data_.second().external_.capacity_);
     }
     data_.second().external_.data_ = new_data;
     data_.second().external_.capacity_ = requested;
   }
};

#endif // SBOVECTOR_HPP