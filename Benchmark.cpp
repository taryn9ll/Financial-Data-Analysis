#include "StockSplit.h"
#include "GetHistoricalData.h"
#include "Matrix.h"
#include "BootStrap.h"
#include "Benchmark.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <string>
#include <random>

using namespace std;
using namespace fre;
namespace fre
{

    void calculateBenchmarkFactors(
        const map<string, double> &historical_benchmark_prices,
        vector<string> &BenchmarkDate,
        vector<double> &BenchmarkReturn,
        vector<double> &BenchmarkCumReturn)
    {
        BenchmarkDate.clear();
        BenchmarkReturn.clear();
        BenchmarkCumReturn.clear();

        double prevPrice = -1.0;
        double cumReturn = 0.0;

        for (const auto &pair : historical_benchmark_prices)
        {
            const auto &date = pair.first;
            const auto &price = pair.second;

            BenchmarkDate.push_back(date);

            if (prevPrice < 0)
            {

                BenchmarkReturn.push_back(0.0);
            }
            else
            {

                double logReturn = std::log(price / prevPrice);

                BenchmarkReturn.push_back(logReturn);
            }

            cumReturn += BenchmarkReturn.back();
            BenchmarkCumReturn.push_back(cumReturn);

            prevPrice = price;
        }
    }

    // Calculate Average AR & CAR
    Matrix FindStartDateAndEndDate(int N_, vector<Stock> &Stocks_, vector<string> &BenchmarkDate_, vector<double> &BenchmarkReturn_, vector<double> &BenchmarkCumReturn_,
                                   vector<double> &AR, vector<double> &CAR)
    {
        string firstDate = "";

        // Iterate through each stock
        for (unsigned i = 0; i < Stocks_.size(); i++)
        {
            // Get the dates from the stock
            firstDate = Stocks_[i].GetTradingDays().front();

            int firstDateIndex;

            // Find the index of the firstDate in the benchmark's date list
            for (unsigned j = 0; j < BenchmarkDate_.size(); j++)
            {
                if (BenchmarkDate_[j] == firstDate)
                    firstDateIndex = j;
            }

            // Calculate abnormal returns (AR) and cumulative abnormal returns (CAR)
            for (size_t k = 0; k < AR.size(); ++k)
            {
                AR[k] += Stocks_[i].GetDailyReturn()[k] - BenchmarkReturn_[k + firstDateIndex];
                CAR[k] += Stocks_[i].GetCumDailyReturn()[k] - BenchmarkCumReturn_[k + firstDateIndex];
            }
        }

        for (size_t k = 0; k < AR.size(); ++k)
        {
            AR[k] /= Stocks_.size();
            CAR[k] /= Stocks_.size();
        }

        return Matrix({AR, CAR});
    }

    void initialAR(Stock &s, vector<string> &BenchmarkDate_, vector<double> &BenchmarkReturn_)
    {
        string firstDate = "";
        Vector single_AR;
        int N = s.GetDailyReturn().size();
        firstDate = s.GetTradingDays().front();

        auto it = find(BenchmarkDate_.begin(), BenchmarkDate_.end(), firstDate);
        size_t index = distance(BenchmarkDate_.begin(), it);
        size_t end = index + N;
        Vector subBenchmarkReturn(BenchmarkReturn_.begin() + index, BenchmarkReturn_.begin() + end);
        single_AR = s.GetDailyReturn() - subBenchmarkReturn;
        s.SetAR(single_AR);
    }
}
