#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <time.h>

using namespace std;

class MOS
{
public:
    MOS(){};
    string name;
    string gate;
    string source;
    string drain;
    bool mosType;
    bool sourceConnected = false;
    bool drainConnected = false;
    bool DGS = false;
    bool scanned = false;
    double width;
    double length;
};

class Placement
{
public:
    void readFile(ifstream &input);
    void seperateMos();
    void init_SA_sequence();
    void findDummyPosition(vector<MOS> &subMosArray, vector<int> &dummyPosArray);
    void findDiffusionPosition();
    void calHPWL();
    void outputFile(ofstream &output);
    void clearAll();
    void SA();
    void m1();
    void m2();
    vector<MOS> mosArray;
    vector<MOS> nMosArray;
    vector<MOS> pMosArray;
    vector<int> SA_sequence;
    vector<int> p_with_dummy_posArray;
    vector<int> n_with_dummy_posArray;
    map<string, set<int>> netPosition;
    vector<string> p_outSequence;
    vector<string> n_outSequence;
    set<int> dummyPosition;
    map<string, pair<bool, bool>> twoSide;
    double HPWL = 0;
    int DiffusionCnt = 0;
    int diffLastIndex = 0;
    vector<string> best_p_outSequence;
    vector<string> best_n_outSequence;
    vector<int> best_SA_sequence;
    set<int> best_dummyPosition;
    vector<int> temp_good_SA_sequence;
    double best_HPWL = 0;
    // map<string, int> temp;
};

double PmosWidth = 0;
double NmosWidth = 0;
double MOScnt = 0;
double netCnt = 0;

int main(int argc, char *argv[])
{
    // srand(810546);
    clock_t start, end;
    start = clock();
    srand(time(NULL));
    // ifstream input("case5.sp");
    // ofstream output("output4.txt");
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    Placement P;
    P.readFile(input);
    P.seperateMos();
    P.init_SA_sequence();
    P.findDummyPosition(P.pMosArray, P.p_with_dummy_posArray);
    P.findDummyPosition(P.nMosArray, P.n_with_dummy_posArray);
    P.findDiffusionPosition();
    PmosWidth = P.pMosArray[1].width;
    NmosWidth = P.nMosArray[1].width;
    P.diffLastIndex = P.DiffusionCnt - 1;
    MOScnt = P.mosArray.size() / 2;
    netCnt = P.netPosition.size() / 2;
    // cout << "MOScnt = " << MOScnt << endl;
    // cout << "netCnt = " << netCnt << endl;
    P.calHPWL();
    P.SA();
    P.outputFile(output);
    end = clock();
    // cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;
    return 0;
}

void Placement::m1()
{
    int choice1 = rand() % (SA_sequence.size());
    int choice2 = rand() % (SA_sequence.size());
    while (choice1 == choice2)
    {
        choice2 = rand() % (SA_sequence.size());
    }
    int temp1, temp2;
    temp1 = SA_sequence[choice1];
    temp2 = SA_sequence[choice2];
    SA_sequence[choice1] = temp2;
    SA_sequence[choice2] = temp1;
}

void Placement::m2()
{
    reverse(SA_sequence.begin(), SA_sequence.end());
}

void Placement::SA()
{
    double T0 = 30000;
    double T = T0;
    int totalMove = 0;
    int move = 0;
    double diff = 0;
    int uphill = 0;
    best_HPWL = HPWL;
    best_p_outSequence = p_outSequence;
    best_n_outSequence = n_outSequence;
    best_SA_sequence = SA_sequence;
    best_dummyPosition = dummyPosition;
    temp_good_SA_sequence = SA_sequence;
    double prev_HPWL;
    prev_HPWL = HPWL;
    vector<int> temp_upswap_SA_sequence;
    clearAll();

    while (true)
    {
        // break;
        int totalMove = 0;
        // cout << "T = " << T << endl;
        while (true)
        {
            // move = rand() % 2;
            move = 0;
            temp_upswap_SA_sequence = SA_sequence;
            switch (move)
            {
            case 0:
                m1();
                break;
                // case 1:
                //     m2();
                //     break;
            }
            findDummyPosition(pMosArray, p_with_dummy_posArray);
            findDummyPosition(nMosArray, n_with_dummy_posArray);
            findDiffusionPosition();
            diffLastIndex = DiffusionCnt - 1;
            calHPWL();
            double R = (double)rand() / (RAND_MAX + 1.0);
            // diff = (HPWL - prev_HPWL) / (netCnt * MOScnt);
            diff = (HPWL - best_HPWL) / (netCnt * MOScnt);

            if (diff < 0 || R < exp(-1 * diff / T))
            {
                // temp_good_SA_sequence = SA_sequence;
                if (HPWL < best_HPWL)
                {
                    best_HPWL = HPWL;
                    best_p_outSequence = p_outSequence;
                    best_n_outSequence = n_outSequence;
                    best_SA_sequence = SA_sequence;
                    best_dummyPosition = dummyPosition;
                    // cout << "curBestHPWL = " << best_HPWL << endl;
                }
                else
                {
                    uphill++;
                }
            }

            else
            {
                SA_sequence = temp_upswap_SA_sequence;
            }
            totalMove++;
            prev_HPWL = HPWL;
            clearAll();
            if (totalMove > 500)
                break;
        }
        SA_sequence = best_SA_sequence;
        if (T <= 30000 && T > 1000)
            T = 0.9 * T;
        else if (T <= 1000 && T > 100)
            T = 0.9 * T;
        else if (T <= 100 && T > 10)
            T = 0.98 * T;
        else if (T <= 10 && T > 0.1)
            T = 0.99 * T;
        // cout << "uphill = " << uphill << endl;
        uphill = 0;
        if (T < 0.1)
            break;
    }
}

void Placement::clearAll()
{
    p_with_dummy_posArray.clear();
    n_with_dummy_posArray.clear();
    netPosition.clear();
    p_outSequence.clear();
    n_outSequence.clear();
    dummyPosition.clear();
    HPWL = 0;
    DiffusionCnt = 0;
    for (int i = 0; i < pMosArray.size(); i++)
    {
        pMosArray[i].DGS = false;
        pMosArray[i].drainConnected = false;
        pMosArray[i].sourceConnected = false;
    }
    for (int i = 0; i < nMosArray.size(); i++)
    {
        nMosArray[i].DGS = false;
        nMosArray[i].drainConnected = false;
        nMosArray[i].sourceConnected = false;
    }
}

void Placement::outputFile(ofstream &output)
{
    output << best_HPWL << endl;
    // cout << "HPWL = " << best_HPWL << endl;
    int i = 0;
    for (auto it = best_dummyPosition.begin(); it != best_dummyPosition.end(); ++it)
    {
        for (; i <= *it; i++)
        {
            output << pMosArray[best_SA_sequence[i]].name.erase(0, 1) << " ";
        }
        output << "Dummy ";
    }
    for (; i < best_SA_sequence.size(); i++)
    {
        output << pMosArray[best_SA_sequence[i]].name.erase(0, 1) << " ";
    }
    output << endl;
    for (int j = 0; j < best_p_outSequence.size(); j++)
    {
        output << best_p_outSequence[j] << " ";
    }
    output << endl;
    i = 0;
    for (auto it = best_dummyPosition.begin(); it != best_dummyPosition.end(); ++it)
    {
        for (; i <= *it; i++)
        {
            output << nMosArray[best_SA_sequence[i]].name.erase(0, 1) << " ";
        }
        output << "Dummy ";
    }
    for (; i < best_SA_sequence.size(); i++)
    {
        output << nMosArray[best_SA_sequence[i]].name.erase(0, 1) << " ";
    }
    output << endl;
    for (int j = 0; j < best_n_outSequence.size(); j++)
    {
        output << best_n_outSequence[j] << " ";
    }
}

void Placement::calHPWL()
{
    for (const auto &s : netPosition)
    {
        double x = 0, y = 0;
        int left = *(s.second.begin());
        int right = *(--s.second.end());
        if (left == 0 && right == diffLastIndex)
        {
            HPWL += 25 * 0.5 + 25 * 0.5 + (right - left - 1) * 34 + (right - left) * 20;
            x = 25 * 0.5 + 25 * 0.5 + (right - left - 1) * 34 + (right - left) * 20;
        }
        else if (left == 0 || right == diffLastIndex)
        {
            HPWL += 25 * 0.5 + 34 * 0.5 + (right - left - 1) * 34 + (right - left) * 20;
            x = 25 * 0.5 + 34 * 0.5 + (right - left - 1) * 34 + (right - left) * 20;
        }
        else if (left == right)
        {
            HPWL += 0;
            x = 0;
        }
        else
        {
            HPWL += 34 * 0.5 + 34 * 0.5 + (right - left - 1) * 34 + (right - left) * 20;
            x = 34 * 0.5 + 34 * 0.5 + (right - left - 1) * 34 + (right - left) * 20;
        }

        if (twoSide[s.first].first && twoSide[s.first].second)
        {
            HPWL += 27 + PmosWidth / 2 + NmosWidth / 2;
            y = 27 + PmosWidth / 2 + NmosWidth / 2;
        }
        // temp[s.first] = x + y;
    }
}

void Placement::findDiffusionPosition()
{
    int position = 0;
    int i = 0;
    for (auto it = dummyPosition.begin(); it != dummyPosition.end(); ++it)
    {
        for (; i <= *it; i++)
        {
            if (pMosArray[SA_sequence[i]].DGS)
            {
                p_outSequence.push_back(pMosArray[SA_sequence[i]].drain);
                p_outSequence.push_back(pMosArray[SA_sequence[i]].gate);
                netPosition[pMosArray[SA_sequence[i]].drain].insert(position);
            }
            else
            {
                p_outSequence.push_back(pMosArray[SA_sequence[i]].source);
                p_outSequence.push_back(pMosArray[SA_sequence[i]].gate);
                netPosition[pMosArray[SA_sequence[i]].source].insert(position);
            }

            if (nMosArray[SA_sequence[i]].DGS)
            {
                n_outSequence.push_back(nMosArray[SA_sequence[i]].drain);
                n_outSequence.push_back(nMosArray[SA_sequence[i]].gate);
                netPosition[nMosArray[SA_sequence[i]].drain].insert(position);
            }
            else
            {
                n_outSequence.push_back(nMosArray[SA_sequence[i]].source);
                n_outSequence.push_back(nMosArray[SA_sequence[i]].gate);
                netPosition[nMosArray[SA_sequence[i]].source].insert(position);
            }

            DiffusionCnt++;
            position++;
        }
        if (pMosArray[SA_sequence[i - 1]].DGS)
        {
            p_outSequence.push_back(pMosArray[SA_sequence[i - 1]].source);
            netPosition[pMosArray[SA_sequence[i - 1]].source].insert(position);
        }
        else
        {
            p_outSequence.push_back(pMosArray[SA_sequence[i - 1]].drain);
            netPosition[pMosArray[SA_sequence[i - 1]].drain].insert(position);
        }
        if (nMosArray[SA_sequence[i - 1]].DGS)
        {
            n_outSequence.push_back(nMosArray[SA_sequence[i - 1]].source);
            netPosition[nMosArray[SA_sequence[i - 1]].source].insert(position);
        }
        else
        {
            n_outSequence.push_back(nMosArray[SA_sequence[i - 1]].drain);
            netPosition[nMosArray[SA_sequence[i - 1]].drain].insert(position);
        }
        DiffusionCnt++;
        position++;
        p_outSequence.push_back("Dummy");
        n_outSequence.push_back("Dummy");
        position++;
        DiffusionCnt++;
    }
    for (; i < SA_sequence.size() - 1; i++)
    {
        if (pMosArray[SA_sequence[i]].DGS)
        {
            p_outSequence.push_back(pMosArray[SA_sequence[i]].drain);
            p_outSequence.push_back(pMosArray[SA_sequence[i]].gate);
            netPosition[pMosArray[SA_sequence[i]].drain].insert(position);
        }

        else
        {
            p_outSequence.push_back(pMosArray[SA_sequence[i]].source);
            p_outSequence.push_back(pMosArray[SA_sequence[i]].gate);
            netPosition[pMosArray[SA_sequence[i]].source].insert(position);
        }

        if (nMosArray[SA_sequence[i]].DGS)
        {
            n_outSequence.push_back(nMosArray[SA_sequence[i]].drain);
            n_outSequence.push_back(nMosArray[SA_sequence[i]].gate);
            netPosition[nMosArray[SA_sequence[i]].drain].insert(position);
        }

        else
        {
            n_outSequence.push_back(nMosArray[SA_sequence[i]].source);
            n_outSequence.push_back(nMosArray[SA_sequence[i]].gate);
            netPosition[nMosArray[SA_sequence[i]].source].insert(position);
        }
        position++;
        DiffusionCnt++;
    }

    if (pMosArray[SA_sequence[SA_sequence.size() - 1]].DGS)
    {
        p_outSequence.push_back(pMosArray[SA_sequence[SA_sequence.size() - 1]].drain);
        p_outSequence.push_back(pMosArray[SA_sequence[SA_sequence.size() - 1]].gate);
        p_outSequence.push_back(pMosArray[SA_sequence[SA_sequence.size() - 1]].source);
        netPosition[pMosArray[SA_sequence[SA_sequence.size() - 1]].drain].insert(position);
        DiffusionCnt++;
        position++;
        netPosition[pMosArray[SA_sequence[SA_sequence.size() - 1]].source].insert(position);
        DiffusionCnt++;
    }
    else
    {
        p_outSequence.push_back(pMosArray[SA_sequence[SA_sequence.size() - 1]].source);
        p_outSequence.push_back(pMosArray[SA_sequence[SA_sequence.size() - 1]].gate);
        p_outSequence.push_back(pMosArray[SA_sequence[SA_sequence.size() - 1]].drain);
        netPosition[pMosArray[SA_sequence[SA_sequence.size() - 1]].source].insert(position);
        DiffusionCnt++;
        position++;
        netPosition[pMosArray[SA_sequence[SA_sequence.size() - 1]].drain].insert(position);
        DiffusionCnt++;
    }
    position--;
    if (nMosArray[SA_sequence[SA_sequence.size() - 1]].DGS)
    {
        n_outSequence.push_back(nMosArray[SA_sequence[SA_sequence.size() - 1]].drain);
        n_outSequence.push_back(nMosArray[SA_sequence[SA_sequence.size() - 1]].gate);
        n_outSequence.push_back(nMosArray[SA_sequence[SA_sequence.size() - 1]].source);
        netPosition[nMosArray[SA_sequence[SA_sequence.size() - 1]].drain].insert(position);
        position++;
        netPosition[nMosArray[SA_sequence[SA_sequence.size() - 1]].source].insert(position);
    }
    else
    {
        n_outSequence.push_back(nMosArray[SA_sequence[SA_sequence.size() - 1]].source);
        n_outSequence.push_back(nMosArray[SA_sequence[SA_sequence.size() - 1]].gate);
        n_outSequence.push_back(nMosArray[SA_sequence[SA_sequence.size() - 1]].drain);
        netPosition[nMosArray[SA_sequence[SA_sequence.size() - 1]].source].insert(position);
        position++;
        netPosition[nMosArray[SA_sequence[SA_sequence.size() - 1]].drain].insert(position);
    }
}

void Placement::findDummyPosition(vector<MOS> &subMosArray, vector<int> &dummyPosArray)
{
    dummyPosArray.push_back(SA_sequence[0]);
    for (int i = 0; i < SA_sequence.size() - 1; i++)
    {
        if (subMosArray[SA_sequence[i]].source == subMosArray[SA_sequence[i + 1]].source && !(subMosArray[SA_sequence[i]].sourceConnected) && !(subMosArray[SA_sequence[i + 1]].sourceConnected))
        {
            subMosArray[SA_sequence[i + 1]].sourceConnected = true;
            subMosArray[SA_sequence[i]].DGS = true;
            subMosArray[SA_sequence[i + 1]].DGS = false;
            dummyPosArray.push_back(SA_sequence[i + 1]);
        }

        else if (subMosArray[SA_sequence[i]].source == subMosArray[SA_sequence[i + 1]].drain && !(subMosArray[SA_sequence[i]].sourceConnected) && !(subMosArray[SA_sequence[i + 1]].drainConnected))
        {
            subMosArray[SA_sequence[i + 1]].drainConnected = true;
            subMosArray[SA_sequence[i]].DGS = true;
            subMosArray[SA_sequence[i + 1]].DGS = true;
            dummyPosArray.push_back(SA_sequence[i + 1]);
        }

        else if (subMosArray[SA_sequence[i]].drain == subMosArray[SA_sequence[i + 1]].source && !(subMosArray[SA_sequence[i]].drainConnected) && !(subMosArray[SA_sequence[i + 1]].sourceConnected))
        {
            subMosArray[SA_sequence[i + 1]].sourceConnected = true;
            subMosArray[SA_sequence[i]].DGS = false;
            subMosArray[SA_sequence[i + 1]].DGS = false;
            dummyPosArray.push_back(SA_sequence[i + 1]);
        }

        else if (subMosArray[SA_sequence[i]].drain == subMosArray[SA_sequence[i + 1]].drain && !(subMosArray[SA_sequence[i]].drainConnected) && !(subMosArray[SA_sequence[i + 1]].drainConnected))
        {
            subMosArray[SA_sequence[i + 1]].drainConnected = true;
            subMosArray[SA_sequence[i]].DGS = false;
            subMosArray[SA_sequence[i + 1]].DGS = true;
            dummyPosArray.push_back(SA_sequence[i + 1]);
        }

        else
        {
            dummyPosArray.push_back(0);
            dummyPosArray.push_back(SA_sequence[i + 1]);
            dummyPosition.insert(i);
        }
    }
}

void Placement::init_SA_sequence()
{
    for (int i = 1; i < nMosArray.size(); i++)
    {
        SA_sequence.push_back(i);
    }
    // SA_sequence.push_back(1);
    // SA_sequence.push_back(3);
    // SA_sequence.push_back(4);
    // SA_sequence.push_back(2);
    // SA_sequence.push_back(5);
    // SA_sequence.push_back(6);
    // SA_sequence.push_back(8);
    // SA_sequence.push_back(7);
    // SA_sequence.push_back(9);
    // SA_sequence.push_back(10);
    // SA_sequence.push_back(12);
    // SA_sequence.push_back(11);
    // SA_sequence.push_back(14);
    // SA_sequence.push_back(13);
    // SA_sequence.push_back(15);
    // SA_sequence.push_back(16);
}

void Placement::seperateMos()
{
    MOS dummy;
    dummy.name = "dummy";
    pMosArray.push_back(dummy);
    nMosArray.push_back(dummy);
    for (int i = 0; i < mosArray.size(); i++)
    {
        if (!mosArray[i].scanned)
        {
            if (mosArray[i].mosType)
            {
                pMosArray.push_back(mosArray[i]);
                twoSide[mosArray[i].source].first = true;
                twoSide[mosArray[i].drain].first = true;

                for (int j = i + 1; j < mosArray.size(); j++)
                {
                    if (!mosArray[j].mosType && !mosArray[j].scanned)
                    {
                        if (mosArray[i].gate == mosArray[j].gate)
                        {
                            mosArray[j].scanned = true;
                            nMosArray.push_back(mosArray[j]);
                            twoSide[mosArray[j].source].second = true;
                            twoSide[mosArray[j].drain].second = true;
                            break;
                        }
                    }
                }
            }

            else
            {
                nMosArray.push_back(mosArray[i]);
                twoSide[mosArray[i].source].second = true;
                twoSide[mosArray[i].drain].second = true;
                for (int j = i + 1; j < mosArray.size(); j++)
                {
                    if (mosArray[j].mosType && !mosArray[j].scanned)
                    {
                        if (mosArray[i].gate == mosArray[j].gate)
                        {
                            mosArray[j].scanned = true;
                            pMosArray.push_back(mosArray[j]);
                            twoSide[mosArray[j].source].first = true;
                            twoSide[mosArray[j].drain].first = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void Placement::readFile(ifstream &input)
{
    string line;
    getline(input, line);
    while (getline(input, line))
    {
        if (line == ".ENDS")
            break;
        MOS mos;
        stringstream ss(line);
        string temp;
        ss >> mos.name >> mos.drain >> mos.gate >> mos.source;
        ss >> temp >> temp;
        if (temp == "nmos_rvt")
            mos.mosType = 0;
        else
            mos.mosType = 1;
        ss >> temp;
        temp.erase(0, 2);
        temp.erase(temp.length() - 1, 1);
        mos.width = stod(temp);
        ss >> temp;
        temp.erase(0, 2);
        temp.erase(temp.length() - 1, 1);
        mos.length = stod(temp);
        mosArray.push_back(mos);
    }
}
