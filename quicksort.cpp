#ifndef QUICKSORT_CPP
#define QUICKSORT_CPP

#ifndef STD_HEADERS
#include "std_headers.h"
#endif

#ifndef PLANNER_H
#include "planner.h"
#endif

#ifndef DICTIONARY_H
#include "dictionary.h"
#endif

#ifndef CODES_H
#include "codes.h"
#endif

#include <cstddef>
#include <utility>

// If you are using the updated planner.cpp, then you simply need to
// sort the list of TrainInfoPerStation objects in stnInfoList.  The
// function that calls Quicksort in planner.cpp (i.e. printStationInfo)
// will then automatically pretty-print the sorted list.
//
// USING THE UPDATED planner.cpp FILE IS STRONGLY RECOMMENDED
//
// Notice that there is a variable K in Quicksort that is to be used as
// described in the problem statement in problems_statement.pdf
// Specifically, if the first call to Quicksort has an argument list of
// n TrainInfoPerStation objects, you can't use additional storage space
// for more than n/K TrainInfoPerStation objects overall (across all
// recursive calls), you must implement random choice for pivot (i.e.
// each element in the list being sorted should be equally likely to
// be chosen as pivot), and each call to finding a random pivot
// within the list being sorted must take time within O(K), regardless
// of how long the list to be sorted is.
//
// In the function signature of Quicksort, stnInfoList is a list of
// TrainInfoPerStation objects that is to be sorted. In the updated
// planner.cpp file, this list (unsorted) is already prepared for you,
// and you need to simply ensure that the list is sorted (see note
// below about sorting order) when Quicksort returns.
//
// A note about the sorting order:
//
// The final list should be sorted with respect to day of week (first)
// and departure time within the day (next).  Thus Sun 900 < Sun 1100
// < Mon 800 < Thu 700 < Fri 1200
//
//
// Based on how we saw some of you were trying to approach the problem
// in the lab of Sep 23, we are providing another function QuicksortSimple
// with a slightly extended signature. In addition to the list stnInfoList,
// which is passed as the first argument to QuicksortSimple, there are two
// integer parameters "start" and "end", just like in the usual Quicksort
// of an array (as in Prof. Naveen Garg's lectures, for example).
// How do we interpret these "start" and "end" variables in this case,
// when we are trying to sort a list?
// Well, here is one way of thinking about this: If we start
// counting elements of the list starting from stnInfoList and
// chasing "next" pointers until nullptr is reached, the corresponding
// elements can be thought of as elements of a (virtual) array indexed
// from 0 onwards.
// Then, the call to Quicksort must sort the part of the list
// consisting of elements at indices start, start+1, ... until end
// of the above virtual array.
// The whole point of this assignment is to have you do this without
// converting the whole list to an array.
//
// Remember it is indeed possible to solve this problem using the
// function Quicksort with only stnInfoList as its argument.  However,
// if you are finding it difficult to implement Quicksort, you can
// implement QuicksortSimple instead.  Those who implement both
// Quicksort and QuicksortSimple potentially stand to gain some bonus
// points.

namespace {

typedef listOfObjects<TrainInfoPerStation *> TrainNode;

int firstOperatingDay(const TrainInfoPerStation *train) {
  for (int day = 0; day < 7; ++day) {
    if (train->daysOfWeek[day]) {
      return day;
    }
  }
  // A malformed/no-service entry sorts after every valid day.
  return 7;
}

bool trainComesBefore(const TrainInfoPerStation *left,
                      const TrainInfoPerStation *right) {
  if (left == right) {
    return false;
  }
  if (left == nullptr) {
    return false;
  }
  if (right == nullptr) {
    return true;
  }

  const int leftDay = firstOperatingDay(left);
  const int rightDay = firstOperatingDay(right);
  if (leftDay != rightDay) {
    return leftDay < rightDay;
  }
  if (left->depTime != right->depTime) {
    return left->depTime < right->depTime;
  }

  // The remaining fields make output repeatable when departure keys tie.
  if (left->arrTime != right->arrTime) {
    return left->arrTime < right->arrTime;
  }
  if (left->journeyCode != right->journeyCode) {
    return left->journeyCode < right->journeyCode;
  }
  if (left->stopSeq != right->stopSeq) {
    return left->stopSeq < right->stopSeq;
  }
  for (int day = 0; day < 7; ++day) {
    if (left->daysOfWeek[day] != right->daysOfWeek[day]) {
      return left->daysOfWeek[day] < right->daysOfWeek[day];
    }
  }
  return false;
}

TrainNode *middleNode(TrainNode *first, std::size_t length) {
  TrainNode *middle = first;
  for (std::size_t i = 0; i < length / 2; ++i) {
    middle = middle->next;
  }
  return middle;
}

struct PartitionResult {
  TrainNode *pivot;
  std::size_t leftSize;
};

PartitionResult partitionRange(TrainNode *first, TrainNode *last,
                               std::size_t length) {
  // A middle-element pivot avoids the common quadratic case for data already
  // ordered by day and time. Swapping payload pointers preserves list links.
  TrainNode *chosenPivot = middleNode(first, length);
  using std::swap;
  swap(chosenPivot->object, last->object);

  TrainInfoPerStation *pivotValue = last->object;
  TrainNode *boundary = first->prev;
  std::size_t leftSize = 0;

  for (TrainNode *cursor = first; cursor != last; cursor = cursor->next) {
    // cursor <= pivot, expressed using only the strict comparator.
    if (!trainComesBefore(pivotValue, cursor->object)) {
      boundary = (boundary == nullptr) ? first : boundary->next;
      swap(boundary->object, cursor->object);
      ++leftSize;
    }
  }

  boundary = (boundary == nullptr) ? first : boundary->next;
  swap(boundary->object, last->object);
  return PartitionResult{boundary, leftSize};
}

void quicksortRange(TrainNode *first, TrainNode *last, std::size_t length) {
  // Recurse only into the smaller partition and iterate over the larger one.
  // This caps auxiliary stack space at O(log n), including poor pivot splits.
  while (first != nullptr && last != nullptr && length > 1) {
    const PartitionResult result = partitionRange(first, last, length);
    const std::size_t rightSize = length - result.leftSize - 1;

    if (result.leftSize < rightSize) {
      if (result.leftSize > 1) {
        quicksortRange(first, result.pivot->prev, result.leftSize);
      }
      first = result.pivot->next;
      length = rightSize;
    } else {
      if (rightSize > 1) {
        quicksortRange(result.pivot->next, last, rightSize);
      }
      last = result.pivot->prev;
      length = result.leftSize;
    }
  }
}

}  // namespace

void Planner::Quicksort(listOfObjects<TrainInfoPerStation *> *stnInfoList) {
  if (stnInfoList == nullptr) {
    return;
  }

  TrainNode *last = stnInfoList;
  std::size_t length = 1;
  while (last->next != nullptr) {
    last = last->next;
    ++length;
  }
  quicksortRange(stnInfoList, last, length);
}

void Planner::QuicksortSimple(
    listOfObjects<TrainInfoPerStation *> *stnInfoList, int start, int end) {
  if (stnInfoList == nullptr || start < 0 || end < start) {
    return;
  }

  TrainNode *first = stnInfoList;
  for (int index = 0; index < start && first != nullptr; ++index) {
    first = first->next;
  }
  if (first == nullptr) {
    return;
  }

  TrainNode *last = first;
  std::size_t length = 1;
  const std::size_t requestedLength =
      static_cast<std::size_t>(end) - static_cast<std::size_t>(start) + 1;
  while (length < requestedLength && last->next != nullptr) {
    last = last->next;
    ++length;
  }
  quicksortRange(first, last, length);
}

#endif
