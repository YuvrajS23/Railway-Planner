#ifndef RAILWAY_PLANNER_BINARY_SEARCH_TREE_H
#define RAILWAY_PLANNER_BINARY_SEARCH_TREE_H

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

// A reusable ordered set backed by an unbalanced binary-search tree.
// Duplicate values (as defined by Compare) are rejected.
template <typename T, typename Compare = std::less<T> >
class BinarySearchTree {
 public:
  enum TraversalOrder { IN_ORDER, PRE_ORDER, POST_ORDER };

 private:
  struct Node {
    T value;
    Node *left;
    Node *right;

    explicit Node(const T &item) : value(item), left(nullptr), right(nullptr) {}
  };

  Node *root_;
  std::size_t size_;
  Compare compare_;

  static void destroy(Node *node) {
    if (node == nullptr) {
      return;
    }
    destroy(node->left);
    destroy(node->right);
    delete node;
  }

  static Node *clone(const Node *node) {
    if (node == nullptr) {
      return nullptr;
    }
    Node *copy = new Node(node->value);
    try {
      copy->left = clone(node->left);
      copy->right = clone(node->right);
    } catch (...) {
      destroy(copy);
      throw;
    }
    return copy;
  }

  Node *findNode(const T &value) const {
    Node *current = root_;
    while (current != nullptr) {
      if (compare_(value, current->value)) {
        current = current->left;
      } else if (compare_(current->value, value)) {
        current = current->right;
      } else {
        return current;
      }
    }
    return nullptr;
  }

  Node *removeNode(Node *node, const T &value, bool &removed) {
    if (node == nullptr) {
      return nullptr;
    }

    if (compare_(value, node->value)) {
      node->left = removeNode(node->left, value, removed);
      return node;
    }
    if (compare_(node->value, value)) {
      node->right = removeNode(node->right, value, removed);
      return node;
    }

    removed = true;
    if (node->left == nullptr) {
      Node *right = node->right;
      delete node;
      return right;
    }
    if (node->right == nullptr) {
      Node *left = node->left;
      delete node;
      return left;
    }

    Node *successor = node->right;
    while (successor->left != nullptr) {
      successor = successor->left;
    }
    node->value = successor->value;
    bool successorRemoved = false;
    node->right = removeNode(node->right, successor->value, successorRemoved);
    return node;
  }

  static void appendInOrder(const Node *node, std::vector<T> &values) {
    if (node == nullptr) {
      return;
    }
    appendInOrder(node->left, values);
    values.push_back(node->value);
    appendInOrder(node->right, values);
  }

  static void appendPreOrder(const Node *node, std::vector<T> &values) {
    if (node == nullptr) {
      return;
    }
    values.push_back(node->value);
    appendPreOrder(node->left, values);
    appendPreOrder(node->right, values);
  }

  static void appendPostOrder(const Node *node, std::vector<T> &values) {
    if (node == nullptr) {
      return;
    }
    appendPostOrder(node->left, values);
    appendPostOrder(node->right, values);
    values.push_back(node->value);
  }

 public:
  explicit BinarySearchTree(const Compare &compare = Compare())
      : root_(nullptr), size_(0), compare_(compare) {}

  BinarySearchTree(const BinarySearchTree &other)
      : root_(clone(other.root_)),
        size_(other.size_),
        compare_(other.compare_) {}

  BinarySearchTree(BinarySearchTree &&other) noexcept
      : root_(other.root_),
        size_(other.size_),
        compare_(std::move(other.compare_)) {
    other.root_ = nullptr;
    other.size_ = 0;
  }

  BinarySearchTree &operator=(BinarySearchTree other) {
    swap(other);
    return *this;
  }

  ~BinarySearchTree() { clear(); }

  void swap(BinarySearchTree &other) noexcept {
    using std::swap;
    swap(root_, other.root_);
    swap(size_, other.size_);
    swap(compare_, other.compare_);
  }

  bool insert(const T &value) {
    Node **link = &root_;
    while (*link != nullptr) {
      if (compare_(value, (*link)->value)) {
        link = &((*link)->left);
      } else if (compare_((*link)->value, value)) {
        link = &((*link)->right);
      } else {
        return false;
      }
    }
    *link = new Node(value);
    ++size_;
    return true;
  }

  bool search(const T &value) const { return findNode(value) != nullptr; }
  bool contains(const T &value) const { return search(value); }

  const T *find(const T &value) const {
    Node *node = findNode(value);
    return node == nullptr ? nullptr : &node->value;
  }

  T *find(const T &value) {
    Node *node = findNode(value);
    return node == nullptr ? nullptr : &node->value;
  }

  bool remove(const T &value) {
    bool removed = false;
    root_ = removeNode(root_, value, removed);
    if (removed) {
      --size_;
    }
    return removed;
  }

  bool erase(const T &value) { return remove(value); }

  const T *minimum() const {
    Node *current = root_;
    if (current == nullptr) {
      return nullptr;
    }
    while (current->left != nullptr) {
      current = current->left;
    }
    return &current->value;
  }

  const T *maximum() const {
    Node *current = root_;
    if (current == nullptr) {
      return nullptr;
    }
    while (current->right != nullptr) {
      current = current->right;
    }
    return &current->value;
  }

  std::vector<T> traversal(TraversalOrder order = IN_ORDER) const {
    std::vector<T> values;
    values.reserve(size_);
    if (order == PRE_ORDER) {
      appendPreOrder(root_, values);
    } else if (order == POST_ORDER) {
      appendPostOrder(root_, values);
    } else {
      appendInOrder(root_, values);
    }
    return values;
  }

  std::vector<T> inorder() const { return traversal(IN_ORDER); }
  std::vector<T> preorder() const { return traversal(PRE_ORDER); }
  std::vector<T> postorder() const { return traversal(POST_ORDER); }

  void clear() {
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
  }

  bool empty() const { return size_ == 0; }
  std::size_t size() const { return size_; }
};

#endif
