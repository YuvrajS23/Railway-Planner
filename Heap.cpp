#ifndef HEAP_CPP
#define HEAP_CPP

#ifndef HEAP_H
#include "Heap.h"
#endif

#include <iostream>
#include <stdexcept>
#include <utility>

template <typename T>
TreeNode<T> *Heap<T>::nodeAt(std::size_t oneBasedIndex) const {
  if (oneBasedIndex == 0 || oneBasedIndex > nodeCount) {
    return nullptr;
  }

  // Ignore the most-significant bit (it represents the root), then follow
  // zero bits left and one bits right to reach the indexed complete-tree node.
  std::size_t mask = 1;
  while (mask <= oneBasedIndex / 2) {
    mask <<= 1;
  }
  mask >>= 1;

  TreeNode<T> *current = root;
  while (mask != 0 && current != nullptr) {
    current = (oneBasedIndex & mask) ? current->right : current->left;
    mask >>= 1;
  }
  return current;
}

template <typename T>
TreeNode<T> *Heap<T>::cloneTree(const TreeNode<T> *node,
                                TreeNode<T> *parent) {
  if (node == nullptr) {
    return nullptr;
  }

  TreeNode<T> *copy = new TreeNode<T>(node->object);
  copy->parent = parent;
  try {
    copy->left = cloneTree(node->left, copy);
    copy->right = cloneTree(node->right, copy);
  } catch (...) {
    destroyTree(copy);
    throw;
  }
  return copy;
}

template <typename T>
void Heap<T>::destroyTree(TreeNode<T> *node) {
  if (node == nullptr) {
    return;
  }
  destroyTree(node->left);
  destroyTree(node->right);
  delete node;
}

template <typename T>
Heap<T>::Heap(const Heap<T> &other)
    : root(cloneTree(other.root, nullptr)), nodeCount(other.nodeCount) {}

template <typename T>
Heap<T>::Heap(Heap<T> &&other) noexcept
    : root(other.root), nodeCount(other.nodeCount) {
  other.root = nullptr;
  other.nodeCount = 0;
}

template <typename T>
Heap<T> &Heap<T>::operator=(Heap<T> other) {
  swap(other);
  return *this;
}

template <typename T>
Heap<T>::~Heap() {
  destroyTree(root);
  root = nullptr;
  nodeCount = 0;
}

template <typename T>
void Heap<T>::swap(Heap<T> &other) noexcept {
  using std::swap;
  swap(root, other.root);
  swap(nodeCount, other.nodeCount);
}

template <typename T>
void Heap<T>::siftUp(TreeNode<T> *node) {
  while (node->parent != nullptr && node->parent->object < node->object) {
    using std::swap;
    swap(node->object, node->parent->object);
    node = node->parent;
  }
}

template <typename T>
void Heap<T>::siftDown(TreeNode<T> *node) {
  while (node != nullptr) {
    TreeNode<T> *largest = node;
    if (node->left != nullptr && largest->object < node->left->object) {
      largest = node->left;
    }
    if (node->right != nullptr && largest->object < node->right->object) {
      largest = node->right;
    }
    if (largest == node) {
      return;
    }

    using std::swap;
    swap(node->object, largest->object);
    node = largest;
  }
}

template <typename T>
void Heap<T>::insert(const T &obj) {
  TreeNode<T> *inserted = new TreeNode<T>(obj);
  const std::size_t newIndex = nodeCount + 1;

  if (root == nullptr) {
    root = inserted;
    nodeCount = 1;
    return;
  }

  TreeNode<T> *parent = nodeAt(newIndex / 2);
  inserted->parent = parent;
  if ((newIndex & 1U) == 0U) {
    parent->left = inserted;
  } else {
    parent->right = inserted;
  }
  ++nodeCount;
  siftUp(inserted);
}

template <typename T>
T Heap<T>::getMax() const {
  if (root == nullptr) {
    throw std::underflow_error("cannot read the maximum of an empty heap");
  }
  return root->object;
}

template <typename T>
void Heap<T>::delMax() {
  if (root == nullptr) {
    throw std::underflow_error("cannot delete the maximum of an empty heap");
  }

  if (nodeCount == 1) {
    delete root;
    root = nullptr;
    nodeCount = 0;
    return;
  }

  TreeNode<T> *last = nodeAt(nodeCount);
  root->object = std::move(last->object);
  TreeNode<T> *parent = last->parent;
  if (parent->left == last) {
    parent->left = nullptr;
  } else {
    parent->right = nullptr;
  }
  delete last;
  --nodeCount;
  siftDown(root);
}

template <typename T>
void Heap<T>::printSubtree(const TreeNode<T> *node, std::size_t depth,
                           std::ostream &out) {
  if (node == nullptr) {
    return;
  }
  printSubtree(node->right, depth + 1, out);
  out << std::string(depth * 2, ' ') << node->object << '\n';
  printSubtree(node->left, depth + 1, out);
}

template <typename T>
void Heap<T>::printHeap(TreeNode<T> *node) {
  printSubtree(node, 0, std::cout);
}

template <typename T>
void Heap<T>::printHeap() const {
  if (root == nullptr) {
    std::cout << "(empty)\n";
    return;
  }
  printSubtree(root, 0, std::cout);
}

#endif
