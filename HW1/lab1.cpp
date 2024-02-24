#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string.h>
#include <chrono>
#include <algorithm>
#include <unordered_set>
#include <limits.h>
#include <utility>

using namespace std;

typedef vector<vector<int>> vect2d;
typedef vector<int> vect;
typedef vector<unordered_set<int>> vu;

vect2d createNetArray(ifstream &input, int netNum, float *balance_factor, int *nodeNum);
vect2d createCellArray(vect2d &netArray, int nodeNumber, int *maxCut);
int getNetNum(ifstream &count);
int getCellNum(vect2d &cellArray);
vect init_partition(int *leftCellCount, int *rightCellCount, vect2d &cellArray, int cellNum);
void printPartition(vect partition);
vect calculate_gain(vect &partition, vect2d &netArray, vect2d &cellArray);
int FS(vect &partition, vect2d &netArray, vect2d &cellArray, int cellNum);
int TE(vect &partition, vect2d &netArray, vect2d &cellArray, int cellNum);
void gainList(vect &gain, vu &leftGainList, vu &rightGainList, vect &partition);
void printGain(vect gain);
void printNetArray(vect2d netArray);
void printCellArray(vect2d cellArray);
void printGainList(vu leftGainList, vu rightGainList, int maxCut);
void init_lockState(vect &lockState, vect2d &cellArray);
void printLock(vect lockstate);
pair<int, int> maxCellGain(vu &leftGainList, vu &rightGainList, int cellNum, int leftCellCount, int rightCellCount);
void updateLockState(vect &lockState, int maxGain_id);
void updateNeighberGain(int moveID, vu &leftGainList, vu &rightGainList, vect &lockState, vect2d &netArray, vect2d &cellArray, vect &partition, vect &gain);
void updatePartition(vect &partition, int *leftCellCount, int *rightCellCount, int maxGain_id);
void outputOpPartition(ofstream &output, int leftCellCount, int rightCellCount, vect &partition, vect2d &netArray);

auto start_time = chrono::high_resolution_clock::now();
float balance_factor;
int netNum = 0;
int nodeNum = 0;
int maxCut = 0;

int main(int argc, char *argv[])
{
    ifstream input;
    ifstream count;
    ofstream output;

    vect2d netArray;
    vect2d cellArray;

    vect partition;
    vect opPartition;
    vect gain;

    string input_filename;
    string line;

    int leftCellCount = 0;
    int rightCellCount = 0;
    int cellNum = 0;
    int endCount = 0;

    input_filename = argv[1];

    input.open(argv[1]);
    output.open(argv[2]);
    count.open(argv[1]);
    netNum = getNetNum(count);

    netArray = createNetArray(input, netNum, &balance_factor, &nodeNum);
    cellArray = createCellArray(netArray, nodeNum, &maxCut);
    cellNum = getCellNum(cellArray);
    // printCellArray(cellArray);
    // printNetArray(netArray);
    // cout << "maxCut = " << maxCut << endl;
    // cout << "netNum = " << netNum << endl;
    // cout << "cellNum = " << cellNum << endl;
    vu leftGainList(2 * maxCut + 1);
    vu rightGainList(2 * maxCut + 1);
    vect lockState(nodeNum + 1);
    partition = init_partition(&leftCellCount, &rightCellCount, cellArray, cellNum);
    // printPartition(partition);
    gain = calculate_gain(partition, netArray, cellArray);
    gainList(gain, leftGainList, rightGainList, partition);
    // cout << "==============================" << endl;
    // printGainList(leftGainList, rightGainList, maxCut);
    init_lockState(lockState, cellArray);
    pair<int, int> temp;
    int moveID = 0;
    int thisgain = 0;
    int opLeftCellCount = leftCellCount;
    int opRightCellCount = rightCellCount;
    int time = 0;
    int total_gain = 0;
    int temp_gain = 0;
    opPartition.assign(partition.begin(), partition.end());
    while (1)
    {
        for (int i = 0; i < cellNum; i++)
        {
            temp = maxCellGain(leftGainList, rightGainList, cellNum, leftCellCount, rightCellCount);
            moveID = temp.first;
            thisgain = temp.second;
            total_gain += (thisgain - maxCut);
            // if (i % 10000 == 0)
            //     cout << "i = " << i << endl;
            // cout << "*************************" << endl;
            // cout << "moveID = " << moveID << endl;
            // cout << "total gain = " << total_gain << endl;
            updateLockState(lockState, moveID);
            // cout << "before neighbor" << endl;
            updateNeighberGain(moveID, leftGainList, rightGainList, lockState, netArray, cellArray, partition, gain);
            // cout << "after neighbor" << endl;
            updatePartition(partition, &leftCellCount, &rightCellCount, moveID);
            // printGainList(leftGainList, rightGainList, maxCut);
            // cout << "================================" << endl;
            if (total_gain >= temp_gain)
            {
                // cout << "memorized" << endl;
                temp_gain = total_gain;
                opLeftCellCount = leftCellCount;
                opRightCellCount = rightCellCount;
                opPartition.assign(partition.begin(), partition.end());
            }
        }
        // break;
        if (temp_gain == 0)
        {
            endCount++;
        }

        if (endCount == 5)
        {
            break;
        }
        total_gain = 0;
        temp_gain = 0;
        partition.assign(opPartition.begin(), opPartition.end());
        leftCellCount = opLeftCellCount;
        rightCellCount = opRightCellCount;
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
        // outputOpPartition(output, leftCellCount, rightCellCount, partition, netArray);
        // cout << "duration = " << duration.count() << endl;
        if (double(duration.count()) > 300000000) // 5 sec
        {
            break;
        }
        // outputOpPartition(output, leftCellCount, rightCellCount, partition, netArray);
        init_lockState(lockState, cellArray);
        gain = calculate_gain(partition, netArray, cellArray);
        gainList(gain, leftGainList, rightGainList, partition);
        time++;
    }

    outputOpPartition(output, leftCellCount, rightCellCount, opPartition, netArray);

    return 0;
}

void outputOpPartition(ofstream &output, int leftCellCount, int rightCellCount, vect &partition, vect2d &netArray)
{
    // cout << "in op partition" << endl;
    int cutSize = 0;
    int checkSame;
    leftCellCount = 0;
    rightCellCount = 0;
    // printPartition(partition);
    // cout << "(int)netArray.size() = " << (int)netArray.size() << endl;
    // cout << netArray[1][0] << endl;
    for (int i = 1; i < partition.size(); i++)
    {
        // cout << "i = " << i << endl;
        if (partition[i] == 0)
        {
            leftCellCount++;
        }

        else if (partition[i] == 1)
        {
            rightCellCount++;
        }
    }
    for (int i = 1; i < (int)netArray.size(); i++)
    {
        // cout << "Net number = " << i << endl;
        for (int j = 0; j < netArray[i].size(); j++)
        {
            // cout << "netArray[i].size()" << netArray[i].size() << endl;
            if (j == 0)
            {
                checkSame = partition[netArray[i][j]];
                // cout << checkSame << endl;
            }
            if (checkSame != partition[netArray[i][j]])
            {
                cutSize++;
                break;
            }
        }
    }
    // cout << "Cutsize = " << cutSize << endl;
    output << "Cutsize = " << cutSize << endl;
    output << "G1 " << leftCellCount << endl;
    for (int i = 1; i < nodeNum + 1; i++)
    {
        if (partition[i] == 0)
        {
            output << "c" << i << " ";
        }
    }
    // cout << "here" << endl;
    output << ";" << endl;
    output << "G2 " << rightCellCount << endl;
    for (int i = 1; i < nodeNum + 1; i++)
    {
        if (partition[i] == 1)
        {
            output << "c" << i << " ";
        }
    }
    output << ";" << endl;

    output.close();
    // cout << "end op" << endl;
}

void updatePartition(vect &partition, int *leftCellCount, int *rightCellCount, int maxGain_id)
{
    if (partition[maxGain_id] == 1)
    {
        // cout << "moving " << maxGain_id << " right to left" << endl;
        partition[maxGain_id] = 0;
        (*leftCellCount)++;
        (*rightCellCount)--;
    }

    else if (partition[maxGain_id] == 0)
    {
        // cout << "moving " << maxGain_id << " left to right" << endl;
        partition[maxGain_id] = 1;
        (*leftCellCount)--;
        (*rightCellCount)++;
    }
}

void updateNeighberGain(int moveID, vu &leftGainList, vu &rightGainList, vect &lockState, vect2d &netArray, vect2d &cellArray, vect &partition, vect &gain)
{
    int netID;
    // cout << "moveID = " << moveID << endl;
    int moveID_init_partition = partition[moveID];
    int fromBlockCellCount = 0;
    int toBlockCellCount = 0;
    // cout << "moveID = " << moveID << endl;
    // printPartition(partition);
    for (int i = 0; i < cellArray[moveID].size(); i++)
    {
        fromBlockCellCount = 0;
        toBlockCellCount = 0;
        netID = cellArray[moveID][i];
        // cout << "netID = " << netID << endl;
        for (int j = 0; j < netArray[netID].size(); j++)
        {
            if (partition[netArray[netID][j]] == moveID_init_partition)
            {
                fromBlockCellCount++;
            }

            else
            {
                toBlockCellCount++;
            }
        }
        // cout << "moveID_init_partition = " << moveID_init_partition << endl;
        // cout << "fromBlockCellCount = " << fromBlockCellCount << endl;
        // cout << "toBlockCellCount = " << toBlockCellCount << endl;

        if (toBlockCellCount == 0)
        {
            for (int j = 0; j < netArray[netID].size(); j++)
            {
                if (lockState[netArray[netID][j]] == 0)
                {
                    if (moveID_init_partition == 0)
                    {
                        leftGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                        leftGainList[gain[netArray[netID][j]] + maxCut + 1].emplace(netArray[netID][j]);
                        gain[netArray[netID][j]] = gain[netArray[netID][j]] + 1;
                    }

                    else
                    {
                        rightGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                        rightGainList[gain[netArray[netID][j]] + maxCut + 1].emplace(netArray[netID][j]);
                        gain[netArray[netID][j]] = gain[netArray[netID][j]] + 1;
                    }

                    // if (moveID_init_partition == 0)
                    // {
                    //     for (int k = 2 * maxCut; k >= 0; k--) // 2 * maxCut; i >= 0
                    //     {
                    //         if (leftGainList[k].find(netArray[netID][j]) == leftGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             auto temp_L = leftGainList[k].find(netArray[netID][j]);
                    //             leftGainList[k].erase(temp_L);
                    //             leftGainList[k + 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }

                    // else
                    // {
                    //     for (int k = 2 * maxCut; k >= 0; k--)
                    //     {
                    //         if (rightGainList[k].find(netArray[netID][j]) == rightGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             auto temp_R = rightGainList[k].find(netArray[netID][j]);
                    //             rightGainList[k].erase(temp_R);
                    //             rightGainList[k + 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }
                }
            }
        }

        if (toBlockCellCount == 1)
        {
            for (int j = 0; j < netArray[netID].size(); j++)
            {
                if (lockState[netArray[netID][j]] == 0)
                {
                    if (moveID_init_partition == 0)
                    {
                        if (partition[netArray[netID][j]] == 1)
                        {
                            rightGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                            rightGainList[gain[netArray[netID][j]] + maxCut - 1].emplace(netArray[netID][j]);
                            gain[netArray[netID][j]] = gain[netArray[netID][j]] - 1;
                            break;
                        }
                    }

                    else
                    {
                        if (partition[netArray[netID][j]] == 0)
                        {
                            leftGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                            leftGainList[gain[netArray[netID][j]] + maxCut - 1].emplace(netArray[netID][j]);
                            gain[netArray[netID][j]] = gain[netArray[netID][j]] - 1;
                            break;
                        }
                    }

                    // if (moveID_init_partition == 0)
                    // {
                    //     for (int k = 2 * maxCut; k >= 0; k--)
                    //     {
                    //         if (rightGainList[k].find(netArray[netID][j]) == rightGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             auto temp_R = rightGainList[k].find(netArray[netID][j]);
                    //             rightGainList[k].erase(temp_R);
                    //             rightGainList[k - 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }

                    // else
                    // {
                    //     for (int k = 2 * maxCut; k >= 0; k--)
                    //     {
                    //         if (leftGainList[k].find(netArray[netID][j]) == leftGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             auto temp_L = leftGainList[k].find(netArray[netID][j]);
                    //             leftGainList[k].erase(temp_L);
                    //             leftGainList[k - 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }
                }
            }
        }

        if (fromBlockCellCount == 2)
        {
            for (int j = 0; j < netArray[netID].size(); j++)
            {
                if (lockState[netArray[netID][j]] == 0)
                {
                    if (moveID_init_partition == 0)
                    {
                        if (partition[netArray[netID][j]] == 0)
                        {
                            leftGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                            leftGainList[gain[netArray[netID][j]] + maxCut + 1].emplace(netArray[netID][j]);
                            gain[netArray[netID][j]] = gain[netArray[netID][j]] + 1;
                            break;
                        }
                    }

                    else
                    {
                        if (partition[netArray[netID][j]] == 1)
                        {
                            rightGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                            rightGainList[gain[netArray[netID][j]] + maxCut + 1].emplace(netArray[netID][j]);
                            gain[netArray[netID][j]] = gain[netArray[netID][j]] + 1;
                            break;
                        }
                    }
                    // if (moveID_init_partition == 0)
                    // {
                    //     for (int k = 2 * maxCut; k >= 0; k--)
                    //     {
                    //         if (leftGainList[k].find(netArray[netID][j]) == leftGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             auto temp_L = leftGainList[k].find(netArray[netID][j]);
                    //             leftGainList[k].erase(temp_L);
                    //             leftGainList[k + 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }

                    // else
                    // {
                    //     for (int k = 2 * maxCut; k >= 0; k--)
                    //     {
                    //         if (rightGainList[k].find(netArray[netID][j]) == rightGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             auto temp_R = rightGainList[k].find(netArray[netID][j]);
                    //             rightGainList[k].erase(temp_R);
                    //             rightGainList[k + 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }
                }
            }
        }
        // cout << "moveID_init_partition = " << moveID_init_partition << endl;
        if (fromBlockCellCount == 1)
        {
            // cout << "here" << endl;
            for (int j = 0; j < netArray[netID].size(); j++)
            {
                // cout << "netArray[netID] = " << netArray[netID][j] << endl;
                if (lockState[netArray[netID][j]] == 0)
                {
                    if (moveID_init_partition == 0)
                    {
                        rightGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                        rightGainList[gain[netArray[netID][j]] + maxCut - 1].emplace(netArray[netID][j]);
                        gain[netArray[netID][j]] = gain[netArray[netID][j]] - 1;
                    }

                    else
                    {
                        leftGainList[gain[netArray[netID][j]] + maxCut].erase(netArray[netID][j]);
                        leftGainList[gain[netArray[netID][j]] + maxCut - 1].emplace(netArray[netID][j]);
                        gain[netArray[netID][j]] = gain[netArray[netID][j]] - 1;
                    }
                    // if (moveID_init_partition == 0)
                    // {
                    //     // printPartition(partition);
                    //     // cout << "===================" << endl;
                    //     // printGainList(leftGainList, rightGainList, maxCut);
                    //     // cout << "netArray[netID][j] = " << netArray[netID][j] << endl;
                    //     for (int k = 2 * maxCut; k >= 0; k--)
                    //     {
                    //         if (rightGainList[k].find(netArray[netID][j]) == rightGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             // cout << "k = " << k << endl;
                    //             // cout << "find it!" << endl;
                    //             auto temp_R = rightGainList[k].find(netArray[netID][j]);
                    //             rightGainList[k].erase(temp_R);
                    //             rightGainList[k - 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }

                    // else
                    // {
                    //     for (int k = 2 * maxCut; k >= 0; k--)
                    //     {
                    //         if (leftGainList[k].find(netArray[netID][j]) == leftGainList[k].end())
                    //         {
                    //             continue;
                    //         }

                    //         else
                    //         {
                    //             auto temp_L = leftGainList[k].find(netArray[netID][j]);
                    //             leftGainList[k].erase(temp_L);
                    //             leftGainList[k - 1].emplace(netArray[netID][j]);
                    //             break;
                    //         }
                    //     }
                    // }
                }
            }
        }
    }
}

void updateLockState(vect &lockState, int maxGain_id)
{
    lockState[maxGain_id] = 1;
}

pair<int, int> maxCellGain(vu &leftGainList, vu &rightGainList, int cellNum, int leftCellCount, int rightCellCount)
{
    int maxGain_id = 0;
    // cout << "BLANCED_FACTOR * (float(leftCellCount+rightCellCount)/2) = "<<BLANCED_FACTOR * (float(leftCellCount+rightCellCount))<<endl;
    float lower;
    float upper;
    lower = ((1.0 - balance_factor) / 2.0) * (float(cellNum));
    upper = ((1.0 + balance_factor) / 2.0) * (float(cellNum));
    // cout << "lower = " << lower << endl;
    // cout << "upper = " << upper << endl;

    // move cell freely

    for (int i = 2 * maxCut; i >= 0; i--)
    {
        if (leftGainList[i].empty() && rightGainList[i].empty())
        {
            // cout << "if both the gain list are empty for this gain value" << endl;
            continue;
        }

        else if (!leftGainList[i].empty() && !rightGainList[i].empty())
        {
            if ((leftCellCount - 1) < lower)
            {
                maxGain_id = *(rightGainList[i].begin());
                rightGainList[i].erase(maxGain_id);
                return make_pair(maxGain_id, i);
            }

            else if ((rightCellCount - 1) < lower)
            {
                maxGain_id = *(leftGainList[i].begin());
                leftGainList[i].erase(maxGain_id);
                return make_pair(maxGain_id, i);
            }

            else if ((leftCellCount + 1) > upper)
            {
                maxGain_id = *(leftGainList[i].begin());
                leftGainList[i].erase(maxGain_id);
                return make_pair(maxGain_id, i);
            }

            else if ((rightCellCount + 1) > upper)
            {
                maxGain_id = *(rightGainList[i].begin());
                rightGainList[i].erase(maxGain_id);
                return make_pair(maxGain_id, i);
            }

            // move freely, then left to right
            else
            {
                maxGain_id = *(leftGainList[i].begin());
                leftGainList[i].erase(maxGain_id);
                return make_pair(maxGain_id, i);
            }
        }

        else if (!leftGainList[i].empty() && rightGainList[i].empty())
        {
            if ((leftCellCount - 1) < lower)
            {
                continue;
            }
            else
            {
                maxGain_id = *(leftGainList[i].begin());
                leftGainList[i].erase(maxGain_id);
                return make_pair(maxGain_id, i);
            }
        }

        else if (leftGainList[i].empty() && !rightGainList[i].empty())
        {
            if ((rightCellCount - 1) < lower)
            {
                continue;
            }
            else
            {
                maxGain_id = *(rightGainList[i].begin());
                rightGainList[i].erase(maxGain_id);
                return make_pair(maxGain_id, i);
            }
        }
    }
    return make_pair(-1, -1);
}

void init_lockState(vect &lockState, vect2d &cellArray)
{
    for (int i = 1; i <= nodeNum; i++)
    {
        if (!cellArray[i].empty())
        {
            lockState[i] = 0;
        }
        else
        {
            lockState[i] = -1;
        }
    }
}

void printLock(vect lockstate)
{
    for (int i = 1; i < lockstate.size(); i++)
    {
        cout << "cell " << i << "'s lockstate = " << lockstate[i] << endl;
    }
}

void printGain(vect gain)
{
    for (int i = 1; i < gain.size(); i++)
    {
        cout << "cell " << i << "'s gain = " << gain[i] << endl;
    }
}
vect calculate_gain(vect &partition, vect2d &netArray, vect2d &cellArray)
{
    // cout << "in calculate gain" << endl;
    vect gain(nodeNum + 1);
    for (int i = 1; i <= nodeNum; i++)
    {
        if (!cellArray[i].empty())
        {
            gain[i] = FS(partition, netArray, cellArray, i) - TE(partition, netArray, cellArray, i);
        }

        else
        {
            gain[i] = INT_MIN;
        }
    }
    // cout << "end calculate gain" << endl;
    return gain;
}
void printPartition(vect partition)
{
    for (int i = 1; i < partition.size(); i++)
    {
        cout << "cell " << i << " = " << partition[i] << endl;
    }
}
vect init_partition(int *leftCellCount, int *rightCellCount, vect2d &cellArray, int cellNum)
{
    vect partition((nodeNum + 1)); // start from index 1
    *leftCellCount = 0;
    *rightCellCount = 0;
    float lower = ((1.0 - balance_factor) / 2.0) * (float(cellNum));
    float upper = ((1.0 + balance_factor) / 2.0) * (float(cellNum));
    // cout << "balance_factor = " << balance_factor << endl;
    // cout << "lower = " << lower << endl;
    // cout << "upper = " << upper << endl;
    int lower_bound = int(lower) + 1;
    bool flag = false;
    //
    // for (int i = 1; i <= lower_bound; i++)
    // {
    //     if (!cellArray[i].empty())
    //     {
    //         partition[i] = 0;
    //         (*leftCellCount)++;
    //     }

    //     else
    //     {
    //         partition[i] = -1;
    //     }
    // }

    // for (int i = lower_bound + 1; i <= nodeNum; i++)
    // {
    //     if (!cellArray[i].empty())
    //     {
    //         partition[i] = 1;
    //         (*rightCellCount)++;
    //     }
    //     else
    //     {
    //         partition[i] = -1;
    //     }
    // }

    for (int i = 1; i <= nodeNum; i++)
    {
        if (!cellArray[i].empty())
        {
            if (flag == false)
            {
                partition[i] = 0;
                (*leftCellCount)++;
                if ((*leftCellCount) > lower_bound)
                {
                    flag = true;
                }
            }

            else if (flag == true)
            {
                partition[i] = 1;
                (*rightCellCount)++;
            }
        }

        else
        {
            partition[i] = -1;
        }
    }

    return partition;
}

int getCellNum(vect2d &cellArray)
{
    int cellNum = 0;
    // cout << "cellArray size = " << cellArray.size() << endl;
    for (int i = 1; i < cellArray.size(); i++)
    {
        if (!cellArray[i].empty())
        {
            // cout << "cellArray[" << i << "] is not empty!" << endl;
            cellNum++;
        }
    }
    return cellNum;
}
int getNetNum(ifstream &count)
{
    netNum = 0;
    string dummy;
    count >> dummy;
    count.ignore(1);
    string line;
    // cout << "getline(input, line) = " << static_cast<int>(getline(input, line)) << endl;
    while (getline(count, line))
    {
        istringstream iss(line);
        string value;
        while (iss >> value)
        {
            if (value.at(0) == 'N' || value.at(0) == 'n' || value.at(0) == ';' || value.at(0) == '0')
            {
                if (value.at(0) == ';')
                {
                    // cout << value << endl;
                    netNum++;
                }
            }
        }
        // netArrayindex++;
    }
    return netNum;
}
vect2d createNetArray(ifstream &input, int netNum, float *balance_factor, int *nodeNum)
{
    int netArrayindex = 1;
    input >> *balance_factor;
    input.ignore(1);
    string line;
    // cout << "getline(input, line) = " << static_cast<int>(getline(input, line)) << endl;
    vect2d netArray(netNum + 1);

    while (getline(input, line, ';'))
    {
        unordered_set<int> temp;
        istringstream iss(line);
        string value;
        int node;
        // cout << "in while" << endl;
        while (iss >> value)
        {
            if (value.at(0) == 'N' || value.at(0) == 'n')
            {
                continue;
            }

            else
            {
                // cout << "value = " << value << endl;
                node = stoi(value.erase(0, 1));
                temp.insert(node);
                // netArray[netArrayindex].push_back(node);
                if (node > *nodeNum)
                {
                    *nodeNum = node;
                }
            }
        }
        for (auto it = temp.begin(); it != temp.end(); it++)
        {
            netArray[netArrayindex].push_back(*it);
        }

        netArrayindex++;
        // netArrayindex++;
    }

    input.close();
    return netArray;
}
int FS(vect &partition, vect2d &netArray, vect2d &cellArray, int cellNum)
{
    int FS_result = 0;
    int alone = 0;
    // read only
    for (const auto &i : cellArray[cellNum])
    {
        for (const auto &s : netArray[i])
        {
            alone = 1;
            if (s != cellNum)
            {
                if (partition[s] == partition[cellNum])
                {
                    alone = 0;
                    break;
                }
            }
        }
        FS_result += alone;
    }
    return FS_result;
}

int TE(vect &partition, vect2d &netArray, vect2d &cellArray, int cellNum)
{
    int TE_result = 0;
    int allSame = 0;

    // read only
    for (const auto &i : cellArray[cellNum])
    {
        for (const auto &s : netArray[i])
        {
            allSame = 1;
            if (s != cellNum)
            {
                if (partition[s] != partition[cellNum])
                {
                    allSame = 0;
                    break;
                }
            }
        }
        TE_result += allSame;
    }
    return TE_result;
}

vect2d createCellArray(vect2d &netArray, int nodeNumber, int *maxCut)
{
    vect2d cellArray(nodeNumber + 1);
    vector<int> pinCount(nodeNumber + 1);
    *(maxCut) = 0;
    for (int i = 0; i < netArray.size(); i++)
    {
        for (int j = 0; j < netArray[i].size(); j++)
        {
            cellArray[netArray[i][j]].push_back(i);
        }
    }

    for (int i = 1; i < cellArray.size(); i++)
    {
        if (cellArray[i].size() > *(maxCut))
        {
            *(maxCut) = cellArray[i].size();
        }
    }
    return cellArray;
}

void printNetArray(vect2d netArray)
{
    for (int i = 1; i < (int)netArray.size(); i++)
    {
        cout << "Net number = " << i << ": ";
        for (const auto &s : netArray[i])
        {
            cout << s << " ";
        }
        cout << endl;
    }
}

void printCellArray(vect2d cellArray)
{
    for (int i = 1; i < (int)cellArray.size(); i++)
    {
        cout << "cell number = " << i << ": ";
        for (const auto &s : cellArray[i])
        {
            cout << s << " ";
        }
        cout << endl;
    }
}
void printGainList(vu leftGainList, vu rightGainList, int maxCut)
{
    cout << "leftGainList: " << endl;
    for (int i = 0; i < (int)leftGainList.size(); i++)
    {
        if (maxCut == i)
        {
            cout << "-->";
        }
        cout << "gain = " << i << ": ";
        for (const auto &s : leftGainList[i])
        {
            cout << s << " ";
        }
        cout << endl;
    }
    cout << "**********************" << endl;

    cout << "rightGainList: " << endl;
    for (int i = 0; i < (int)rightGainList.size(); i++)
    {
        if (maxCut == i)
        {
            cout << "-->";
        }
        cout << "gain = " << i << ": ";
        for (const auto &s : rightGainList[i])
        {
            cout << s << " ";
        }
        cout << endl;
    }
}

void gainList(vect &gain, vu &leftGainList, vu &rightGainList, vect &partition)
{
    for (int i = 0; i < leftGainList.size(); i++)
    {
        leftGainList[i].clear();
    }
    for (int i = 0; i < rightGainList.size(); i++)
    {
        rightGainList[i].clear();
    }
    // printPartition(partition);
    for (int i = 1; i <= nodeNum; i++)
    {
        if (partition[i] == 1)
        {
            // cout << "in right" << endl;
            // cout << "i = " << i << endl;
            rightGainList[maxCut + gain[i]].emplace(i);
        }
        else if (partition[i] == 0)
        {
            leftGainList[maxCut + gain[i]].emplace(i);
        }
    }
    // printGainList(leftGainList, rightGainList, 5);
}