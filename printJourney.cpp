#ifndef PRINT_JOURNEY_CPP
#define PRINT_JOURNEY_CPP

#ifndef STD_HEADERS_H
#include "std_headers.h"
#endif

using namespace std;

// Customized Train information per station with additional StopOver count
class TrainInfoPerStation {
public:
  int journeyCode; // same as train number
  unsigned short stopSeq; // sequence of this station's stop in
                          // itinerary of train, i.e. 1st stop in journey
                          // or 2nd stop in journey or ...
  bool daysOfWeek[7]; // Days of week when this train travels
                      // to/from this station
  int arrTime;  // Arrival time at station; -1 if train starts from here
  int depTime;  // Departure time from station; -1 if train ends here
  int StopOVerCount;

public:
  TrainInfoPerStation(int jCode, unsigned short stpSq, int aTm, int dTm)
  {
    journeyCode = jCode;
    stopSeq = stpSq;
    arrTime = aTm;
    depTime = dTm;
    for (int i = 0; i < 7; i++) { daysOfWeek[i] = false;}
    StopOverCount = 0;
  }

  ~TrainInfoPerStation() {;}

  void setDayOfWeek(int i) { if ((0 <= i) && (i < 7)) daysOfWeek[i] = true; }
  void resetDayOfWeek(int i) { if ((0 <= i) && (i < 7)) daysOfWeek[i] = false; }
    
};

// Queue for storing adjacent vertices into list and pop first element and add its adjacent vertices to the end of the list
class Queue{

  StationConnectionInfo** adjancencyFiltered;

  // Track of index to insert next element
  int nextIndex;

  public:

  Queue() {
    adjancencyFiltered = new StationConnectionInfo* [DICT_SIZE];

    for (int i = 0; i < DICT_SIZE; i++) {
      adjancencyFiltered[i] = new StationConnectionInfo();
    }

    nextIndex = 0;
  }

  void insert(StationConnectionInfo* next) {
    adjancencyFiltered[nextIndex] = next;
    nextIndex++;
  }

  // Returns first element and removes it from the list
  StationConnectionInfo* pop() {

    StationConnectionInfo* temp = adjancencyFiltered[0];
    for (int i = 0; i < nextIndex; i++) {
      adjancencyFiltered[i] = adjancencyFiltered[i+1];
    }
    nextIndex--;

    return temp;
  }

  // Checks if Queue is empty or not
  bool isEmpty() {
    return nextIndex == 0;
  }

};
  
void Planner::printPlanJourneys(string srcStnName, string destStnName, int maxStopOvers, int maxTransitTime)
{

  // insert your code here
  
  // Whenever you need to print a journey, construct a
  // listOfObjects<TrainInfoPerStation *> appropriately, and then
  // use printStationInfo that we had used in Lab 7.
  // printStationInfo is a private member function of
  // the Planner class. It is declared in planner.h and implemented in
  // planner.cpp


  // Stationindex of source
  int index1 = stnNameToIndex.get(srcStnName)->value;

  // Stationindex of destination
  int index2 = stnNameToIndex.get(destStnName)->value;

  // To track if printList variable has been created or not
  bool made = false;
  listOfObjects<TrainInfoPerStation *> * printList;
  listOfObjects<listOfObjects<TrainInfoPerStation *>> * trackList;

  Queue* queue = new Queue();

  // index of source
  StationAdjacencyList temp = adjacency[index1];
  // Adjacent vertices of source to next station rather than backwards
  listOfObjects<StationConnectionInfo *> *to = temp.toStations;

  // Traverse through all adjacent vertices
  while(to != nullptr){

    listOfObjects<TrainInfoPerStation *> *temp = to->object->trains;
    
    // Station index of adjacent vertex
    int nextindex = to->object->adjacentStnIndex;

    if (nextindex == index2) {
      while (temp != nullptr) {
        if (!made) {
          // printList = new listOfObjects<TrainInfoPerStation *>(temp->object);
          made = true;
        } else {
          // printList->next = new listOfObjects<TrainInfoPerStation *>(temp->object);
        }
        temp = temp->next;
      }
    } else {
      
      // If station not found, then we will add it to the queue
      queue->insert(to->object);
    }

    to = to->next;
  }

  // First while loop is for looping through all adjacent vertices of source
  // Based on the first loop, we have added necessary vertices to the queue
  // Second while loop is for looping through all vertices in the queue
  // Only intersection of trains based on same JourneyCode will be added to "newInsert" and inserted to queue


  while (!queue->isEmpty()) {
      
      StationConnectionInfo* temp = queue->pop();


      //Adjacent
      StationAdjacencyList temp3 = adjacency[temp->adjacentStnIndex];
      listOfObjects<StationConnectionInfo *> *to2 = temp3.toStations;


      StationConnectionInfo* newInsert = new StationConnectionInfo();

      while(to2 != nullptr){

        bool added = false;

      listOfObjects<TrainInfoPerStation *>* temp2 = temp->trains;

        // Trains of current station
        while (temp2 != nullptr) {

        listOfObjects<TrainInfoPerStation *> *temp4 = to2->object->trains;


          // temp4 => Trains of next station
          while(temp4 != nullptr) {

            if (temp4->object->depTime - temp2->object->arrTime <= maxTransitTime) {

              int nextindex = to2->object->adjacentStnIndex;

              if (nextindex == index2) {
                if (!made) {
                  printList = new listOfObjects<TrainInfoPerStation *>(temp4->object);
                  made = true;
                } else {
                  printList->next = new listOfObjects<TrainInfoPerStation *>(temp4->object);
                }
                temp4 = temp4->next;
                continue;
              }

              listOfObjects<TrainInfoPerStation *> *temp5 = newInsert->trains;
              listOfObjects<TrainInfoPerStation *> *parenttemp5 = newInsert->trains;

              while (temp5 != nullptr) {
                parenttemp5 = temp5;
                temp5 = temp5->next;
              }

              listOfObjects<TrainInfoPerStation *> * duplicatetemp = new listOfObjects<TrainInfoPerStation *>(temp2->object);

              if(parenttemp5 == nullptr) {
                newInsert->trains = duplicatetemp;
              } else {
                parenttemp5->next = duplicatetemp;
              }

              added = true;
            }

            temp4 = temp4->next;
          }

          temp2 = temp2->next;
      }

        if(added) {

          newInsert->adjacentStnIndex = to2->object->adjacentStnIndex;
          queue->insert(newInsert);
          newInsert = new StationConnectionInfo();
        }

        to2 = to2->next;
      }

  }

  if (!made) {
    cout << "No direct journeys available" << endl;
  } else {
    printStationInfo(printList);
  }

  
  return;
}

#endif
