#pragma once
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <cctype>

#include "Matrix.h"

using namespace std;
namespace fre
{
    class Stock
    {
    private:
        string ticker;
        string date; // day "zero" as the earning announced date
        string period_ending;
        double estimate_earning;
        double reported_earning;
        double surprise;
        double surprise_percent;

        Vector adjClosePrices;
        vector<string> dates;
        string group;
        Vector dailyReturn;
        Vector cumDailyReturn;
        Vector AR;

    public:
        // using & to avoid deep copy
        Stock() : ticker(""), date(""), period_ending(""),
                  estimate_earning(0.0), reported_earning(0.0), surprise(0.0),
                  surprise_percent(0.0), adjClosePrices({}), dates({}), group(""),
                  dailyReturn({}), cumDailyReturn({}), AR({}) {}

        Stock(const string &ticker_, const string &date_, const string &period_ending_,
              double estimate_earning_, double reported_earning_, double surprise_,
              double surprise_percent_, const vector<double> &adjClosePrices_,
              const vector<string> &dates_, string group_, const vector<double> &dailyReturn_,
              const vector<double> &cumDailyReturn_,
              const Vector &AR_): ticker(ticker_), date(date_),
                                period_ending(period_ending_), estimate_earning(estimate_earning_),
                                reported_earning(reported_earning_), surprise(surprise_),
                                surprise_percent(surprise_percent_), adjClosePrices(adjClosePrices_),
                                dates(dates_), group(group_), dailyReturn(dailyReturn_),
                                cumDailyReturn(cumDailyReturn_), AR(AR_) {}


        // Mutator functions for setting earning data
        void setEarningData(
            const string &ticker_,
            const string &date_,
            const string &period_ending_,
            double estimate_earning_,
            double reported_earning_,
            double surprise_,
            double surprise_percent_);

        // compare operator
        bool operator<(const Stock &other) const;
        friend ostream &operator<<(ostream &out, const Stock &s);

        // Members Accessor
        string GetTicker() const  { return ticker; }
        string GetDate() const  { return date; }
        string GetPeriod()  const { return period_ending; }
        double GetEstimate()  const { return estimate_earning; }
        double GetReportEarnings()  const { return reported_earning; }
        double GetSuprise() const  { return surprise; }
        double GetSuprisePct()  const { return surprise_percent; }
        string GetGroup() const { return group; }
        vector<string> GetTradingDays()  const { return dates; }
        Vector GetAdjClosePrices() const  { return adjClosePrices; }
        Vector GetDailyReturn() const  { return dailyReturn; }
        Vector GetCumDailyReturn()  const { return cumDailyReturn; }

        Vector CalcCumReturns();
        Vector CalcReturns();

        void SetAdjClosePrices(Vector adjClosePrices_) { adjClosePrices = adjClosePrices_; }
        void SetDates(vector<string> dates_) { dates = dates_; }
        void SetGroup(string Group) { group = Group; }
        void SetDailyReturn(Vector dailyReturn_) { dailyReturn = dailyReturn_; }
        void SetCumDailyReturn(Vector cumDailyReturn_) { cumDailyReturn = cumDailyReturn_; }
        void SetAR(Vector AR_) { AR = AR_; }
    };

    class StockSplit
    {
        // contain information for three groups
    private:
        // Groups of stocks
        vector<Stock> missEstimateGroup;
        vector<Stock> meetEstimateGroup;
        vector<Stock> beatEstimateGroup;

        // Function to read the CSV file
        vector<Stock> readCSV(const string &filename);

    public:
        // Load data and process groups
        bool SplitData(const std::string &filename);

        // Accessors for groups
        const vector<Stock> &getMissEstimateGroup() const;
        const vector<Stock> &getMeetEstimateGroup() const;
        const vector<Stock> &getBeatEstimateGroup() const;
    };

    map<string, Stock> convertStocksToMap(vector<Stock> vectorGroup, string group);
}
