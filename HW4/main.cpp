#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>

using namespace std;

class Interval
{
public:
    int index;
    pair<int, int> boundary;
    bool placed = false;
};

class Net
{
public:
    pair<string, int> track;
    set<int> pinLoc;
    set<int> isAbove;
    set<int> isBelow;
    int aboveCnt = 0;
    int belowCnt = 0;
    pair<int, int> boundary;
};

class ConstraintLeftEdge
{
public:
    void readfile(ifstream &input);
    void init();
    void buildVCG();
    void buildInterval();
    map<int, map<int, int>> buildSideTrack(map<int, map<int, int>>);
    void routing();
    void outputFile(ofstream &output);

private:
    map<int, map<int, int>> T_boundary;
    map<int, map<int, int>> B_boundary;
    map<int, map<int, int>> T_track;
    map<int, map<int, int>> B_track;
    vector<int> TopSequence;
    vector<int> BottomSequence;
    map<int, Net> netTrack;
    vector<Interval> I;
    int totTrackNum = 0;
};

bool compare(Interval a, Interval b);

int main(int argc, char *argv[])
{
    ifstream inputFile(argv[1]);
    ofstream outputFile(argv[2]);
    ConstraintLeftEdge C;
    C.readfile(inputFile);
    inputFile.close();
    C.init();
    C.routing();
    C.outputFile(outputFile);
    return 0;
}

void ConstraintLeftEdge::outputFile(ofstream &output)
{
    output << "Channel density: " << totTrackNum << endl;
    for (auto &s : netTrack)
    {
        output << "Net " << s.first << endl;
        if (s.second.track.first == "C")
        {
            output << "C" << -(s.second.track.second - totTrackNum - 1) << " ";
            output << s.second.boundary.first << " " << s.second.boundary.second << endl;
        }

        else
        {
            output << s.second.track.first << s.second.track.second << " ";
            output << s.second.boundary.first << " " << s.second.boundary.second << endl;
        }
    }
    output.close();
}

void ConstraintLeftEdge::routing()
{
    int netCnt = I.size();
    sort(I.begin(), I.end(), compare);
    vector<int> placeList;

    for (auto it = T_track.rbegin(); it != T_track.rend(); it++)
    {
        for (auto &s : it->second)
        {
            int remainLeft = s.first;
            int remainRight = s.second;
            for (int i = 0; i < I.size(); i++)
            {
                if (I[i].boundary.first >= remainLeft && I[i].boundary.second <= remainRight && I[i].placed == false && netTrack[I[i].index].belowCnt == 0)
                {
                    I[i].placed = true;
                    remainLeft = I[i].boundary.second + 1;
                    netTrack[I[i].index].track.first = "T";
                    netTrack[I[i].index].track.second = it->first;
                    placeList.push_back(I[i].index);
                    netCnt--;
                }
                if (remainLeft >= remainRight)
                {
                    break;
                }
            }
        }

        for (int i = 0; i < placeList.size(); i++)
        {
            for (auto it = netTrack[placeList[i]].isAbove.begin(); it != netTrack[placeList[i]].isAbove.end(); ++it)
            {
                int isAboveID = *it;
                netTrack[isAboveID].belowCnt--;
            }
        }
        placeList.clear();
    }

    for (auto it = B_track.rbegin(); it != B_track.rend(); it++)
    {

        for (auto &s : it->second)
        {
            int remainLeft = s.first;
            int remainRight = s.second;

            for (int i = 0; i < I.size(); i++)
            {
                if (I[i].boundary.first >= remainLeft && I[i].boundary.second <= remainRight && I[i].placed == false && netTrack[I[i].index].aboveCnt == 0)
                {
                    I[i].placed = true;
                    remainLeft = I[i].boundary.second + 1;
                    netTrack[I[i].index].track.first = "B";
                    netTrack[I[i].index].track.second = it->first;
                    placeList.push_back(I[i].index);
                    netCnt--;
                }

                if (remainLeft >= remainRight)
                {
                    break;
                }
            }
        }

        for (int i = 0; i < placeList.size(); i++)
        {
            for (auto it = netTrack[placeList[i]].isBelow.begin(); it != netTrack[placeList[i]].isBelow.end(); ++it)
            {
                int isBelowID = *it;
                netTrack[isBelowID].aboveCnt--;
            }
        }
        placeList.clear();
    }

    totTrackNum = 0;
    while (netCnt != 0)
    {
        totTrackNum++;
        int remainLeft = 0;
        for (int i = 0; i < I.size(); i++)
        {
            if (I[i].placed == false && I[i].boundary.first >= remainLeft && netTrack[I[i].index].belowCnt == 0)
            {
                I[i].placed = true;
                remainLeft = I[i].boundary.second + 1;

                netTrack[I[i].index].track.first = "C";
                netTrack[I[i].index].track.second = totTrackNum;
                placeList.push_back(I[i].index);
                netCnt--;
            }
        }

        for (int i = 0; i < placeList.size(); i++)
        {
            for (auto it = netTrack[placeList[i]].isAbove.begin(); it != netTrack[placeList[i]].isAbove.end(); ++it)
            {
                int isAboveID = *it;
                netTrack[isAboveID].belowCnt--;
            }
        }

        placeList.clear();
    }
}

bool compare(Interval a, Interval b)
{
    return a.boundary.first < b.boundary.first;
}

void ConstraintLeftEdge::buildVCG()
{
    for (int i = 0; i < TopSequence.size(); i++)
    {
        if (TopSequence[i] != 0)
        {
            netTrack[TopSequence[i]].pinLoc.insert(i);
        }

        if (BottomSequence[i] != 0)
        {
            netTrack[BottomSequence[i]].pinLoc.insert(i);
        }

        if (TopSequence[i] != 0 && BottomSequence[i] != 0 && TopSequence[i] != BottomSequence[i])
        {
            if (netTrack[TopSequence[i]].isAbove.count(BottomSequence[i]) == 0)
            {
                netTrack[TopSequence[i]].isAbove.insert(BottomSequence[i]);
                netTrack[TopSequence[i]].aboveCnt++;
            }

            if (netTrack[BottomSequence[i]].isBelow.count(TopSequence[i]) == 0)
            {
                netTrack[BottomSequence[i]].isBelow.insert(TopSequence[i]);
                netTrack[BottomSequence[i]].belowCnt++;
            }
        }
    }
}

void ConstraintLeftEdge::buildInterval()
{
    for (auto &s : netTrack)
    {
        Interval temp;
        s.second.boundary.first = *(s.second.pinLoc.begin());
        s.second.boundary.second = *(--s.second.pinLoc.end());

        temp.boundary = s.second.boundary;
        temp.index = s.first;

        I.push_back(temp);
    }
}

map<int, map<int, int>> ConstraintLeftEdge::buildSideTrack(map<int, map<int, int>> boundary)
{
    map<int, map<int, int>> side_track;
    int left, right;

    for (auto &it : boundary)
    {
        map<int, int> temp_map;
        for (auto &inner : boundary)
        {
            for (auto &in : inner.second)
            {
                temp_map[in.first] = in.second;
            }
            if (&inner == &it)
                break;
        }

        for (auto inner = temp_map.begin(); inner != temp_map.end(); ++inner)
        {
            left = inner->second + 1;
            auto nextInner = std::next(inner);
            if (nextInner != temp_map.end())
            {
                right = nextInner->first - 1;
                if (right - left < 1)
                    continue;

                side_track[it.first][left] = right;
            }
        }

        auto tmp = temp_map.begin();
        if (tmp->first != 0 && tmp->first != 1)
        {
            left = 0;
            right = tmp->first - 1;
            side_track[it.first][left] = right;
        }
        tmp = --temp_map.end();
        if (tmp->second != TopSequence.size() - 1 && tmp->second != TopSequence.size() - 2)
        {
            left = tmp->second + 1;
            right = TopSequence.size() - 1;
            side_track[it.first][left] = right;
        }
    }

    return side_track;
}

void ConstraintLeftEdge::init()
{
    buildVCG();
    buildInterval();
    T_track = buildSideTrack(T_boundary);
    B_track = buildSideTrack(B_boundary);
}

void ConstraintLeftEdge::readfile(ifstream &input)
{
    string line;
    while (getline(input, line))
    {
        stringstream ss(line);
        char dummy_char;
        int trackNum, startPos, endPos;

        if (line[0] == 'T')
        {
            ss >> dummy_char >> trackNum >> startPos >> endPos;
            T_boundary[trackNum][startPos] = endPos;
        }

        else if (line[0] == 'B')
        {
            ss >> dummy_char >> trackNum >> startPos >> endPos;
            B_boundary[trackNum][startPos] = endPos;
        }

        else
        {
            vector<int> tempSequence;
            string tempString;
            int tempInt;
            while (ss >> tempString)
            {
                tempInt = stoi(tempString);
                tempSequence.push_back(tempInt);
                if (tempInt != 0)
                {
                    netTrack[tempInt];
                }
            }

            if (TopSequence.empty())
            {
                TopSequence = tempSequence;
            }
            else
            {
                BottomSequence = tempSequence;
            }
        }
    }
}