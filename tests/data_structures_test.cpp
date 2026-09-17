#include "../Heap.h"
#include "../data_structures/AVLTree.h"
#include "../data_structures/BinarySearchTree.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

void testHeap() {
  Heap<int> heap;
  const int input[] = {4, 10, -3, 7, 7, 25, 1, 18};
  for (std::size_t i = 0; i < sizeof(input) / sizeof(input[0]); ++i) {
    heap.insert(input[i]);
  }
  assert(heap.size() == 8);
  assert(heap.getMax() == 25);

  // Copies own independent node trees.
  Heap<int> copied(heap);
  copied.insert(99);
  assert(copied.getMax() == 99);
  assert(heap.getMax() == 25);

  const int expected[] = {25, 18, 10, 7, 7, 4, 1, -3};
  for (std::size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
    assert(heap.getMax() == expected[i]);
    heap.delMax();
  }
  assert(heap.empty());

  bool getThrew = false;
  try {
    heap.getMax();
  } catch (const std::underflow_error &) {
    getThrew = true;
  }
  assert(getThrew);

  bool deleteThrew = false;
  try {
    heap.delMax();
  } catch (const std::underflow_error &) {
    deleteThrew = true;
  }
  assert(deleteThrew);

  Heap<int> moved(std::move(copied));
  assert(copied.empty());
  assert(moved.getMax() == 99);

  // Exercise the public printer without adding noise to the test output.
  std::ostringstream printed;
  std::streambuf *original = std::cout.rdbuf(printed.rdbuf());
  moved.printHeap();
  std::cout.rdbuf(original);
  assert(!printed.str().empty());
}

void testBinarySearchTree() {
  BinarySearchTree<int> tree;
  const int input[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
  for (std::size_t i = 0; i < sizeof(input) / sizeof(input[0]); ++i) {
    assert(tree.insert(input[i]));
  }
  assert(!tree.insert(6));
  assert(tree.size() == 9);
  assert(tree.search(7));
  assert(!tree.search(5));
  assert(*tree.minimum() == 1);
  assert(*tree.maximum() == 14);

  const int sortedInput[] = {1, 3, 4, 6, 7, 8, 10, 13, 14};
  const std::vector<int> expected(sortedInput,
                                  sortedInput + sizeof(sortedInput) /
                                                    sizeof(sortedInput[0]));
  assert(tree.inorder() == expected);
  assert(tree.preorder().size() == tree.size());
  assert(tree.postorder().size() == tree.size());

  assert(tree.remove(1));   // leaf
  assert(tree.remove(14));  // one child
  assert(tree.remove(3));   // two children
  assert(!tree.remove(42));
  const int remainingInput[] = {4, 6, 7, 8, 10, 13};
  const std::vector<int> remaining(
      remainingInput,
      remainingInput + sizeof(remainingInput) / sizeof(remainingInput[0]));
  assert(tree.inorder() == remaining);

  BinarySearchTree<int> copied = tree;
  assert(copied.remove(8));
  assert(!copied.search(8));
  assert(tree.search(8));
}

void testAVLTree() {
  AVLTree<int> tree;
  for (int value = 1; value <= 100; ++value) {
    assert(tree.insert(value));
    assert(tree.isBalanced());
  }
  assert(!tree.insert(50));
  assert(tree.size() == 100);
  assert(tree.height() <= 8);

  for (int value = 1; value <= 100; value += 2) {
    assert(tree.remove(value));
    assert(tree.isBalanced());
  }
  assert(!tree.remove(101));
  assert(tree.size() == 50);

  const std::vector<int> values = tree.inorder();
  assert(std::is_sorted(values.begin(), values.end()));
  for (std::size_t i = 0; i < values.size(); ++i) {
    assert(values[i] == static_cast<int>((i + 1) * 2));
    assert(tree.contains(values[i]));
  }

  AVLTree<int> copied(tree);
  copied.clear();
  assert(copied.empty());
  assert(tree.size() == 50);
  assert(tree.isBalanced());
}

}  // namespace

int main() {
  testHeap();
  testBinarySearchTree();
  testAVLTree();
  std::cout << "All heap, BST, and AVL tests passed.\n";
  return 0;
}
