#ifndef SBOVECTOR_HPP
#define SBOVECTOR_HPP

#include <array>
#include <memory>
#include <initializer_list>
#include <type_traits>

namespace details_ {

template<typename T>
constexpr bool is_compactable() {
  return (std::is_empty_v<T> && !std::is_final_v<T>);
}


template<typename A, typename B, bool = is_compactable<A>()>
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
      : a_(), b_(std::forward<B_Args>(b_args), ...) {}

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
    VectorImpl() : count_(0), inline_{} {}

    VectorImpl(Allocator& alloc, size_t count, const DataType& value)
      : count_(count) {
      DataType* init_ptr = inline_.data();
      if (count_ > BufferSize) {
        external_.data_ = alloc.allocate(count);
        external_.capacity_ = count_;
        init_ptr = external_.data_;
      }
      while (count--) {
        *init_ptr++ = value;
      }
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

  struct Iterator {

  };

 public:
   using value_type = DataType;
   using allocator_type = Allocator;
   using size_type = size_t;
   using pointer = DataType*;
   using const_pointer = const pointer;
   using iterator = Iterator;
   using const_iterator = const Iterator;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;
   using reference = DataType&;
   using const_reference = const reference;

   SBOVector() : data_(std::false_type{}) {}

   explicit SBOVector(const Allocator& a) noexcept
       : data_(std::true_type{}, a) {}

   SBOVector(size_t count, const DataType& value, const Allocator& alloc = Allocator())
       : data_(std::true_type{}, alloc) {
     data_.second() = {data_.first(), count, value};
   }

   explicit SBOVector(size_t, const Allocator& = Allocator()) {}

   template <typename InputIter>
   SBOVector(InputIter, InputIter, const Allocator& = Allocator());

   template<int OtherSize>
   SBOVector(const SBOVector<DataType, OtherSize, Allocator>&,
             const Allocator& = Allocator()) {}

   template<int OtherSize>
   SBOVector(SBOVector<DataType, OtherSize, Allocator>&&,
             const Allocator& = Allocator()) noexcept {}

   SBOVector(std::initializer_list<DataType>, const Allocator& = Allocator()) {}

   ~SBOVector() {
     data_.second().clear(data_.first());
   }

   template<int OtherSize>
   SBOVector& operator=(
       const SBOVector<DataType, OtherSize, Allocator>& other) {
     return *this;
   }
   template<int OtherSize>
   SBOVector& operator=(SBOVector<DataType, OtherSize, Allocator>&&) {
     return *this;
   }

   SBOVector& operator=(std::initializer_list<DataType>) {
     return *this;
   }
   
   void assign(size_t, const DataType&) {}

   template<typename InputIt>
   void assign(InputIt, InputIt) {}

   void assign(std::initializer_list<DataType>) {}

   Allocator get_allocator() const noexcept { return Allocator(); }

   reference at(size_t) { return inline_.at(0); }
   const_reference at(size_t) const { return inline_.at(0); }

   reference operator[](size_t index) { return at(index); }
   const_reference operator[](size_t index) const { return at(index); }

   reference front() { return at(0); }
   const_reference front() const { return at(0); }

   reference back() { return at(0); }
   const_reference back() const { return at(0); }

   pointer data() noexcept { return nullptr; }
   const_pointer data() const noexcept { return nullptr; }

   iterator begin() noexcept { 
     return {};
   }
   const_iterator begin() const noexcept { return {};
   }

   const_iterator cbegin() const noexcept { return begin();
   }

   iterator end() noexcept { return {}; }
   const_iterator end() const noexcept { return {}; }
   const_iterator cend() const noexcept { return end(); }

   bool empty() const noexcept { return 0 == size(); }
   size_t size() const noexcept { return data_.second().count_; }
   size_t max_size() const noexcept { return 0; }
   void reserve(size_t) {}

   size_t capacity() const noexcept {
     if (size() <= BufferSize)  // wrt:c++20 [[likely]]
       return BufferSize;
     return data_.second().external_.capacity_;
   }

   void shrink_to_fit() {}

   void clear() noexcept {}

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

   void push_back(const DataType&) {}
   void push_back(DataType&&) {}

   template <typename... Args>
   reference emplace_back(Args&&...) { return back(); }

   void pop_back() {}

   void resize(size_t) {}
   void resize(size_t, const DataType&) {}

   void swap(SBOVector&) {}
};

#endif // SBOVECTOR_HPP