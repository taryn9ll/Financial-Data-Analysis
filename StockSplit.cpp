#include "StockSplit.h"
#include "Benchmark.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <cmath>

#include "Matrix.h"
#include "StockSplit.h"
using namespace std;
namespace fre
{

    // Reads stock data from CSV file and returns a vector of Stock objects
    vector<Stock> StockSplit::readCSV(const string &filename)
    {
        vector<Stock> stocks;
        ifstream file(filename);
        if (!file.is_open())
        {
            cout << "Error: Cannot open file " << filename << endl;
            return stocks;
        }

        string line;
        getline(file, line); // Skip the header

        while (getline(file, line))
        {
            if (line.empty() || all_of(line.begin(), line.end(), [](unsigned char c)
                                       { return isspace(c); }))
            {
                continue; // Skip empty or whitespace-only lines
            }

            stringstream ss(line);
            Stock stock;
            string temp, ticker, date, period_ending;
            double estimate_earning, reported_earning, surprise, surprise_percent;
            getline(ss, ticker, ',');
            getline(ss, date, ',');
            getline(ss, period_ending, ',');

            getline(ss, temp, ',');
            estimate_earning = stod(temp);
            getline(ss, temp, ',');
            reported_earning = stod(temp);
            getline(ss, temp, ',');
            surprise = stod(temp);
            getline(ss, temp, ',');
            surprise_percent = stod(temp);

            stock.setEarningData(ticker, date, period_ending, estimate_earning, reported_earning, surprise, surprise_percent);
            stocks.push_back(stock);
        }
        file.close();
        return stocks;
    }

    bool StockSplit::SplitData(const string &filename)
    {
        vector<Stock> stocks = readCSV(filename);

        if (stocks.empty())
        {
            cerr << "Error: No data loaded from file " << filename << endl;
            return false;
        }

        // Sort by surprise_percent using overloaded operator<
        sort(stocks.begin(), stocks.end());

        // Split into three equivalent size of groups
        size_t groupSize = stocks.size() / 3;

        missEstimateGroup = vector<Stock>(stocks.begin(), stocks.begin() + groupSize);                 // Lowest surprise group: Miss Estimate Group
        meetEstimateGroup = vector<Stock>(stocks.begin() + groupSize, stocks.begin() + 2 * groupSize); // The rest stocks in between Meet Estimate Group
        beatEstimateGroup = vector<Stock>(stocks.begin() + 2 * groupSize, stocks.end());               // Highest surprise group: Beat Estimate Group

        return true;
    }

    const vector<Stock> &StockSplit::getMissEstimateGroup() const
    {
        return missEstimateGroup;
    }

    const vector<Stock> &StockSplit::getMeetEstimateGroup() const
    {
        return meetEstimateGroup;
    }

    const vector<Stock> &StockSplit::getBeatEstimateGroup() const
    {
        return beatEstimateGroup;
    }

    void Stock::setEarningData(const std::string &ticker_, const std::string &date_, const std::string &period_ending_, double estimate_earning_, double reported_earning_, double surprise_, double surprise_percent_)
    {
        ticker = ticker_;
        date = date_;
        period_ending = period_ending_;
        estimate_earning = estimate_earning_;
        reported_earning = reported_earning_;
        surprise = surprise_;
        surprise_percent = surprise_percent_;
    }
    bool Stock::operator<(const Stock &other) const
    {
        return surprise_percent < other.surprise_percent;
    }

    ostream &operator<<(ostream &out, const Stock &s)
    {
        out << "Ticker: " << s.ticker << endl;
        out << "Earning Announcement Date: " << s.date << endl;
        out << "Earning Period Ending: " << s.period_ending << endl;
        out << "Earning Estimate: " << s.estimate_earning << endl;
        out << "Reported Earning Per Share: " << s.reported_earning << endl;
        out << "Earning Suprise " << s.surprise << endl;
        out << "Earning Suprise Percentage: " << s.surprise_percent << endl;
        out << "Group: " << s.group << endl;
        out << "No Enough Daily Price Date: False" << endl;
        out << endl;

        out << "Stock Daily Price Dates:" << endl;
        out << s.dates << endl;

        out << "Stock Daily Price:" << endl;
        out << s.adjClosePrices << endl;

        out << "Stock Cumulative Daily Returns:" << endl;
        out << s.cumDailyReturn << endl;

        out << "Stock Abnormal Daily Returns:" << endl;
        out << s.AR << endl;

        return out;
    }

    map<string, Stock> convertStocksToMap(vector<Stock> vectorGroup, string group)
    {
        map<string, Stock> mapGroup;
        for (vector<Stock>::iterator itr = vectorGroup.begin(); itr != vectorGroup.end(); itr++)
        {
            itr->SetGroup(group);
            mapGroup[itr->GetTicker()] = *itr;
        }
        return mapGroup;
    }
    Vector Stock::CalcReturns()
    {
        Vector calc_returns;
        int size = adjClosePrices.size();
        for (int i = 0; i < size - 1; i++)
        {
            calc_returns.push_back(log(adjClosePrices[i + 1] / adjClosePrices[i]));
        }
        dailyReturn = calc_returns;
        return calc_returns;
    }

    Vector Stock::CalcCumReturns()
    {
        Vector calc_returns = CalcReturns();
        double cumReturn = 0.0;
        int size = calc_returns.size();
        Vector calc_cumReturns(size);
        for (int i = 0; i < size; i++)
        {
            cumReturn = cumReturn + calc_returns[i];
            calc_cumReturns[i] = cumReturn;
        }
        return calc_cumReturns;
    }
}

