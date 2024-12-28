#include "StockSplit.h"
#include "GetHistoricalData.h"
#include "Matrix.h"
#include "BootStrap.h"
#include "Benchmark.h"
#include "gnuplot_linux.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <string>

using namespace std;
using namespace fre;
string inputEarningFile = "Russell3000EarningsAnnouncements.csv";
string token = "674b5a31d2b182.05658505";

int main()
{

    StockSplit stock_group;
    stock_group.SplitData(inputEarningFile);

    vector<Stock> MissEstimateGroup = stock_group.getMissEstimateGroup();
    vector<Stock> MeetEstimateGroup = stock_group.getMeetEstimateGroup();
    vector<Stock> BeatEstimateGroup = stock_group.getBeatEstimateGroup();

    vector<string> BenchmarkDate;
    vector<double> BenchmarkReturn;
    vector<double> BenchmarkCumReturn;

    // Store stocks and corresponding information for each group in an STL map
    //  convert vector group to map group
    map<string, Stock> MissEstimateGroupMap;
    map<string, Stock> MeetEstimateGroupMap;
    map<string, Stock> BeatEstimateGroupMap;

    char selection;
    map<string, double> historical_benchmark_prices;
    int N = -1;

    // Declare global CAAR variables for plotting
    vector<double> MissCAARAvg, MeetCAARAvg, BeatCAARAvg;
    vector<double> xData;

    while (true)
    {
        cout << "Menu:" << endl;
        cout << "1 - Enter N to retrieve 2N+1 days of historical price data for all stocks" << endl;

        cout << "2 - Pull information for one stock from one group" << endl;

        cout << "3 - Show AAR, AAR-STD, CAAR and CAAR-STD for one group" << endl;
        cout << "4 - Show the gnuplot graph with CAAR for all 3 groups" << endl;
        cout << "5 - Exit the program." << endl
             << endl;

        cout << "Enter selection: ";
        cin >> selection;

        switch (selection)
        {
        case '1':
        {
            cout << "Enter value for N between 40 and 80: ";
            bool input = true;
            while (input)
            {
                cin >> N;
                if (N < 40 || N > 80)
                {
                    cerr << "Error: N must be between 40 and 80." << endl;
                }
                else
                {
                    input = false;
                }
            }
            cout << "Fetching data for all stocks ..." << endl;

            GetIWVData("IWV", "2024-01-01", "2024-12-01", historical_benchmark_prices, token);

            vector<string> IntradeDays = GetDates(historical_benchmark_prices); // Get trading days
            sort(IntradeDays.begin(), IntradeDays.end());

            cout << "Benchmark prices Completed!" << endl;

            GetAllData(MissEstimateGroup, MeetEstimateGroup, BeatEstimateGroup, IntradeDays, N, token);
            cout << "All stock prices completed!" << endl;

            calculateBenchmarkFactors(historical_benchmark_prices, BenchmarkDate, BenchmarkReturn, BenchmarkCumReturn);
            cout << "Benchmark prices Completed!" << endl;

            MissEstimateGroupMap = convertStocksToMap(MissEstimateGroup, "Miss");
            MeetEstimateGroupMap = convertStocksToMap(MeetEstimateGroup, "Meet");
            BeatEstimateGroupMap = convertStocksToMap(BeatEstimateGroup, "Beat");

            break;
        }
        case '2':
        {
            // Pull information for one stock from one group
            string symbol;
            cout << "Enter ticker symbol for detailed information: ";
            cin >> symbol;
            cout << endl;

            auto itMiss = MissEstimateGroupMap.find(symbol);
            auto itMeet = MeetEstimateGroupMap.find(symbol);
            auto itBeat = BeatEstimateGroupMap.find(symbol);

            if (itMiss != MissEstimateGroupMap.end())
            {
                initialAR(itMiss->second, BenchmarkDate, BenchmarkReturn);
                cout << itMiss->second << endl;
            }
            else if (itMeet != MeetEstimateGroupMap.end())
            {
                initialAR(itMeet->second, BenchmarkDate, BenchmarkReturn);
                cout << itMeet->second << endl;
            }
            else if (itBeat != BeatEstimateGroupMap.end())
            {
                initialAR(itBeat->second, BenchmarkDate, BenchmarkReturn);
                cout << itBeat->second << endl;
            }
            else
            {
                cout << "This stock does not have enough data to display, please enter another one" << endl;
            }

            break;
        }
        case '3':
        {
            if (N == -1)
            {
                cout << "Error: You must initialize N in case 1 before using option 3." << endl;
                break;
            }

            int sampleSize = 30; // Number of items to sample in each bootstrap iteration
            int iterations = 40; // Number of bootstrap iterations

            // Perform bootstrap
            Matrix MissAARMatrix;
            Matrix MissCAARMatrix;
            for (int i = 0; i < iterations; ++i)
            {
                vector<Stock> bootstrapSampleResultMiss = bootstrapSample(MissEstimateGroup, sampleSize);
                vector<double> MissAAR(2 * N + 1, 0.0);
                vector<double> MissCAAR(2 * N + 1, 0.0);
                Matrix TempMiss = FindStartDateAndEndDate(N, bootstrapSampleResultMiss, BenchmarkDate, BenchmarkReturn, BenchmarkCumReturn, MissAAR, MissCAAR);
                MissAARMatrix.push_back(TempMiss[0]);
                MissCAARMatrix.push_back(TempMiss[1]);
            }

            vector<double> MissAARAvg = CalculateAvg(MissAARMatrix, N, sampleSize);
            vector<double> MissAARStd = CalculateStd(MissAARMatrix, N, sampleSize);
            MissCAARAvg = CalculateAvg(MissCAARMatrix, N, sampleSize); // Update global variable
            vector<double> MissCAARStd = CalculateStd(MissCAARMatrix, N, sampleSize);

            Matrix MissResult(4);
            MissResult[0] = MissAARAvg;
            MissResult[1] = MissAARStd;
            MissResult[2] = MissCAARAvg;
            MissResult[3] = MissCAARStd;

            Matrix MeetAARMatrix;
            Matrix MeetCAARMatrix;
            for (int i = 0; i < iterations; ++i)
            {
                vector<Stock> bootstrapSampleResultMeet = bootstrapSample(MeetEstimateGroup, sampleSize);
                vector<double> MeetAAR(2 * N + 1, 0.0);
                vector<double> MeetCAAR(2 * N + 1, 0.0);
                Matrix TempMeet = FindStartDateAndEndDate(N, bootstrapSampleResultMeet, BenchmarkDate, BenchmarkReturn, BenchmarkCumReturn, MeetAAR, MeetCAAR);
                MeetAARMatrix.push_back(TempMeet[0]);
                MeetCAARMatrix.push_back(TempMeet[1]);
            }

            vector<double> MeetAARAvg = CalculateAvg(MeetAARMatrix, N, sampleSize);
            vector<double> MeetAARStd = CalculateStd(MeetAARMatrix, N, sampleSize);
            MeetCAARAvg = CalculateAvg(MeetCAARMatrix, N, sampleSize); // Update global variable
            vector<double> MeetCAARStd = CalculateStd(MeetCAARMatrix, N, sampleSize);
            Matrix MeetResult(4);
            MeetResult[0] = MeetAARAvg;
            MeetResult[1] = MeetAARStd;
            MeetResult[2] = MeetCAARAvg;
            MeetResult[3] = MeetCAARStd;

            Matrix BeatAARMatrix;
            Matrix BeatCAARMatrix;
            for (int i = 0; i < iterations; ++i)
            {
                vector<Stock> bootstrapSampleResultBeat = bootstrapSample(BeatEstimateGroup, sampleSize);
                vector<double> BeatAAR(2 * N + 1, 0.0);
                vector<double> BeatCAAR(2 * N + 1, 0.0);
                Matrix TempBeat = FindStartDateAndEndDate(N, bootstrapSampleResultBeat, BenchmarkDate, BenchmarkReturn, BenchmarkCumReturn, BeatAAR, BeatCAAR);
                BeatAARMatrix.push_back(TempBeat[0]);
                BeatCAARMatrix.push_back(TempBeat[1]);
            }

            vector<double> BeatAARAvg = CalculateAvg(BeatAARMatrix, N, sampleSize);
            vector<double> BeatAARStd = CalculateStd(BeatAARMatrix, N, sampleSize);
            BeatCAARAvg = CalculateAvg(BeatCAARMatrix, N, sampleSize); // Update global variable
            vector<double> BeatCAARStd = CalculateStd(BeatCAARMatrix, N, sampleSize);
            Matrix BeatResult(4);
            BeatResult[0] = BeatAARAvg;
            BeatResult[1] = BeatAARStd;
            BeatResult[2] = BeatCAARAvg;
            BeatResult[3] = BeatCAARStd;
            vector<Matrix> AllResult(3);
            AllResult[0] = MissResult;
            AllResult[1] = MeetResult;
            AllResult[2] = BeatResult;

            // Choose one group
            string groupID;
            while (true)
            {
                cout << "Enter group(Beat, Meet, Miss) for detailed information: ";
                cin >> groupID;
                cout << endl;

                if (groupID != "Beat" && groupID != "Meet" && groupID != "Miss")
                {
                    cout << "Wrong group name. Please try again!" << endl;
                }
                else
                {
                    break;
                }
            }

            cout << "Group " << groupID << ":" << endl;

            if (groupID == "Beat")
            {
                cout << "Average Abnormal Return (AAR): " << endl;
                cout << BeatAARAvg << endl;
                cout << "AAR Standard Deviation (AAR-STD): " << endl;
                cout << BeatAARStd << endl;
                cout << "Cumulative Average Abnormal Return (CAAR): " << endl;
                cout << BeatCAARAvg << endl;
                cout << "CAAR Standard Deviation: " << endl;
                cout << BeatCAARStd << endl;
            }
            else if (groupID == "Meet")
            {
                cout << "Average Abnormal Return (AAR): " << endl;
                cout << MeetAARAvg << endl;
                cout << "AAR Standard Deviation (AAR-STD): " << endl;
                cout << MeetAARStd << endl;
                cout << "Cumulative Average Abnormal Return (CAAR): " << endl;
                cout << MeetCAARAvg << endl;
                cout << "CAAR Standard Deviation: " << endl;
                cout << MeetCAARStd << endl;
            }
            else
            {
                cout << "Average Abnormal Return (AAR): " << endl;
                cout << MissAARAvg << endl;
                cout << "AAR Standard Deviation (AAR-STD): " << endl;
                cout << MissAARStd << endl;
                cout << "Cumulative Average Abnormal Return (CAAR): " << endl;
                cout << MissCAARAvg << endl;
                cout << "CAAR Standard Deviation: " << endl;
                cout << MissCAARStd << endl;
            }
            break;
        }

        case '4':
        {
            if (N == -1)
            {
                cout << "Error: You must initialize N in case 1 before using option 4." << endl;
                break;
            }

            if (MissCAARAvg.empty() || MeetCAARAvg.empty() || BeatCAARAvg.empty())
            {
                cout << "Error: You must calculate AAR and CAAR in case 3 beofre using option 4." << endl;
                break;
            }

            xData.resize(2 * N);
            for (int i = 0; i < 2 * N; ++i)
            {
                xData[i] = -N + 1 + i;
            }

            plotCAAR(xData, BeatCAARAvg, MeetCAARAvg, MissCAARAvg); // Use updated global variables
            break;
        }

        case '5':
        {
            return 0;
        }
        }
    }
}