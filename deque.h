#pragma once

#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

template <typename T>
class Deque {
 private:
  size_t first = 0;
  size_t sz = 0;
  std::vector<T*> arr;

  static const size_t CHUNK = 16;

  static size_t col(size_t ind) { return ind % CHUNK; }
  static size_t row(size_t ind) { return ind / CHUNK; }
  size_t afterLast() const { return first + sz; }
  static size_t devByChunk(size_t cnt) { return (cnt + CHUNK - 1) / CHUNK; }
  size_t cap() const { return arr.size() * CHUNK; }
  static T* newChunk() {
    return reinterpret_cast<T*>(new char[sizeof(T) * CHUNK]);
  }
  static void delChunk(T* p) { delete[] reinterpret_cast<char*>(p); }

  static size_t getStart(size_t capacity) { return capacity / 2; }

  static size_t startPos(size_t n) { return n / 2; }

  void setStart() {
    if (sz == 0) {
      first = startPos(cap());
    }
  }

  void fullClear() {
    delElements();
    delChunks(arr);
    sz = 0;
    arr = std::vector<T*>();
  }

  size_t getInd(size_t num) const { return first + num; }
  T* getPtr(size_t num) {
    size_t ind = getInd(num);
    return arr[row(ind)] + col(ind);
  }

  const T* getPtr(size_t num) const {
    size_t ind = getInd(num);
    return arr[row(ind)] + col(ind);
  }

  void delElements() {
    for (size_t i = 0; i < sz; ++i) {
      getPtr(i)->~T();
    }
  }

  void setChunks(std::vector<T*>& cur) {
    try {
      for (size_t i = 0; i < cur.size(); ++i) {
        cur[i] = newChunk();
      }
    } catch (...) {
      delChunks(cur);
      cur = std::vector<T*>();
      throw;
    }
  }

  void delChunks(std::vector<T*>& cur) {
    for (size_t i = 0; i < cur.size(); ++i) {
      if (cur[i] != nullptr) {
        delChunk(cur[i]);
        cur[i] = nullptr;
      }
    }
  }

 public:
  Deque() = default;

  Deque(const Deque<T>& d) : arr(devByChunk(d.sz), nullptr) {
    setChunks(arr);

    try {
      for (size_t i = 0; i < d.sz; ++i) {
        new (getPtr(i)) T(d[i]);
        ++sz;
      }
    } catch (...) {
      fullClear();
      throw;
    }
  }

  Deque(int n) : Deque(n, T()) {}

  Deque(int n, const T& el) : arr(devByChunk(n), nullptr) {
    setChunks(arr);

    try {
      for (int i = 0; i < n; ++i) {
        new (getPtr(i)) T(el);
        ++sz;
      }
    } catch (...) {
      fullClear();
      throw;
    }
  }

  Deque<T>& operator=(const Deque<T>& d) {
    if (this == &d) {
      return *this;
    }
    Deque copy(d);
    mySwap(copy);
    return *this;
  }

  void mySwap(Deque<T>& d) {
    std::swap(first, d.first);
    std::swap(sz, d.sz);
    std::swap(arr, d.arr);
  }

  size_t size() const { return sz; }

  T& operator[](size_t ind) { return *getPtr(ind); }

  const T& operator[](size_t ind) const { return *getPtr(ind); }

  T& at(size_t ind) {
    if (0 <= ind && ind < sz) {
      return *getPtr(ind);
    } else {
      throw std::out_of_range("you lose");
    }
  }

  const T& at(size_t ind) const {
    if (0 <= ind && ind < sz) {
      return *getPtr(ind);
    } else {
      throw std::out_of_range("you lose");
    }
  }

  void push_back(const T& el) {
    if (afterLast() == cap()) {
      std::vector<T*> newarr(std::max(static_cast<size_t>(1), arr.size() * 3),
                             nullptr);
      size_t startRow = newarr.size() / 3;
      size_t start = startRow * CHUNK + first;
      for (size_t i = 0; i < arr.size(); ++i) {
        newarr[i + startRow] = arr[i];
      }
      newarr[row(start + sz)] = newChunk();
      try {
        new (newarr[row(start + sz)] + col(start + sz)) T(el);
      } catch (...) {
        delChunk(newarr[row(start + sz)]);
        throw;
      }
      arr = newarr;
      first = start;
    } else {
      if (arr[row(first + sz)] == nullptr) {
        arr[row(first + sz)] = newChunk();
      }
      new (getPtr(sz)) T(el);
    }
    ++sz;
  }

  void pop_back() {
    --sz;
    getPtr(sz)->~T();
  }

  void push_front(const T& el) {
    if (first == 0) {
      std::vector<T*> newarr(std::max(static_cast<size_t>(3), arr.size() * 3),
                             nullptr);
      size_t startRow = newarr.size() / 3;
      size_t start = startRow * CHUNK;
      for (size_t i = 0; i < arr.size(); ++i) {
        newarr[i + startRow] = arr[i];
      }
      newarr[startRow - 1] = newChunk();
      try {
        new (newarr[startRow - 1] + CHUNK - 1) T(el);
      } catch (...) {
        delChunk(newarr[startRow - 1]);
        throw;
      }
      arr = newarr;
      first = start - 1;
    } else {
      if (arr[row(first - 1)] == nullptr) {
        arr[row(first - 1)] = newChunk();
      }
      new (getPtr(-1)) T(el);
      --first;
    }
    ++sz;
  }

  void pop_front() {
    getPtr(0)->~T();
    ++first;
    --sz;
  }

  template <bool IsConst>
  class CommonIterator {
   public:
    using value_type = T;
    using ppointer = std::conditional_t<IsConst, T* const*, T**>;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using difference_type = ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

   private:
    friend Deque;

    int row, col;
    ppointer arr;

   public:
    CommonIterator(int r, int c, ppointer arr) : row(r), col(c), arr(arr) {}

    CommonIterator(const CommonIterator<false>& other)
        : row(other.row), col(other.col), arr(other.arr) {}
    CommonIterator& operator=(const CommonIterator<false>& other) {
      row = other.row;
      col = other.col;
      arr = other.arr;
      return *this;
    }
    CommonIterator& operator++() {
      ++col;
      if (col == CHUNK) {
        ++row;
        col = 0;
      }
      return *this;
    }

    CommonIterator& operator--() {
      if (col == 0) {
        col = CHUNK - 1;
        --row;
      } else {
        --col;
      }
      return *this;
    }

    CommonIterator operator++(int) {
      auto copy = *this;
      operator++();
      return copy;
    }

    CommonIterator operator--(int) {
      auto copy = *this;
      operator--();
      return copy;
    }

    CommonIterator& operator+=(ptrdiff_t x) {
      int ind = row * CHUNK + col + x;
      row = Deque::row(ind);
      col = Deque::col(ind);
      return *this;
    }

    CommonIterator& operator-=(ptrdiff_t x) { return operator+=(-x); }

    CommonIterator operator+(ptrdiff_t x) const {
      CommonIterator copy(*this);
      copy += x;
      return copy;
    }

    CommonIterator operator-(ptrdiff_t x) const { return this->operator+(-x); }

    bool operator<(const CommonIterator& other) const {
      return arr < other.arr || (arr == other.arr && ind() < other.ind());
    }
    bool operator==(const CommonIterator& other) const {
      return ind() == other.ind() && arr == other.arr;
    }
    bool operator<=(const CommonIterator& other) const {
      return !(other < *this);
    }
    bool operator>(const CommonIterator& other) const { return other < *this; }
    bool operator>=(const CommonIterator& other) const {
      return !(*this < other);
    }
    bool operator!=(const CommonIterator& other) const {
      return !(*this == other);
    }

    ptrdiff_t operator-(const CommonIterator& other) const {
      return ind() - other.ind();
    }

    reference operator*() const { return *(arr[row] + col); }

    pointer operator->() const { return arr[row] + col; }

    int ind() const { return row * CHUNK + col; }
    int getRow() const { return row; }
    int getCol() const { return col; }
  };

  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;

  iterator begin() { return iterator(row(first), col(first), arr.data()); }
  const_iterator begin() const { return cbegin(); }
  const_iterator cbegin() const {
    return const_iterator(row(first), col(first), arr.data());
  }

  iterator end() {
    return iterator(row(first + sz), col(first + sz), arr.data());
  }
  const_iterator end() const { return cend(); }
  const_iterator cend() const {
    return const_iterator(row(first + sz), col(first + sz), arr.data());
  }

  std::reverse_iterator<iterator> rbegin() {
    return std::reverse_iterator(end());
  }
  std::reverse_iterator<const_iterator> rbegin() const { return crbegin(); }
  std::reverse_iterator<const_iterator> crbegin() const {
    return std::reverse_iterator(cend());
  }

  std::reverse_iterator<iterator> rend() {
    return std::reverse_iterator(begin());
  }
  std::reverse_iterator<const_iterator> rend() const { return crend(); }
  std::reverse_iterator<const_iterator> crend() const {
    return std::reverse_iterator(cbegin());
  }

  void insert(iterator it, T el) {
    if (sz != 0) {
      while (it != end()) {
        std::swap(el, *it);
        ++it;
      }
    }
    push_back(el);
  }

  void erase(iterator it) {
    ++it;
    while (it != end()) {
      std::swap(*(it - 1), *it);
      ++it;
    }
    pop_back();
  }

  ~Deque() {
    delElements();
    delChunks(arr);
  }
};
