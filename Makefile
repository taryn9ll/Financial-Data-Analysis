# Compiler and flags
CC = g++
CFLAGS = -Wall -g -std=c++11
LIBS = -lcurl -lpthread

# Object files
OBJS = FinalProject.o GetHistoricalData.o StockSplit.o Matrix.o BootStrap.o Benchmark.o gnuplot_linux.o

# Target executable
TARGET = FinalProject

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Individual compilation rules
FinalProject.o: FinalProject.cpp Benchmark.h Matrix.h BootStrap.h GetHistoricalData.h StockSplit.h
	$(CC) $(CFLAGS) -c FinalProject.cpp

GetHistoricalData.o: GetHistoricalData.cpp GetHistoricalData.h
	$(CC) $(CFLAGS) -c GetHistoricalData.cpp

StockSplit.o: StockSplit.cpp StockSplit.h Matrix.h
	$(CC) $(CFLAGS) -c StockSplit.cpp

Matrix.o: Matrix.cpp Matrix.h
	$(CC) $(CFLAGS) -c Matrix.cpp

BootStrap.o: BootStrap.cpp BootStrap.h Matrix.h StockSplit.h GetHistoricalData.h
	$(CC) $(CFLAGS) -c BootStrap.cpp

Benchmark.o: Benchmark.cpp Benchmark.h Matrix.h BootStrap.h GetHistoricalData.h StockSplit.h
	$(CC) $(CFLAGS) -c Benchmark.cpp

gnuplot_linux.o: gnuplot_linux.cpp gnuplot_linux.h
	$(CC) $(CFLAGS) -c gnuplot_linux.cpp

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)
	