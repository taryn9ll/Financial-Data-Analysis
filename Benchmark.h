#include "StockSplit.h"
#include "GetHistoricalData.h"
#include "Matrix.h"
#include "BootStrap.h"
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
        vector<double> &BenchmarkCumReturn);
    Matrix FindStartDateAndEndDate(int N_, vector<Stock> &Stocks_, vector<string> &BenchmarkDate_, vector<double> &BenchmarkReturn_, vector<double> &BenchmarkCumReturn_,
                                   vector<double> &AR, vector<double> &CAR);
    
    void initialAR(Stock &s, vector<string> &BenchmarkDate_, vector<double> &BenchmarkReturn_);
}