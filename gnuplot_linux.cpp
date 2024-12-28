#include "gnuplot_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

namespace fre
{
    void plotCAAR(const vector<double> &xData, const vector<double> &beatCAAR, const vector<double> &meetCAAR, const vector<double> &missCAAR)
    {
        if (xData.empty() || beatCAAR.empty() || meetCAAR.empty() || missCAAR.empty())
        {
            printf("Error: One or more input vectors are empty. Ensure all data is provided.\n");
            return;
        }

        if (xData.size() != beatCAAR.size() || xData.size() != meetCAAR.size() || xData.size() != missCAAR.size())
        {
            printf("Error: Input vector sizes are inconsistent. Ensure all vectors have the same length.\n");
            return;
        }

        const char *beatFile = "beatCAAR.dat";
        const char *meetFile = "meetCAAR.dat";
        const char *missFile = "missCAAR.dat";

        // Write data to files
        FILE *tempDataFile = fopen(beatFile, "w");
        for (size_t i = 0; i < xData.size(); ++i)
        {
            fprintf(tempDataFile, "%lf %lf\n", xData[i], beatCAAR[i]);
        }
        fclose(tempDataFile);

        tempDataFile = fopen(meetFile, "w");
        for (size_t i = 0; i < xData.size(); ++i)
        {
            fprintf(tempDataFile, "%lf %lf\n", xData[i], meetCAAR[i]);
        }
        fclose(tempDataFile);

        tempDataFile = fopen(missFile, "w");
        for (size_t i = 0; i < xData.size(); ++i)
        {
            fprintf(tempDataFile, "%lf %lf\n", xData[i], missCAAR[i]);
        }
        fclose(tempDataFile);

        // Plot by Gnuplot
        FILE *gnuplotPipe = popen("gnuplot -persist", "w");
        if (!gnuplotPipe)
        {
            printf("Error: Gnuplot not found.\n");
            return;
        }

        // Set the output format and file
        fprintf(gnuplotPipe, "set terminal pngcairo size 800,600 enhanced font 'Arial,12'\n");
        fprintf(gnuplotPipe, "set output 'CAAR_plot.png'\n");

        const char *title = "CAAR of Russell 3000 Stocks Based on Earnings Surprises";
        const char *yLabel = "Cumulative Average Abnormal Return (CAAR)";
        fprintf(gnuplotPipe, "reset\n"); // Reset Gnuplot environment
        fprintf(gnuplotPipe, "set grid\n");
        fprintf(gnuplotPipe, "set title \"%s\"\n", title);
        fprintf(gnuplotPipe, "set xlabel 'Days Around Earnings Announcement Date'\n");
        fprintf(gnuplotPipe, "set ylabel \"%s\"\n", yLabel);
        fprintf(gnuplotPipe, "plot '%s' with lines title 'Beat Group', '%s' with lines title 'Meet Group', '%s' with lines title 'Miss Group'\n", beatFile, meetFile, missFile);
        fflush(gnuplotPipe);

        printf("Plot saved as 'CAAR_plot.png'.\n");
        printf("Press Enter to continue...\n");
        getchar();

        fprintf(gnuplotPipe, "exit\n");
        fflush(gnuplotPipe);
        pclose(gnuplotPipe);

        // Clean up
        remove(beatFile);
        remove(meetFile);
        remove(missFile);
    }
}