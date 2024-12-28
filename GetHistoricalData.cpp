#include "StockSplit.h"
#include "Matrix.h"
#include "GetHistoricalData.h"

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
#include <atomic>
#include <chrono>

using namespace std;
namespace fre
{
    void *myrealloc(void *ptr, size_t size)
    {
        if (ptr)
            return realloc(ptr, size); // resizes the memory block pointed to by ptr to size bytes
        else
            return malloc(size); // allocate a new block of size size bytes
    }

    int write_data2(void *ptr, size_t size, size_t nmemb, void *data)
    {
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *)data;
        mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1); // dynamic allocation
        if (mem->memory)
        {
            memcpy(&(mem->memory[mem->size]), ptr, realsize);
            mem->size += realsize;
            mem->memory[mem->size] = 0;
        }
        return realsize;
    }

    vector<string> GetDates(map<string, double> PricePath)
    {
        vector<string> Dates;
        for (map<string, double>::iterator itr = PricePath.begin(); itr != PricePath.end(); itr++)
        {
            Dates.push_back(itr->first);
        }
        return Dates;
    }

    Vector GetPrices(map<string, double> pricepath)
    {
        vector<double> prices;
        for (auto itr = pricepath.begin(); itr != pricepath.end(); itr++)
        {
            prices.push_back(itr->second);
        }
        return prices;
    }

    string trim(const string &str)
    {
        size_t first = str.find_first_not_of(" \t\n\r");
        size_t last = str.find_last_not_of(" \t\n\r");
        return (first == string::npos) ? "" : str.substr(first, last - first + 1);
    }

    void GetsStartAndEndDate(string &startDate, string &endDate, vector<string> IntradeDays, string date0, int N)
    {

        auto lower = lower_bound(IntradeDays.begin(), IntradeDays.end(), date0);
        if (lower == IntradeDays.end())
        {
            cout << endl;
            cout << "Date0 isn't a trading day: " << date0 << endl;
            cout << "No valid closest date found." << endl;
        }
        else
        {
            if (*lower != date0)
            {
                cout << endl;
                cout << "Date0 isn't a trading day: " << date0 << endl;
                cout << "The closest trading date to " << date0 << " is: " << *lower << endl;
            }

            // Check if we have enough data to move N days backward
            if (distance(IntradeDays.begin(), lower) < N)
            {
                cout << endl;
                cout << "No enough data before " << date0 << endl;
            }
            else if (distance(lower, IntradeDays.end()) <= N)
            {
                cout << endl;
                cout << "No enough data after " << date0 << endl;
            }
            else
            {
                // Move N days backward
                advance(lower, -N);
                startDate = *lower;
                // Move N days forward
                lower += 2 * N;
                endDate = *lower;
            }
        }
    }

    // Retrieve adj closed price data for each group in Russell 3000 stocks
    int GetGroupData(vector<Stock> &tickers, vector<string> IntradeDays, int N, string api_token, std::atomic<int> &progress, ProgressBar bar)
    {
        CURL *handle;
        CURLcode status;

        // Set up the program environment that libcurl needs
        curl_global_init(CURL_GLOBAL_ALL);
        handle = curl_easy_init(); // curl_easy_init() returns a CURL easy handle

        if (!handle)
        {
            cout << "Curl init failed!" << endl;
            return -1;
        }

        string url_common = "https://eodhistoricaldata.com/api/eod/";

        for (vector<Stock>::iterator itr = tickers.begin(); itr != tickers.end();)
        {
            string Symbol = trim(itr->GetTicker());

            if (Symbol.empty())
            {
                // Remove tickers with empty symbols
                itr = tickers.erase(itr);
            }
            else
            {
                string date0 = itr->GetDate(); // Get the earning announcment date for the ticker
                string StartDate = "Less", EndDate = "Less";
                GetsStartAndEndDate(StartDate, EndDate, IntradeDays, date0, N);
                if (StartDate == "Less" || EndDate == "Less")
                {
                     // Warn if not enough data and remove the ticker
                    cout << endl;
                    cout << "Warning: " << itr->GetTicker() << " doesn't have enough data" << endl;
                    itr = tickers.erase(itr);
                    continue;
                }
                else
                {

                    struct MemoryStruct data;
                    data.memory = NULL;
                    data.size = 0;

                    string url_request = url_common + itr->GetTicker() + ".US?" + "from=" + StartDate + "&to=" + EndDate + "&api_token=" + api_token + "&period=d";
                    curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());
                    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
                    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
                    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
                    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data2);
                    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&data);

                    status = curl_easy_perform(handle);

                    if (status != CURLE_OK)
                    {
                        cout << endl;
                        cout << "curl_easy_perform() failed: " << curl_easy_strerror(status) << endl;
                        return -1;
                    }

                    stringstream sData;
                    sData.str(data.memory);
                    string sDate, line, sAdjClose;
                    double dAdjClose;
                    map<string, double> historical_prices;

                    getline(sData, line);
                    while (getline(sData, line))
                    {
                        if (line.empty())
                        {
                            cout << endl;
                            cout << "Empty line encountered." << endl;
                            continue;
                        }

                        size_t firstComma = line.find_first_of(',');
                        size_t lastComma = line.find_last_of(',');

                        if (firstComma == string::npos || lastComma == string::npos || firstComma >= lastComma)
                        {
                            cout << endl;
                            cout << "Line format not as expected." << endl;
                            continue;
                        }

                        sDate = line.substr(0, firstComma);
                        line.erase(lastComma);
                        sAdjClose = line.substr(line.find_last_of(',') + 1);

                        if (sAdjClose.empty())
                        {
                            cout << endl;
                            cout << "Adjusted close price is empty for line: " << line << endl;
                            continue;
                        }

                        dAdjClose = strtod(sAdjClose.c_str(), NULL);
                        historical_prices[sDate] = dAdjClose;
                    }

                    if (distance(historical_prices.begin(), historical_prices.end()) != 2 * N + 1)
                    {
                        if (historical_prices.size() == 0)
                        {
                            cout << data.memory << endl;
                        }
                        cout << endl;
                        cout << itr->GetTicker() << " does not have enough data. The ticker has been removed." << endl;
                        itr = tickers.erase(itr);
                        continue;
                    }

                    itr->SetAdjClosePrices(GetPrices(historical_prices));

                    itr->SetDates(GetDates(historical_prices));
                    Vector Price = itr->GetAdjClosePrices();
                    itr->SetCumDailyReturn(itr->CalcCumReturns());

                    free(data.memory);
                    data.size = 0;
                }
                ++itr; // Increment iterator
            }

            ++progress;
            bar.update(progress);
        }

        curl_easy_cleanup(handle);
        curl_global_cleanup();

        return 0;
    }

    // Retrieve IWV data
    int GetIWVData(string ticker, string start_date, string end_date, map<string, double> &historical_price, string api_token)
    {
        CURL *handle;
        CURLcode status;

        // set up the program environment that libcurl needs
        curl_global_init(CURL_GLOBAL_ALL);

        // curl_easy_init() returns a CURL easy handle
        handle = curl_easy_init();

        // if everything's all right with the easy handle
        if (handle)
        {
            string url_common = "https://eodhistoricaldata.com/api/eod/";

            struct MemoryStruct data;
            data.memory = NULL;
            data.size = 0;

            string url_request = url_common + ticker + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d";
            curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());

            // adding a user agent
            curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);

            // read into memory
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data2);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&data);

            // retrieve data
            status = curl_easy_perform(handle);

            // Check if the request was successful
            if (status != CURLE_OK)
            {
                cout << "curl_easy_perform() failed: " << curl_easy_strerror(status) << endl;
                return -1;
            }

            stringstream sData;
            sData.str(data.memory);
            string sDate;     // To store extracted date
            string line;      // To store each line of data
            string sAdjClose; // To store the adjusted close price as a string
            double dAdjClose; // To store the adjusted close price as a double

            while (getline(sData, line))
            {
                size_t found = line.find('-');
                if (found != string::npos)
                {
                    sDate = line.substr(0, line.find_first_of(','));
                    line.erase(line.find_last_of(','));

                    // Extract the adjusted close price
                    sAdjClose = line.substr(line.find_last_of(',') + 1);
                    dAdjClose = strtod(sAdjClose.c_str(), NULL);

                    // Store the date and adjusted close price in the historical_price map
                    historical_price[sDate] = dAdjClose;
                }
            }

            free(data.memory);
            data.size = 0;
        }
        else
        {
            cout << "Curl init failed!" << endl;
            return -1;
        }

        curl_easy_cleanup(handle);
        curl_global_cleanup();
        return 0;
    }

    // Multi-threading for retrieving historical data for Russell 3000 stocks
    void GetAllData(vector<Stock> &missTickers, vector<Stock> &meetTickers, vector<Stock> &beatTickers, vector<string> IntradeDays, int N, string api_token)
    {
        cout << "Start to retrieve Data..." << endl;
        // start time
        auto startTime = chrono::high_resolution_clock::now();

        // total tasks
        int totalTasks = missTickers.size() + meetTickers.size() + beatTickers.size();
        std::atomic<int> progress(0);
        ProgressBar bar(totalTasks);

        // To enhance efficiency, here we use multi-processing to rechieve data
        thread missThread(GetGroupData, ref(missTickers), IntradeDays, N, api_token, ref(progress), ref(bar));
        thread meetThread(GetGroupData, ref(meetTickers), IntradeDays, N, api_token, ref(progress), ref(bar));
        thread beatThread(GetGroupData, ref(beatTickers), IntradeDays, N, api_token, ref(progress), ref(bar));

        missThread.join();
        meetThread.join();
        beatThread.join();

        bar.finish();

        auto endTime = chrono::high_resolution_clock::now();
        // calculate time consumption
        auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
        cout << "Retrieved all Data." << endl;
        cout << "Total time taken: " << duration << " seconds." << endl;
    }

}
