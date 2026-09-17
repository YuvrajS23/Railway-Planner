#ifndef PRINT_JOURNEY_CPP
#define PRINT_JOURNEY_CPP

#ifndef STD_HEADERS_H
#include "std_headers.h"
#endif

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <utility>
#include <vector>

using namespace std;

namespace {

const int MINUTES_PER_DAY = 24 * 60;
const int MINUTES_PER_WEEK = 7 * MINUTES_PER_DAY;

// A state includes the service on which the traveller arrived.  That makes
// train changes (rather than every intermediate station) count as stop-overs.
struct JourneyStateKey {
  int station;
  int journeyCode;
  int stopSequence;
  int stopOvers;

  bool operator<(const JourneyStateKey &other) const
  {
    if (station != other.station) return station < other.station;
    if (journeyCode != other.journeyCode) return journeyCode < other.journeyCode;
    if (stopSequence != other.stopSequence) return stopSequence < other.stopSequence;
    return stopOvers < other.stopOvers;
  }
};

struct JourneySearchNode {
  JourneyStateKey key;
  TrainInfoPerStation *arrivalInfo;
  int elapsedMinutes;
  int previousNode;
  int journeyCodeUsed;
  int transferWaitMinutes;
  int travelMinutes;
};

bool parseRailwayTime(int railwayTime, int &minutesAfterMidnight)
{
  if (railwayTime < 0) return false;

  int hours = railwayTime / 100;
  int minutes = railwayTime % 100;
  if ((hours < 0) || (hours > 23) || (minutes < 0) || (minutes > 59)) {
    return false;
  }

  minutesAfterMidnight = hours * 60 + minutes;
  return true;
}

bool hasOperatingDay(const TrainInfoPerStation *trainInfo)
{
  if (trainInfo == nullptr) return false;
  for (int day = 0; day < 7; day++) {
    if (trainInfo->daysOfWeek[day]) return true;
  }
  return false;
}

// Return the shortest forward interval between two weekly timetable events.
// If older input data has no weekday flags, fall back to the clock values.
int timetableInterval(const TrainInfoPerStation *fromInfo,
                      bool useDepartureAtFrom,
                      const TrainInfoPerStation *toInfo,
                      bool useDepartureAtTo)
{
  if ((fromInfo == nullptr) || (toInfo == nullptr)) return -1;

  int fromMinutes = 0;
  int toMinutes = 0;
  int fromTime = useDepartureAtFrom ? fromInfo->depTime : fromInfo->arrTime;
  int toTime = useDepartureAtTo ? toInfo->depTime : toInfo->arrTime;
  if (!parseRailwayTime(fromTime, fromMinutes) ||
      !parseRailwayTime(toTime, toMinutes)) {
    return -1;
  }

  int best = numeric_limits<int>::max();
  if (hasOperatingDay(fromInfo) && hasOperatingDay(toInfo)) {
    for (int fromDay = 0; fromDay < 7; fromDay++) {
      if (!fromInfo->daysOfWeek[fromDay]) continue;
      int fromWeekMinute = fromDay * MINUTES_PER_DAY + fromMinutes;

      for (int toDay = 0; toDay < 7; toDay++) {
        if (!toInfo->daysOfWeek[toDay]) continue;
        int toWeekMinute = toDay * MINUTES_PER_DAY + toMinutes;
        int interval = (toWeekMinute - fromWeekMinute + MINUTES_PER_WEEK) %
                       MINUTES_PER_WEEK;
        best = min(best, interval);
      }
    }
  }
  else {
    best = (toMinutes - fromMinutes + MINUTES_PER_DAY) % MINUTES_PER_DAY;
  }

  return (best == numeric_limits<int>::max()) ? -1 : best;
}

string formatMinutes(int minutes)
{
  ostringstream output;
  output << (minutes / 60) << "h " << (minutes % 60) << "m";
  return output.str();
}

} // namespace

void Planner::printPlanJourneys(string srcStnName, string destStnName,
                                int maxStopOvers, int maxTransitTime)
{
  Entry<int> *sourceEntry = stnNameToIndex.get(srcStnName);
  Entry<int> *destinationEntry = stnNameToIndex.get(destStnName);

  if ((sourceEntry == nullptr) || (destinationEntry == nullptr) ||
      (sourceEntry->value < 0) || (destinationEntry->value < 0)) {
    cout << "Unable to plan route: source or destination station was not found."
         << endl;
    return;
  }

  const int source = sourceEntry->value;
  const int destination = destinationEntry->value;
  maxStopOvers = max(0, maxStopOvers);
  maxTransitTime = max(0, maxTransitTime);

  if (source == destination) {
    cout << "Shortest railway route:" << endl;
    cout << "  " << srcStnName << " (already at destination)" << endl;
    cout << "Stop-overs: 0" << endl;
    return;
  }

  vector<JourneySearchNode> nodes;
  map<JourneyStateKey, int> nodeForState;
  priority_queue<pair<int, int>, vector<pair<int, int> >,
                 greater<pair<int, int> > > frontier;

  JourneySearchNode start;
  start.key.station = source;
  start.key.journeyCode = -1;
  start.key.stopSequence = -1;
  start.key.stopOvers = 0;
  start.arrivalInfo = nullptr;
  start.elapsedMinutes = 0;
  start.previousNode = -1;
  start.journeyCodeUsed = -1;
  start.transferWaitMinutes = 0;
  start.travelMinutes = 0;
  nodes.push_back(start);
  nodeForState[start.key] = 0;
  frontier.push(make_pair(0, 0));

  int destinationNode = -1;

  // Dijkstra runs over timetable-aware states.  Segment duration and transfer
  // wait are non-negative edge weights, so the first destination popped is
  // the shortest feasible scheduled route.
  while (!frontier.empty()) {
    int currentDistance = frontier.top().first;
    int currentNodeIndex = frontier.top().second;
    frontier.pop();

    if (currentDistance != nodes[currentNodeIndex].elapsedMinutes) continue;
    if (nodes[currentNodeIndex].key.station == destination) {
      destinationNode = currentNodeIndex;
      break;
    }

    const JourneySearchNode current = nodes[currentNodeIndex];
    listOfObjects<StationConnectionInfo *> *connection =
        adjacency[current.key.station].toStations;

    while (connection != nullptr) {
      StationConnectionInfo *connectionInfo = connection->object;
      if ((connectionInfo == nullptr) ||
          (connectionInfo->adjacentStnIndex < 0) ||
          (connectionInfo->adjacentStnIndex >= DICT_SIZE)) {
        connection = connection->next;
        continue;
      }

      const int nextStation = connectionInfo->adjacentStnIndex;
      listOfObjects<TrainInfoPerStation *> *outgoing = connectionInfo->trains;

      while (outgoing != nullptr) {
        TrainInfoPerStation *departureInfo = outgoing->object;
        if ((departureInfo == nullptr) || (departureInfo->depTime < 0)) {
          outgoing = outgoing->next;
          continue;
        }

        // Find this service's record at the next station.  stopSeq makes sure
        // the record really is the arrival for this particular graph edge.
        TrainInfoPerStation *nextArrivalInfo = nullptr;
        listOfObjects<TrainInfoPerStation *> *candidate = stationInfo[nextStation];
        while (candidate != nullptr) {
          TrainInfoPerStation *candidateInfo = candidate->object;
          if ((candidateInfo != nullptr) &&
              (candidateInfo->journeyCode == departureInfo->journeyCode) &&
              (candidateInfo->stopSeq == departureInfo->stopSeq + 1)) {
            nextArrivalInfo = candidateInfo;
            break;
          }
          candidate = candidate->next;
        }

        if (nextArrivalInfo == nullptr) {
          outgoing = outgoing->next;
          continue;
        }

        bool continuingOnTrain =
            (current.arrivalInfo != nullptr) &&
            (current.arrivalInfo->journeyCode == departureInfo->journeyCode) &&
            (current.arrivalInfo->stopSeq == departureInfo->stopSeq);
        bool changingTrain =
            (current.arrivalInfo != nullptr) && !continuingOnTrain;
        int nextStopOvers = current.key.stopOvers + (changingTrain ? 1 : 0);
        if (nextStopOvers > maxStopOvers) {
          outgoing = outgoing->next;
          continue;
        }

        int transferWait = 0;
        if (changingTrain) {
          transferWait = timetableInterval(current.arrivalInfo, false,
                                           departureInfo, true);
          if ((transferWait >= 0) &&
              (static_cast<long long>(transferWait) >
               static_cast<long long>(maxTransitTime) * 60)) {
            outgoing = outgoing->next;
            continue;
          }

          // A malformed/legacy timetable cannot provide a reliable wait.
          // It remains searchable, but contributes no guessed wait duration.
          if (transferWait < 0) transferWait = 0;
        }

        int travelTime = timetableInterval(departureInfo, true,
                                           nextArrivalInfo, false);
        // Keep every edge strictly positive for Dijkstra even when timetable
        // fields are missing or departure and arrival times are identical.
        if (travelTime <= 0) travelTime = 1;

        if (currentDistance > numeric_limits<int>::max() -
                              transferWait - travelTime) {
          outgoing = outgoing->next;
          continue;
        }
        int nextDistance = currentDistance + transferWait + travelTime;

        JourneyStateKey nextKey;
        nextKey.station = nextStation;
        nextKey.journeyCode = nextArrivalInfo->journeyCode;
        nextKey.stopSequence = nextArrivalInfo->stopSeq;
        nextKey.stopOvers = nextStopOvers;

        map<JourneyStateKey, int>::iterator known = nodeForState.find(nextKey);
        int nextNodeIndex;
        if (known == nodeForState.end()) {
          JourneySearchNode nextNode;
          nextNode.key = nextKey;
          nextNode.arrivalInfo = nextArrivalInfo;
          nextNode.elapsedMinutes = nextDistance;
          nextNode.previousNode = currentNodeIndex;
          nextNode.journeyCodeUsed = departureInfo->journeyCode;
          nextNode.transferWaitMinutes = transferWait;
          nextNode.travelMinutes = travelTime;
          nodes.push_back(nextNode);
          nextNodeIndex = static_cast<int>(nodes.size()) - 1;
          nodeForState[nextKey] = nextNodeIndex;
          frontier.push(make_pair(nextDistance, nextNodeIndex));
        }
        else {
          nextNodeIndex = known->second;
          if (nextDistance < nodes[nextNodeIndex].elapsedMinutes) {
            nodes[nextNodeIndex].arrivalInfo = nextArrivalInfo;
            nodes[nextNodeIndex].elapsedMinutes = nextDistance;
            nodes[nextNodeIndex].previousNode = currentNodeIndex;
            nodes[nextNodeIndex].journeyCodeUsed = departureInfo->journeyCode;
            nodes[nextNodeIndex].transferWaitMinutes = transferWait;
            nodes[nextNodeIndex].travelMinutes = travelTime;
            frontier.push(make_pair(nextDistance, nextNodeIndex));
          }
        }

        outgoing = outgoing->next;
      }

      connection = connection->next;
    }
  }

  if (destinationNode < 0) {
    cout << "No route from " << srcStnName << " to " << destStnName
         << " satisfies the limits of " << maxStopOvers
         << " stop-over(s) and " << maxTransitTime
         << " transit hour(s) per stop-over." << endl;
    return;
  }

  vector<int> route;
  for (int nodeIndex = destinationNode; nodeIndex >= 0;
       nodeIndex = nodes[nodeIndex].previousNode) {
    route.push_back(nodeIndex);
  }
  reverse(route.begin(), route.end());

  cout << "Shortest railway route (Dijkstra):" << endl;
  cout << "  " << stnNameToIndex.getKeyAtIndex(source) << endl;
  for (size_t i = 1; i < route.size(); i++) {
    const JourneySearchNode &leg = nodes[route[i]];
    cout << "    -- journey " << leg.journeyCodeUsed;
    if (leg.transferWaitMinutes > 0) {
      cout << ", transfer wait " << formatMinutes(leg.transferWaitMinutes);
    }
    cout << ", travel " << formatMinutes(leg.travelMinutes) << " -->" << endl;
    cout << "  " << stnNameToIndex.getKeyAtIndex(leg.key.station) << endl;
  }
  cout << "Total scheduled time: "
       << formatMinutes(nodes[destinationNode].elapsedMinutes) << endl;
  cout << "Stop-overs (train changes): "
       << nodes[destinationNode].key.stopOvers << endl;
}

#endif
