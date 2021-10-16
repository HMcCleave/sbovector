#ifndef SBOVECTOR_HPP
#define SBOVECTOR_HPP

#include <array>
#include <memory>
#include <initializer_list>

namespace details_ {

template<typename A, typename B>
struct CompactPair final : private A {
  B b_;
  A& first() { return *this; }
  const A& first() const { return *this; }
  B& second() { return b_; }
  const B& second() const { return b_; }
};

} // namespace details_

template<typename DataType, size_t BufferSize, class Allocator = std::allocator<DataType>>
class SBOVector {
 private:

  struct VectorImpl {
    size_t count_;
    union {
      std::array<DataType, BufferSize> inline_;
      struct {
        std::unique_ptr<DataType[], Allocator> data_;
        size_t capacity_;
      } external_;
    };
    VectorImpl() {}
    ~VectorImpl() {}
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

   SBOVector() {}
   explicit SBOVector(const Allocator&) noexcept {}
   SBOVector(size_t, const DataType&, const Allocator& = Allocator()) {}
   explicit SBOVector(size_t, const Allocator& = Allocator()) {}

   template<class InputIter>
   SBOVector(InputIter, InputIter, const Allocator& = Allocator());

   template<int OtherSize>
   SBOVector(const SBOVector<DataType, OtherSize, Allocator>&,
             const Allocator& = Allocator()) {}

   template<int OtherSize>
   SBOVector(SBOVector<DataType, OtherSize, Allocator>&&,
             const Allocator& = Allocator()) noexcept {}

   SBOVector(std::initializer_list<DataType>, const Allocator& = Allocator()) {}

   ~SBOVector() {}

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

   bool empty() const noexcept { return true; }
   size_t size() const noexcept { return 0; }
   size_t max_size() const noexcept { return 0; }
   void reserve(size_t) {}
   size_t capacity() const noexcept { return 0; }
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
   template<class... Args>
   iterator emplace(const_iterator pos, Args&&... args) {
     return begin();
   }

   iterator erase(const_iterator) { return begin(); }
   iterator erase(const_iterator, const_iterator) { return begin(); }

   void push_back(const DataType&) {}
   void push_back(DataType&&) {}

   template<class... Args>
   reference emplace_back(Args&&...) { return back(); }

   void pop_back() {}

   void resize(size_t) {}
   void resize(size_t, const DataType&) {}

   void swap(SBOVector&) {}
};

#endif // SBOVECTOR_HPP