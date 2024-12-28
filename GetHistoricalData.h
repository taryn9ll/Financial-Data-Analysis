#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <curl/curl.h>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <algorithm>
#include <cmath>
#include <chrono>

#include "StockSplit.h"
#include "Matrix.h"

using namespace std;
namespace fre
{
    struct MemoryStruct
    {
        char *memory;
        size_t size;
    };

    struct DataRow
    {
        string date;
        double prevClose;
        double close;
        double dailyReturn;
    };

    //
    class ProgressBar
    {
    public:
        ProgressBar(int total, int barWidth = 50)
            : totalTasks(total), barWidth(barWidth), currentProgress(0) {}

        // Updates the progress bar with the current progress value
        void update(int progress)
        {
            currentProgress = progress;
            display();
        }

        void finish()
        {
            currentProgress = totalTasks;
            display();
            std::cout << std::endl; // Move to the next line after completion
        }

    private:
        int totalTasks;      // Total number of tasks to be completed
        int barWidth;        // Width of the progress bar (number of characters)
        int currentProgress; // Current progress value

        void display()
        {
            float percentage = static_cast<float>(currentProgress) / totalTasks; // Calculate completion percentage
            int pos = static_cast<int>(barWidth * percentage);                   // Calculate position of the progress marker

            std::cout << "\r[";
            for (int i = 0; i < barWidth; ++i)
            {
                if (i < pos)
                    std::cout << "=";
                else if (i == pos)
                    std::cout << ">"; // Progress marker
                else
                    std::cout << " ";
            }
            std::cout << "] " << static_cast<int>(percentage * 100) << "% (" << currentProgress << "/" << totalTasks << ")";
            std::cout.flush();
        }
    };

    void *myrealloc(void *ptr, size_t size);
    int write_data2(void *ptr, size_t size, size_t nmemb, void *data);

    string trim(const string &str);

    vector<string> GetDates(map<string, double> PricePath);
    vector<double> GetPrices(map<string, double> pricepath);

    void GetsStartAndEndDate(string &startDate, string &endDate, vector<string> IntradeDays, string date0, int N);

    //Retrieve data functions
    int GetGroupData(vector<Stock> &tickers, vector<string> IntradeDays, int N, string api_token, std::atomic<int> &progress, ProgressBar bar);
    int GetIWVData(string ticker, string start_date, string end_date, map<string, double> &historical_price, string api_token);
    void GetAllData(vector<Stock> &missTickers, vector<Stock> &meetTickers, vector<Stock> &beatTickers, vector<string> IntradeDays, int N, string api_token);
}
