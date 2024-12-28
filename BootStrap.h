#include "StockSplit.h"
#include "GetHistoricalData.h"
#include "Matrix.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <string>
using namespace std;
using namespace fre;
namespace fre
{
    vector<Stock> bootstrapSample(const vector<Stock> &input, int sampleSize);
    Vector CalculateAvg(Matrix &M, int N, int sampleSize);
    Vector CalculateStd(Matrix &M, int N, int sampleSize);
}
