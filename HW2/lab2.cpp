#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stack>
#include <cstdlib>
#include <cmath>
#include <queue>
#include <list>
#include <algorithm>
#include <climits>

using namespace std;

class Block
{
public:
    vector<pair<int, int>> size;
    string name;
    Block *leftChild = nullptr;
    Block *rightChild = nullptr;
    pair<int, int> outCorrdiinate;
    pair<int, int> WidthNheight;
    pair<int, int> initWH;
};

class realBlock
{
public:
    vector<pair<int, int>> widthHeight;
    vector<string> name;
};

int readFile(ifstream &FILE, float &lower, float &upper);
vector<string> initNPE(int totBlock);
vector<Block *> initBlock(vector<string> &NPE);
void SB(Block *tempL, Block *tempR, Block *OP);
float BottomUp(vector<Block *> &BlockID);
pair<vector<int>, vector<int>> findPosition(vector<string> &NPE);
void m1(vector<string> &NE, vector<int> &operandPosition);
void m2(vector<string> &NE, vector<int> &operatorPosition);
void m3(vector<string> &NE, vector<int> &operatorPosition, vector<int> &operandPosition);
float costFunction(int area, float ratio);
void Delenew(vector<Block *> &BlockID);
// void SA(vector<string> &NPE, vector<int> &operatorPosition, vector<int> &operandPosition, float minCost);
vector<string> SA(vector<string> &NPE, vector<int> &operatorPosition, vector<int> &operandPosition, float minCost);
void TopDown(vector<Block *> &BlockID, vector<string> &NPE);
void outputFile(vector<Block *> &BlockID, ofstream &output, int minArea);
pair<int, int> MergeH(pair<int, int> const &a, pair<int, int> const &b);
pair<int, int> MergeV(pair<int, int> const &a, pair<int, int> const &b);

realBlock block;
// vector<pair<int, int>> block;
float alpha = 0.2;
float lower, upper;
int totArea;
float ratioStar;

int main(int argc, char *argv[])
{
    // string input_filename = "test.txt";
    srand(time(NULL));
    totArea = 0;
    // ifstream input("input300.txt");
    // ofstream output("123.txt");
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    int totBlock;
    totBlock = readFile(input, lower, upper);
    ratioStar = (upper + lower) / 2;

    vector<string> NPE;
    vector<Block *> BlockID;

    pair<vector<int>, vector<int>> tempPosition;
    vector<int> operatorPosition;
    vector<int> operandPosition;
    vector<string> BestNPE;
    NPE = initNPE(totBlock);
    int minArea = 0;
    while (true)
    {
        minArea = INT_MAX;
        BlockID = initBlock(NPE);
        tempPosition = findPosition(NPE);
        operatorPosition = tempPosition.first;
        operandPosition = tempPosition.second;

        float minCost = 0;
        minCost = BottomUp(BlockID);
        BestNPE = SA(NPE, operatorPosition, operandPosition, minCost);
        BlockID = initBlock(NPE);
        minCost = BottomUp(BlockID);

        int tempArea;
        float minRatio = 0;

        for (int i = 0; i < BlockID[BlockID.size() - 1]->size.size(); i++)
        {
            tempArea = BlockID[BlockID.size() - 1]->size[i].first * BlockID[BlockID.size() - 1]->size[i].second;
            int longer = max(BlockID[BlockID.size() - 1]->size[i].first, BlockID[BlockID.size() - 1]->size[i].second);
            int shorter = min(BlockID[BlockID.size() - 1]->size[i].first, BlockID[BlockID.size() - 1]->size[i].second);
            float tempRatio = (float)longer / (float)shorter;
            if (tempArea < minArea)
            {
                minRatio = tempRatio;
                minArea = tempArea;
                BlockID[BlockID.size() - 1]->WidthNheight = {BlockID[BlockID.size() - 1]->size[i].first, BlockID[BlockID.size() - 1]->size[i].second};
            }
        }
        if (minRatio >= lower && minRatio <= upper)
            break;
    }
    // BestNPE = SA(NPE, operatorPosition, operandPosition, minCost);
    // // for (const std::string &element : BestNPE)
    // // {
    // //     std::cout << element << std::endl;
    // // }

    // BlockID = initBlock(NPE);
    // minCost = BottomUp(BlockID);

    // int minArea = INT_MAX;
    // int tempArea;
    // float minRatio = 0;

    // for (int i = 0; i < BlockID[BlockID.size() - 1]->size.size(); i++)
    // {
    //     tempArea = BlockID[BlockID.size() - 1]->size[i].first * BlockID[BlockID.size() - 1]->size[i].second;
    //     int longer = max(BlockID[BlockID.size() - 1]->size[i].first, BlockID[BlockID.size() - 1]->size[i].second);
    //     int shorter = min(BlockID[BlockID.size() - 1]->size[i].first, BlockID[BlockID.size() - 1]->size[i].second);
    //     float tempRatio = (float)longer / (float)shorter;
    //     if (tempArea < minArea)
    //     {
    //         minRatio = tempRatio;
    //         minArea = tempArea;
    //         BlockID[BlockID.size() - 1]->WidthNheight = {BlockID[BlockID.size() - 1]->size[i].first, BlockID[BlockID.size() - 1]->size[i].second};
    //     }
    // }
    BlockID[BlockID.size() - 1]->outCorrdiinate = {0, 0};
    // cout << "lower = " << lower << endl;
    // cout << "upper = " << upper << endl;
    // cout << "totArea = " << totArea << endl;

    // std::cout << "minArea = " << minArea << endl;
    TopDown(BlockID, NPE);
    outputFile(BlockID, output, minArea);
    return 0;
}

void outputFile(vector<Block *> &BlockID, ofstream &output, int minArea)
{
    // output.open();
    output << "A = " << minArea << "\n";
    output << "R = " << float(BlockID[BlockID.size() - 1]->WidthNheight.first) / BlockID[BlockID.size() - 1]->WidthNheight.second << "\n";
    for (int i = 0; i < BlockID.size() - 1; i++)
    {
        if (BlockID[i]->name != "V" && BlockID[i]->name != "H")
        {
            // output << "b" << BlockID[i]->name << " " << BlockID[i]->outCorrdiinate.first << " " << BlockID[i]->outCorrdiinate.second;
            output << block.name[stoi(BlockID[i]->name) - 1] << " " << BlockID[i]->outCorrdiinate.first << " " << BlockID[i]->outCorrdiinate.second;
            if (BlockID[i]->WidthNheight.first != BlockID[i]->initWH.first)
            {
                output << " R\n";
            }

            else
            {
                output << "\n";
            }
        }
    }

    output.close();
}

void TopDown(vector<Block *> &BlockID, vector<string> &NPE)
{
    queue<Block *> q;
    q.push(BlockID[BlockID.size() - 1]);
    while (!q.empty())
    {
        bool flag = false;
        Block *temp;
        Block *leftChild;
        Block *rightChild;
        temp = q.front();
        q.pop();

        leftChild = temp->leftChild;
        rightChild = temp->rightChild;

        for (int i = 0; i < leftChild->size.size(); i++)
        {
            if (flag)
                break;
            for (int j = 0; j < rightChild->size.size(); j++)
            {
                if (temp->name == "H")
                {
                    if (leftChild->size[i].second + rightChild->size[j].second == temp->WidthNheight.second && max(leftChild->size[i].first, rightChild->size[j].first) == temp->WidthNheight.first)
                    {
                        leftChild->WidthNheight = leftChild->size[i];
                        rightChild->WidthNheight = rightChild->size[j];
                        leftChild->outCorrdiinate = temp->outCorrdiinate;
                        rightChild->outCorrdiinate.first = leftChild->outCorrdiinate.first;
                        rightChild->outCorrdiinate.second = leftChild->outCorrdiinate.second + leftChild->WidthNheight.second;
                        flag = true;
                        break;
                    }
                }

                else
                {
                    if (leftChild->size[i].first + rightChild->size[j].first == temp->WidthNheight.first && max(leftChild->size[i].second, rightChild->size[j].second) == temp->WidthNheight.second)
                    {
                        leftChild->WidthNheight = leftChild->size[i];
                        rightChild->WidthNheight = rightChild->size[j];
                        leftChild->outCorrdiinate = temp->outCorrdiinate;
                        rightChild->outCorrdiinate.first = leftChild->outCorrdiinate.first + leftChild->WidthNheight.first;
                        rightChild->outCorrdiinate.second = leftChild->outCorrdiinate.second;
                        flag = true;
                        break;
                    }
                }
            }
        }

        if (leftChild->name == "V" || leftChild->name == "H")
            q.push(leftChild);
        if (rightChild->name == "V" || rightChild->name == "H")
            q.push(rightChild);
    }
}

void Delenew(vector<Block *> &BlockID)
{
    int len = BlockID.size();
    for (int i = 0; i < len; i++)
    {
        delete BlockID[i];
    }
}

vector<string> SA(vector<string> &NPE, vector<int> &operatorPosition, vector<int> &operandPosition, float minCost)
{

    int move;
    vector<Block *> bestBlockID;
    float tempCost;
    float T0;
    pair<vector<int>, vector<int>> tempPosition;
    T0 = 30000;
    // cout << "Init T0 = " << T0 << endl;
    float T = T0;

    vector<string> Best;
    vector<string> Cal;
    Best = NPE;
    Cal = NPE;

    bestBlockID = initBlock(Best);
    float BestCost = BottomUp(bestBlockID);

    while (true)
    {
        int totalMove = 0;
        int rejectMove = 0;
        int upHillMove = 0;
        vector<Block *> oldBlock;
        float oldCost;
        // Cal = Best;

        while (true)
        {
            if (oldBlock.empty())
            {
                oldBlock = initBlock(Cal);
                oldCost = BottomUp(oldBlock);
                tempPosition = findPosition(Cal);
                operatorPosition = tempPosition.first;
                operandPosition = tempPosition.second;
            }
            move = rand() % 3;
            switch (move)
            {
            case 0:
                m1(Cal, operandPosition);
                break;
            case 1:
                m2(Cal, operatorPosition);
                break;
            case 2:
                m3(Cal, operatorPosition, operandPosition);
                break;
            }
            totalMove++;
            vector<Block *> newBlock = initBlock(Cal);
            float newCost = BottomUp(newBlock);
            // tempPosition = findPosition(Cal);
            // operatorPosition = tempPosition.first;
            // operandPosition = tempPosition.second;
            float diff;
            diff = newCost - oldCost;
            double R = (double)rand() / RAND_MAX;
            // cout << "newCost = " << newCost << endl;
            // cout << "oldCost = " << BestCost << endl;
            // cout << "bestCost = " << BestCost << endl;

            if (diff < 0 || R < exp(-1 * diff / T))
            {
                if (oldBlock != bestBlockID)
                {
                    Delenew(oldBlock);
                }
                oldBlock = newBlock;
                oldCost = newCost;
                NPE = Cal;
                tempPosition = findPosition(Cal);
                operatorPosition = tempPosition.first;
                operandPosition = tempPosition.second;
                if (diff > 0)
                    upHillMove++;
                if (newCost < BestCost /*&& ratio >= lower && ratio <= upper*/)
                {
                    // cout << "a" << endl;
                    Best = Cal;
                    BestCost = newCost;
                    if (!bestBlockID.empty())
                    {
                        Delenew(bestBlockID);
                    }
                    bestBlockID = newBlock;
                    // Best = Cal;
                }
            }

            else
            {
                Cal = NPE;
                tempPosition = findPosition(Cal);
                operatorPosition = tempPosition.first;
                operandPosition = tempPosition.second;
                Delenew(newBlock);
                rejectMove++;
            }

            if (upHillMove > 10 * Cal.size() || totalMove > 20 * Cal.size())
                break;
        }
        T = (T > 0.05 * T0) ? 0.1 * T : 0.9 * T;
        // if (T <= 30000 && T > 2000)
        //     T = 0.85 * T;
        // else if (T <= 2000 && T > 1000)
        //     T = 0.9 * T;
        // else if (T <= 1000 && T > 500)
        //     T = 0.95 * T;
        // else if (T <= 500 && T > 200)
        //     T = 0.96 * T;
        // else if (T <= 200 && T > 10)
        //     T = 0.97 * T;
        // else if (T <= 10 && T > 0.1)
        //     T = 0.98 * T;
        // cout << "T = " << T << endl;
        if ((double)(rejectMove) / totalMove > 0.95 || T < 0.00001)
            break;
    }
    // cout << "NPE = BEST" << endl;
    NPE = Best;
    return Best;
}

float costFunction(int area, float ratio)
{
    float cost;
    float penalty = (ratio < lower || ratio > upper) ? 1.0 : 0.0;
    // cost = alpha * area / totArea + (1 - alpha) * ((ratio - ratioStar) / ratioStar) * ((ratio - ratioStar) / ratioStar) + (1 - alpha) * penalty;
    cost = 100 * (alpha * (area / totArea - 1) + (1 - alpha) * penalty);
    return cost;
}

void m3(vector<string> &NE, vector<int> &operatorPosition, vector<int> &operandPosition)
{
    bool flag = false;
    int count = 0;
    int choice = 0;
    int num_O_or_O_num;
    vector<string> NE_temp;
    while (true)
    {
        // choice = rand() % operandPosition.size();
        // choice = rand() % operandPosition.size();
        num_O_or_O_num = rand() % 2;

        if (num_O_or_O_num == 0)
        {
            choice = rand() % operandPosition.size();
            int operandCnt = 0;
            int operatorCnt = 0;
            if (choice == operandPosition.size() - 1)
            {
                // NE[operandPosition[choice] + 1];
                for (int i = 0; i <= operandPosition[choice] + 1; i++)
                {
                    if (NE[i] != "H" && NE[i] != "V")
                        operandCnt++;
                    else
                        operatorCnt++;
                }

                if (operandCnt > operatorCnt)
                {
                    if (operandPosition[choice] + 1 != NE.size() - 1)
                    {
                        if (NE[operandPosition[choice] - 1] != NE[operandPosition[choice] + 1])
                        {
                            // cout << "H" << endl;
                            NE_temp = NE;
                            break;
                        }
                    }
                }
            }

            else if (operandPosition[choice + 1] != operandPosition[choice] + 1)
            {
                for (int i = 0; i < operatorPosition.size(); i++)
                {
                    if (operatorPosition[i] == operandPosition[choice] + 1)
                    {
                        if (2 * (i + 1) < operandPosition[choice] + 1)
                        {
                            NE_temp = NE;
                            if (i == 0)
                                flag = true;

                            else if (operatorPosition[i - 1] != operatorPosition[i])
                                flag = true;
                            else
                            {
                                if (NE[operandPosition[choice] - 1] != NE[operandPosition[choice] + 1])
                                    flag = true;
                            }
                        }
                    }

                    if (flag)
                        break;
                }
                if (flag)
                    break;
            }
        }

        else
        {
            choice = rand() % (operatorPosition.size() - 1);

            if (operatorPosition[choice + 1] != operatorPosition[choice] + 1)
            {
                if (operatorPosition[choice + 1] != operatorPosition[choice] + 2)
                {
                    break;
                }

                else
                {
                    if (NE[operatorPosition[choice]] != NE[operatorPosition[choice + 1]])
                    {
                        break;
                    }
                }
            }
        }
        if (count == 30)
            break;
        count++;
    }

    if (count < 30)
    {
        if (num_O_or_O_num == 0)
        {
            // cout << "satisfy balloting" << endl;
            string temp1;
            string temp2;
            temp1 = NE[operandPosition[choice]];
            temp2 = NE[operandPosition[choice] + 1];
            NE[operandPosition[choice]] = temp2;
            NE[operandPosition[choice] + 1] = temp1;
            // cout << "here" << endl;
        }

        else
        {
            string temp1;
            string temp2;
            temp1 = NE[operatorPosition[choice]];
            temp2 = NE[operatorPosition[choice] + 1];
            NE[operatorPosition[choice]] = temp2;
            NE[operatorPosition[choice] + 1] = temp1;
        }
    }
}

void m2(vector<string> &NE, vector<int> &operatorPosition)
{
    int choice = rand() % operatorPosition.size();
    // int choice = 1;
    int leftBound = operatorPosition[choice];
    int rightBound = operatorPosition[choice];

    for (int i = 1; choice + i < operatorPosition.size(); i++)
    {
        if (operatorPosition[choice + i] == operatorPosition[choice] + i)
        {
            rightBound++;
            continue;
        }
        break;
    }

    for (int i = 1; choice - i >= 0; i++)
    {
        if (operatorPosition[choice - i] == operatorPosition[choice] - i)
        {
            leftBound--;
            continue;
        }
        break;
    }

    for (int i = leftBound; i <= rightBound; i++)
    {
        if (NE[i] == "H")
            NE[i] = "V";
        else
            NE[i] = "H";
    }
}

void m1(vector<string> &NE, vector<int> &operandPosition)
{
    int choice = rand() % (operandPosition.size() - 1);
    string temp1, temp2;
    temp1 = NE[operandPosition[choice]];
    temp2 = NE[operandPosition[choice + 1]];
    NE[operandPosition[choice]] = temp2;
    NE[operandPosition[choice + 1]] = temp1;
}

pair<vector<int>, vector<int>> findPosition(vector<string> &NPE)
{
    vector<int> operatorPosition;
    vector<int> operandPosition;
    for (int i = 0; i < NPE.size(); i++)
    {
        if (NPE[i] != "H" && NPE[i] != "V")
        {
            operandPosition.push_back(i);
        }

        else
        {
            operatorPosition.push_back(i);
        }
    }

    return make_pair(operatorPosition, operandPosition);
}

float BottomUp(vector<Block *> &BlockID)
{
    stack<Block *> st;
    for (int i = 0; i < BlockID.size(); i++)
    {
        if (BlockID[i]->name != "H" && BlockID[i]->name != "V")
            st.push(BlockID[i]);
        else
        {
            Block *tempL = st.top();
            st.pop();
            Block *tempR = st.top();
            st.pop();
            SB(tempL, tempR, BlockID[i]);
            st.push(BlockID[i]);
        }
    }

    // int tempArea = 0;
    // float tempRatio = 0;
    // float minCost = INT_MAX;
    // float tempCost = 0;

    // for (int i = 0; i < BlockID[BlockID.size() - 1]->size.size(); i++)
    // {
    //     tempArea = BlockID[BlockID.size() - 1]->size[i].first * BlockID[BlockID.size() - 1]->size[i].second;
    //     tempRatio = float(BlockID[BlockID.size() - 1]->size[i].first) / BlockID[BlockID.size() - 1]->size[i].second;
    //     tempCost = costFunction(tempArea, tempRatio);
    //     if (tempCost < minCost)
    //         minCost = tempCost;
    // }

    Block *a = st.top();
    float minCost = INT_MAX;
    int minArea = INT_MAX;

    for (auto it = a->size.begin(); it != a->size.end(); it++)
    {
        int area = (*it).first * (*it).second;
        int longer = max((*it).first, (*it).second);
        int shorter = min((*it).first, (*it).second);
        float R = (float)longer / (float)shorter;
        // float penalty = (R >= lower && R <= upper) ? 0.0 : 1.0;
        float penalty = (R >= lower && R <= upper) ? 0.0 : 10.0;
        float curCost = alpha * (float)area / (float)totArea + (1 - alpha) * penalty;
        if (curCost < minCost)
        {
            minCost = curCost;
        }
    }

    return minCost;
}

void SB(Block *tempL, Block *tempR, Block *OP)
{
    OP->leftChild = tempR;
    OP->rightChild = tempL;

    if (OP->name == "H")
    {
        auto it1 = tempL->size.rbegin();
        auto it2 = tempR->size.rbegin();

        int w1 = 0;
        int w2 = 0;

        while (it1 != tempL->size.rend() && it2 != tempR->size.rend())
        {
            OP->size.push_back(MergeH((*it1), (*it2)));
            w1 = (*it1).first;
            w2 = (*it2).first;
            if (w1 > w2)
                it1++;
            else if (w2 > w1)
                it2++;
            else
            {
                it1++;
                it2++;
            }
        }

        reverse(OP->size.begin(), OP->size.end());
        // for (int i = 0; i < tempL->size.size(); i++)
        // {
        //     int width = 0;
        //     int height = 0;

        //     int w_left = tempL->size[i].first;
        //     int h_left = tempL->size[i].second;

        //     for (int j = 0; j < tempR->size.size(); j++)
        //     {
        //         int w_right = tempR->size[j].first;
        //         int h_right = tempR->size[j].second;
        //         bool flag = true;

        //         if (w_left > w_right)
        //             width = w_left;
        //         else
        //             width = w_right;

        //         height = h_left + h_right;

        //         for (int k = 0; k < OP->size.size(); k++)
        //         {
        //             if (width >= OP->size[k].first && height >= OP->size[k].second)
        //             {
        //                 flag = false;
        //                 break;
        //             }
        //         }

        //         if (flag)
        //             OP->size.push_back({width, height});
        //     }
        // }
    }

    else
    {
        auto it1 = tempL->size.begin();
        auto it2 = tempR->size.begin();

        int h1 = 0;
        int h2 = 0;

        while (it1 != tempL->size.end() && it2 != tempR->size.end())
        {
            OP->size.push_back(MergeV((*it1), (*it2)));
            int w_1 = (*it1).first;
            int w_2 = (*it2).first;
            if (w_1 > w_2)
                it1++;
            else if (w_2 > w_1)
                it2++;
            else
            { // w_1 == w_2
                it1++;
                it2++;
            }
        }

        // reverse(OP->size.begin(), OP->size.end());
        // for (int i = 0; i < tempL->size.size(); i++)
        // {
        //     int width = 0;
        //     int height = 0;

        //     int w_left = tempL->size[i].first;
        //     int h_left = tempL->size[i].second;

        //     for (int j = 0; j < tempR->size.size(); j++)
        //     {
        //         bool flag = true;
        //         int w_right = tempR->size[j].first;
        //         int h_right = tempR->size[j].second;

        //         if (h_left > h_right)
        //             height = h_left;
        //         else
        //             height = h_right;

        //         width = w_left + w_right;

        //         for (int k = 0; k < OP->size.size(); k++)
        //         {
        //             if (width >= OP->size[k].first && height >= OP->size[k].second)
        //             {
        //                 flag = false;
        //                 break;
        //             }
        //         }

        //         if (flag)
        //             OP->size.push_back({width, height});
        //     }
        // }
    }
}

pair<int, int> MergeV(pair<int, int> const &a, pair<int, int> const &b)
{
    int width = a.first + b.first;
    int height = max(a.second, b.second);
    return {width, height};
}

pair<int, int> MergeH(pair<int, int> const &a, pair<int, int> const &b)
{
    int width = max(a.first, b.first);
    int height = a.second + b.second;
    return {width, height};
}

vector<Block *> initBlock(vector<string> &NPE)
{
    vector<Block *> BlockID;
    // int blockcnt = 0;
    for (int i = 0; i < NPE.size(); i++)
    {
        // cout << blockcnt << endl;
        // cout << "i = " << i << endl;
        Block *temp = new Block;
        temp->name = NPE[i];
        if (NPE[i] != "V" && NPE[i] != "H")
        {
            // cout << "here 2" << endl;
            // cout << "processing " << NPE[i] << endl;
            // cout << "block.size() = " << block.size() << endl;
            // pair<int, int> tempPair = block[stoi(NPE[i]) - 1];
            pair<int, int> tempPair = block.widthHeight[stoi(NPE[i]) - 1];
            // cout << "here" << endl;
            temp->initWH = tempPair;
            if (tempPair.first < tempPair.second)
            {
                temp->size.push_back({tempPair.first, tempPair.second});
                if (tempPair.first != tempPair.second)
                {
                    temp->size.push_back({tempPair.second, tempPair.first});
                }
            }
            else
            {
                temp->size.push_back({tempPair.second, tempPair.first});
                if (tempPair.first != tempPair.second)
                {
                    temp->size.push_back({tempPair.first, tempPair.second});
                }
            }
        }

        BlockID.push_back(temp);
    }

    return BlockID;
}

vector<string> initNPE(int totBlock)
{
    vector<string> NPE;
    NPE.push_back("1");
    NPE.push_back("2");
    NPE.push_back("V");
    for (int i = 3; i <= totBlock; i++)
    {
        NPE.push_back(to_string(i));
        NPE.push_back("V");
    }
    return NPE;
}

int readFile(ifstream &FILE, float &lower, float &upper)
{
    FILE >> lower >> upper;
    string name_index;
    string line;
    int count = 0;
    int width = 0, height = 0;
    FILE.ignore(2, '\n');
    while (getline(FILE, line))
    {
        stringstream ss(line);
        ss >> name_index >> width >> height;
        // name_index.erase(0, 1);
        // if (stoi(name_index) > block.size())
        // {
        //     block.resize(stoi(name_index));
        // }
        block.name.push_back(name_index);
        block.widthHeight.push_back({width, height});
        // block[stoi(name_index) - 1] = {width, height};
        totArea += width * height;
        count++;
    }
    return count;
}