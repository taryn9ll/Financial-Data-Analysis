#include "StockSplit.h"
#include "GetHistoricalData.h"
#include "Matrix.h"
#include "BootStrap.h"
#include <iostream>
#include <iomanip>
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
    // Random selecting 30 stocks from each group, sampleSize defined in main
    vector<Stock> bootstrapSample(const vector<Stock> &input, int sampleSize)
    {
        vector<Stock> sample;
        random_device rd;                                     // Seed for the random number generator
        mt19937 gen(rd());                                    // Random number generator
        uniform_int_distribution<> dist(0, input.size() - 1); // Distribution over input indices

        for (int i = 0; i < sampleSize; ++i)
        {
            int randomIndex = dist(gen);          // Generate random index with replacement
            sample.push_back(input[randomIndex]); // Add the randomly selected item
        }

        return sample;
    }

    Vector CalculateAvg(Matrix &M, int N, int sampleSize) // N is the number of days
    {
        Vector Avg;
        int length = 2 * N;
        for (int i = 0; i < length; i++)
        {
            double sum = 0.0;
            for (int j = 0; j < sampleSize; j++)
            {
                sum += M[j][i];
            }
            Avg.push_back(sum / sampleSize);
        }

        return Avg;
    }

    Vector CalculateStd(Matrix &M, int N, int sampleSize)
    {
        int length = 2 * N;
        Vector Std(length);
        for (int i = 0; i < length; i++)
        {
            Std[i] = 0.0;
        }
        for (Matrix ::iterator itr = M.begin(); itr != M.end(); itr++)
        {
            Std = Std + *itr * *itr;
        }
        Vector Avg = CalculateAvg(M, N, sampleSize);

        for (int i = 0; i < length; i++)
        {
            Std[i] = Std[i] / sampleSize;
        }
        Std = Std - Avg * Avg;
        for (int i = 0; i < length; i++)
        {
            Std[i] = sqrt(Std[i] / (sampleSize - 1));
        }

        return Std;
    }

}
