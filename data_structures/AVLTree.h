#ifndef RAILWAY_PLANNER_AVL_TREE_H
#define RAILWAY_PLANNER_AVL_TREE_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

// Ordered set implemented as an AVL tree. Insertions and removals rebalance
// with single or double rotations, keeping search operations O(log n).
template <typename T, typename Compare = std::less<T> >
class AVLTree {
 public:
  enum TraversalOrder { IN_ORDER, PRE_ORDER, POST_ORDER };

 private:
  struct Node {
    T value;
    Node *left;
    Node *right;
    int height;

    explicit Node(const T &item)
        : value(item), left(nullptr), right(nullptr), height(1) {}
  };

  Node *root_;
  std::size_t size_;
  Compare compare_;

  static int nodeHeight(const Node *node) {
    return node == nullptr ? 0 : node->height;
  }

  static void updateHeight(Node *node) {
    node->height = 1 + std::max(nodeHeight(node->left), nodeHeight(node->right));
  }

  static int balanceFactor(const Node *node) {
    return node == nullptr ? 0
                           : nodeHeight(node->left) - nodeHeight(node->right);
  }

  static Node *rotateRight(Node *root) {
    Node *newRoot = root->left;
    Node *movedSubtree = newRoot->right;
    newRoot->right = root;
    root->left = movedSubtree;
    updateHeight(root);
    updateHeight(newRoot);
    return newRoot;
  }

  static Node *rotateLeft(Node *root) {
    Node *newRoot = root->right;
    Node *movedSubtree = newRoot->left;
    newRoot->left = root;
    root->right = movedSubtree;
    updateHeight(root);
    updateHeight(newRoot);
    return newRoot;
  }

  static Node *rebalance(Node *node) {
    if (node == nullptr) {
      return nullptr;
    }
    updateHeight(node);
    const int balance = balanceFactor(node);
    if (balance > 1) {
      if (balanceFactor(node->left) < 0) {
        node->left = rotateLeft(node->left);
      }
      return rotateRight(node);
    }
    if (balance < -1) {
      if (balanceFactor(node->right) > 0) {
        node->right = rotateRight(node->right);
      }
      return rotateLeft(node);
    }
    return node;
  }

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
    copy->height = node->height;
    try {
      copy->left = clone(node->left);
      copy->right = clone(node->right);
    } catch (...) {
      destroy(copy);
      throw;
    }
    return copy;
  }

  Node *insertNode(Node *node, const T &value, bool &inserted) {
    if (node == nullptr) {
      inserted = true;
      return new Node(value);
    }
    if (compare_(value, node->value)) {
      node->left = insertNode(node->left, value, inserted);
    } else if (compare_(node->value, value)) {
      node->right = insertNode(node->right, value, inserted);
    } else {
      return node;
    }
    return rebalance(node);
  }

  Node *removeNode(Node *node, const T &value, bool &removed) {
    if (node == nullptr) {
      return nullptr;
    }

    if (compare_(value, node->value)) {
      node->left = removeNode(node->left, value, removed);
    } else if (compare_(node->value, value)) {
      node->right = removeNode(node->right, value, removed);
    } else {
      removed = true;
      if (node->left == nullptr || node->right == nullptr) {
        Node *child = node->left == nullptr ? node->right : node->left;
        delete node;
        return child;
      }

      Node *successor = node->right;
      while (successor->left != nullptr) {
        successor = successor->left;
      }
      node->value = successor->value;
      bool successorRemoved = false;
      node->right = removeNode(node->right, successor->value,
                               successorRemoved);
    }
    return rebalance(node);
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

  static bool validateBalance(const Node *node, int &computedHeight) {
    if (node == nullptr) {
      computedHeight = 0;
      return true;
    }
    int leftHeight = 0;
    int rightHeight = 0;
    if (!validateBalance(node->left, leftHeight) ||
        !validateBalance(node->right, rightHeight)) {
      return false;
    }
    computedHeight = 1 + std::max(leftHeight, rightHeight);
    const int difference = leftHeight - rightHeight;
    return difference >= -1 && difference <= 1 &&
           node->height == computedHeight;
  }

 public:
  explicit AVLTree(const Compare &compare = Compare())
      : root_(nullptr), size_(0), compare_(compare) {}

  AVLTree(const AVLTree &other)
      : root_(clone(other.root_)),
        size_(other.size_),
        compare_(other.compare_) {}

  AVLTree(AVLTree &&other) noexcept
      : root_(other.root_),
        size_(other.size_),
        compare_(std::move(other.compare_)) {
    other.root_ = nullptr;
    other.size_ = 0;
  }

  AVLTree &operator=(AVLTree other) {
    swap(other);
    return *this;
  }

  ~AVLTree() { clear(); }

  void swap(AVLTree &other) noexcept {
    using std::swap;
    swap(root_, other.root_);
    swap(size_, other.size_);
    swap(compare_, other.compare_);
  }

  bool insert(const T &value) {
    bool inserted = false;
    root_ = insertNode(root_, value, inserted);
    if (inserted) {
      ++size_;
    }
    return inserted;
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

  int height() const { return nodeHeight(root_); }

  bool isBalanced() const {
    int computedHeight = 0;
    return validateBalance(root_, computedHeight);
  }

  void clear() {
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
  }

  bool empty() const { return size_ == 0; }
  std::size_t size() const { return size_; }
};

#endif
