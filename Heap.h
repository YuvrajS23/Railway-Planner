#ifndef HEAP_H
#define HEAP_H

#ifndef STD_HEADERS
#include "std_headers.h"
#endif

#include <cstddef>
#include <iosfwd>

// A node is kept public for compatibility with the original assignment API.
// Heap owns every node reachable from its root.
template <typename T> class TreeNode {
 public:
  T object;
  TreeNode<T> *left;
  TreeNode<T> *right;
  TreeNode<T> *parent;

  explicit TreeNode(const T &initObj)
      : object(initObj), left(nullptr), right(nullptr), parent(nullptr) {}
};

// Complete binary max-heap backed by linked tree nodes. Priority is defined
// only in terms of T::operator<, so custom value types need no extra helpers.
template <typename T> class Heap {
 private:
  TreeNode<T> *root;
  std::size_t nodeCount;

  TreeNode<T> *nodeAt(std::size_t oneBasedIndex) const;
  static TreeNode<T> *cloneTree(const TreeNode<T> *node,
                                TreeNode<T> *parent);
  static void destroyTree(TreeNode<T> *node);
  static void printSubtree(const TreeNode<T> *node, std::size_t depth,
                           std::ostream &out);
  void siftUp(TreeNode<T> *node);
  void siftDown(TreeNode<T> *node);

 public:
  Heap() : root(nullptr), nodeCount(0) {}
  Heap(const Heap<T> &other);
  Heap(Heap<T> &&other) noexcept;
  Heap<T> &operator=(Heap<T> other);
  ~Heap();

  void swap(Heap<T> &other) noexcept;

  void insert(const T &obj);
  void delMax();
  T getMax() const;

  bool empty() const { return nodeCount == 0; }
  std::size_t size() const { return nodeCount; }

  // Prints a sideways tree. The pointer overload is retained for callers of
  // the original API; printHeap() prints the full heap.
  void printHeap(TreeNode<T> *node);
  void printHeap() const;
};

// Template definitions must be visible wherever Heap<T> is instantiated.
#include "Heap.cpp"

#endif

