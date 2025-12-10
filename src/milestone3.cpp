/**
*
* milestone3.cpp : This file contains the 'main' function. Program execution begins and ends there.
*
* 09/23/24 - Created by ChatGPT with prompt "write C++ program reads and parses the file: milestone3_config.json"
*            The file: "milestone3_config.json" is in the following format:
*
                {
                    "Milestone3": [
                        {
                            "files": [
                                {
                                    "testcaseFile": "milestone3.json",
                                    "outputFile": "generatedOutputFile.txt",
                                    "errorLogFile":"logFile.txt"
                                }
                            ],
                            "defaultVariables": [
                                {
                                    "numberOfIterations": 10000,
                                    "readSize": 100
                                }
                            ]
                        }
                    ]
                }
            and where the testcaseFile has the following format:
                {
                    "FileReadBenchmark": [
                        {
                            "testCase1": [
                                {
                                    "readTest": {"inputFileName": "top-output.txt", "numberOfIterations": 100}
                                }
                                ...
                            ]
                        }
                    ]
                }

9/15/2025 - create by Joseph Hui;

*/
#include "cache-manager.hpp"
#include "benchmark.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <sstream>

#include "json.hpp"

using json = nlohmann::json;

#define _CRT_SECURE_NO_WARNINGS
#define CONFIG_FILE "milestone3_config.json"

// Global variable to be used for logging output
std::ofstream _outFile;
int iteration = 0;
std::mutex m;

//need this bc I added my method(s) first and it uses logToFileAndConsole that appears further down
void logToFileAndConsole(std::string message);


struct benchStats{

    //time taken to complete one benchmark in seconds
    float elapsedTime;
    int iteration = 0;

    //number of calls to each function
    int numGets = 0;
    int numContains = 0;
    int numAdds = 0;
    int numRemoves = 0;
    int numClear = 0;



    float totTimeGets = 0;
    float totTimeAdd = 0;
    float totTimeContains;
    float totTimeRemove = 0;
    float totTimeClear = 0;


    //average, min and max amount of time Gets function takes
    float avgGets = 0;
    float minGets = std::numeric_limits<float>::infinity();;;
    float maxGets = 0;

    //average, min and max amount of time Contains function takes
    float avgContains = 0;
    float minContains = std::numeric_limits<float>::infinity();;;
    float maxContains = 0;
    
    //average, min and max amount of time Adds function takes
    float avgAdds = 0;
    float minAdds = std::numeric_limits<float>::infinity();;;
    float maxAdds = 0;

    //average, min and max amount of time Removes function takes
    float avgRemoves = 0;
    float minRemoves = std::numeric_limits<float>::infinity();;;
    float maxRemoves = 0;

     //average, min and max amount of time Clear function takes
    float avgClear = 0;
    float minClear = std::numeric_limits<float>::infinity();;;
    float maxClear = 0;

    int sleepInterval = 0;
};


int testKey(bool inCache){

    static std::mt19937 gen(std::random_device{}());

    // generate a number mostly inside 1–1000
    if (inCache) {
        std::uniform_int_distribution<int> distHit(1, 1000);
        return distHit(gen);
    }

    // generate a number always outside 1–1000
    std::uniform_int_distribution<int> distMiss(1001, 2000);
    return distMiss(gen);
}

void benchStatsToString(benchStats &stats){

    std::stringstream ss;
    ss << std::this_thread::get_id();
    //logToFileAndConsole("\nThread id: " + ss.str());
    //std::cout << "entered benchmark function, thread id: " << std::this_thread::get_id();
    logToFileAndConsole("Method Component\t\tthreadId\t\ttot time\titer #\t\tavg\t\t\t\tmin\t\t\t\tmax\t\t");
    //logToFileAndConsole("GetItem");
    logToFileAndConsole("GetItem \t\t\t"+ ss.str() + "\t\t" + std::to_string(stats.totTimeGets) + "\t" + std::to_string(iteration) +"\t\t\t" +std::to_string(stats.avgGets)  +"\t\t" + std::to_string(stats.minGets)  +"\t\t" + std::to_string(stats.minGets));
    
    //logToFileAndConsole("Adds");
    logToFileAndConsole("Add      \t\t\t" + ss.str() + "\t\t" + std::to_string(stats.totTimeAdd) + "\t" + std::to_string(iteration) +"\t\t\t" +std::to_string(stats.avgAdds)  +"\t\t" + std::to_string(stats.minAdds)  +"\t\t" + std::to_string(stats.maxAdds));

    //logToFileAndConsole("Contains");
    logToFileAndConsole("Contains\t\t\t" + ss.str() + "\t\t" + std::to_string(stats.totTimeContains) + "\t" + std::to_string(iteration) +"\t\t\t" +std::to_string(stats.avgContains)  +"\t\t" + std::to_string(stats.minContains)  +"\t\t" + std::to_string(stats.maxContains));

    //logToFileAndConsole("Removes");
    logToFileAndConsole("Remove  \t\t\t" + ss.str() + "\t\t" + std::to_string(stats.totTimeRemove) + "\t" + std::to_string(iteration) +"\t\t\t" +std::to_string(stats.avgRemoves)  +"\t\t" + std::to_string(stats.minRemoves)  +"\t\t" + std::to_string(stats.maxRemoves));

    logToFileAndConsole("Clear  \t\t\t\t" + ss.str() + "\t\t" + std::to_string(stats.totTimeClear) + "\t" + std::to_string(iteration) +"\t\t\t" +std::to_string(stats.avgClear)  +"\t\t" + std::to_string(stats.minClear)  +"\t\t" + std::to_string(stats.maxClear));


    logToFileAndConsole("Number of calls to getItem: " + std::to_string(stats.numGets));

    logToFileAndConsole("Number of calls to addItem: " + std::to_string(stats.numAdds));

    logToFileAndConsole("Number of calls to Contains: " + std::to_string(stats.numContains));

    logToFileAndConsole("Number of calls to Remove: " + std::to_string(stats.numRemoves));

    logToFileAndConsole("Number of calls to Clear: " + std::to_string(stats.numClear));


    if(stats.elapsedTime){
        logToFileAndConsole("Elapsed time of this benchmark: " + std::to_string(stats.elapsedTime) + "\n");
    }

}


/**
*
* loadCacheManager
*
* @brief Loading Cache Manager with 1000 items with integer key values from 1-1000 
*
* @param    cache         cache manager object passed by reference 
* @param    int         start range of cache manager population 
* @param    int         end range of cache manager population 
*
* @return   nothing, cache manager will be populated with key and values specified by range, 
*/
void loadCacheManager(cache::CacheManager<int, std::string, bench::TbbBench> &cm, int startRange, int endRange ) {

    std::lock_guard<std::mutex> lock (m); //auto lock and unlock the following section of code

    using namespace std::literals::chrono_literals;

    std::stringstream stream;
    stream << std::this_thread::get_id();
    std::string threadId = stream.str();

    for (auto key = startRange; key <= endRange; ++key) {
        std::string value = "Test value for key: " + std::to_string(key);
        cm.add(key, value);
    }


}


void benchmarkCacheManager(cache::CacheManager<int, std::string, bench::TbbBench> &cm, benchStats &stats) {
   
    //std::lock_guard<std::mutex> lock (m); 
    auto start = std::chrono::steady_clock::now();

    //setup for accumalative time of each component function (which will be used for computing avg time for each)
    std::chrono::duration<double> getTotTime;
    std::chrono::duration<double> containsTotTime;
    std::chrono::duration<double> addTotTime;
    std::chrono::duration<double> removeTotTime;
    std::chrono::duration<double> clearTotTime;


    //setup for finding min time of each component method
    float getMinTime = std::numeric_limits<float>::infinity();;
    float containsMinTime = std::numeric_limits<float>::infinity();;
    float addMinTime = std::numeric_limits<float>::infinity();;
    float removeMinTime = std::numeric_limits<float>::infinity();;
    float clearMinTime = std::numeric_limits<float>::infinity();;


    //setup for finding min time of each component method
    float getMaxTime = 0;
    float containsMaxTime = 0;
    float addMaxTime = 0;
    float removeMaxTime = 0;
    float clearMaxTime = 0;
    

    for (int i = 1; i < 10; i ++) {

        for (int j = 1; j < 4; j++){

            auto getTimeNL1 = std::chrono::steady_clock::now();
            cm.getItem(1);
            auto getEndNL1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> GetDurationNL1 = getEndNL1 - getTimeNL1;
            getTotTime += GetDurationNL1;

            if(GetDurationNL1.count() < getMinTime){
                getMinTime = GetDurationNL1.count();
            }

            if(GetDurationNL1.count() > getMaxTime){
                getMaxTime = GetDurationNL1.count();
            }

            stats.numGets++;

        }


        auto addTime = std::chrono::steady_clock::now();
        cm.add(1, "stuff");
        auto addEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double> AddDuration = addEnd - addTime;
        addTotTime += AddDuration;

        if(AddDuration.count() < addMinTime){
            addMinTime = AddDuration.count();
        }

        if(AddDuration.count() > addMaxTime){
            addMaxTime = AddDuration.count();
        }

        stats.numAdds++;        


        for (int j = 1; j < 4; j++){
            auto getTimeNL2 = std::chrono::steady_clock::now();
            cm.getItem(1);
            auto getEndNL2 = std::chrono::steady_clock::now();
            std::chrono::duration<double> GetDurationNL2 = getEndNL2 - getTimeNL2;
            getTotTime += GetDurationNL2;
            if(GetDurationNL2.count() < getMinTime){
                getMinTime = GetDurationNL2.count();
            }
            if(GetDurationNL2.count() > getMaxTime){
                getMaxTime = GetDurationNL2.count();
            }
            stats.numGets++;
        }


        if (i == 2 || i == 8){

            auto containsTime = std::chrono::system_clock::now();
            cm.contains(1);
            auto containsEnd = std::chrono::system_clock::now();
            std::chrono::duration<double> escontainsDuration = containsEnd - containsTime;
            containsTotTime += escontainsDuration;

            if(escontainsDuration.count() < containsMinTime){
                containsMinTime = escontainsDuration.count();
            }

            if(escontainsDuration.count() > containsMaxTime){
                containsMaxTime = escontainsDuration.count();
            }

            stats.numContains++; 
        }

        for (int j = 1; j < 4; j++){

            auto getTimeNL3 = std::chrono::system_clock::now();
            cm.getItem(1);
            auto getEndNL3 = std::chrono::system_clock::now();
            std::chrono::duration<double> GetDurationNL3 = getEndNL3 - getTimeNL3;
            getTotTime += GetDurationNL3;

            if(GetDurationNL3.count() < getMinTime){
                getMinTime = GetDurationNL3.count();
            }

            if(GetDurationNL3.count() > getMaxTime){
                getMaxTime = GetDurationNL3.count();
            }

            stats.numGets++;; 
        }
        

       
        auto removeTime = std::chrono::system_clock::now();
        cm.remove(1);
        auto removeEnd = std::chrono::system_clock::now();
        std::chrono::duration<double> removeDuration = removeEnd - removeTime;
        removeTotTime += removeDuration;

        if(removeDuration.count() < removeMinTime){
            removeMinTime = removeDuration.count();
        }

        if(removeDuration.count() >removeMaxTime){
        removeMaxTime = removeDuration.count();
        }

        stats.numRemoves++;
        
    
    std::this_thread::sleep_for(std::chrono::milliseconds(stats.sleepInterval));

    }

    auto clearTime = std::chrono::system_clock::now();
        cm.clear();
        auto clearEnd = std::chrono::system_clock::now();
        std::chrono::duration<double> clearDuration = clearEnd - clearTime;
        clearTotTime += clearDuration;

        if(clearDuration.count() < clearMinTime){
            clearMinTime = clearDuration.count();
        }

        if(clearDuration.count() > clearMaxTime){
            clearMaxTime = clearDuration.count();
        }

        stats.numClear++;

    auto End = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = End - start;
    
    //set total time of how long each component method took
    stats.totTimeGets = getTotTime.count();
    stats.totTimeAdd = addTotTime.count();
    stats.totTimeContains = containsTotTime.count();
    stats.totTimeRemove = removeTotTime.count();
    stats.totTimeClear =clearTotTime.count();

    //set duration of one benchmark 
    stats.elapsedTime = elapsed_seconds.count();

    //set average time of each component method
    stats.avgGets = stats.totTimeGets / stats.numGets;
    stats.avgAdds = stats.totTimeAdd  / stats.numAdds;
    stats.avgContains = stats.totTimeContains / stats.numContains;
    stats.avgRemoves = stats.totTimeRemove / stats.numRemoves;
    stats.avgRemoves = stats.totTimeClear / stats.numClear;

    //set mins
    stats.minGets = getMinTime;
    stats.minAdds = addMinTime;
    stats.minContains = containsMinTime;
    stats.minRemoves = removeMinTime;
    stats.minClear = clearMinTime;

    //set maxs
    stats.maxGets = getMaxTime;
    stats.maxAdds = addMaxTime;
    stats.maxContains = containsMaxTime;
    stats.maxRemoves = removeMaxTime;
    stats.maxClear = clearMaxTime;

    //set iteration number
    iteration++;
    //benchStatsToString(stats);

    

}







/**
*
* getOutFile
*
* @brief function to return pointer to outFile
*
* @param        none
*
* @return       pointer to output file
*/
std::ofstream& getOutFile() {
    return _outFile;
}


/**
*
* setOutFile
*
* @brief function to set path and open output file
*
* @param filePath       the path to output file
*
* @return               nothing
*/
void setOutFile(const std::string& filePath) {
    // Close the current file if it's already open
    if (_outFile.is_open()) {
        _outFile.close();
    }

    // Open the new file
    _outFile.open(filePath);
    if (!_outFile.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }
}

/**
 * logToFileAndConsole
 *
 * @brief Logs a message to both the console and the output file.
 *
 * This helper function prints a message to the console and writes the same
 * message to the output file.
 *
 * @param message The message to log
 */
void logToFileAndConsole(std::string message) {
    // Get the output file
    std::ofstream& outFile = getOutFile();

    std::cout << message << std::endl;  // Print to console 
    outFile << message << std::endl;  // Write to file
}

/**
*
* getItemTest
*
* @brief calls CacheManager.getItem()
*
* @param    config              benchmark config
*
* @return   nothing, but output is sent to console and written to output file
*/
void getItemTest(json config, cache::CacheManager<int, std::string, bench::TbbBench> &cm) {
    // sample code to retrieve and print a found entry
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution distr(1, 10);
    int testKey = distr(gen);
    auto val = cm.getItem(testKey);
    logToFileAndConsole("\n\nRetrieved key: " + std::to_string(testKey) + "; with value: " + *val);

    // try one that is not in the cache
    testKey = testKey * 1000;
    val = cm.getItem(testKey);
    if (!val) {
       logToFileAndConsole("Key: " + std::to_string(testKey) + " not found (expected).");
    } else {
       logToFileAndConsole("Retrieved key: " + std::to_string(testKey) + "; with value: " + *val);
    }
}


/**
*
* timeWrapper
*
* @brief Simple wrapper function that times a limited number of functions (hardcoded for now)
*
* @param    config              benchmark config
*
* @return   nothing, but output is sent to console and written to output file
*/
void timeWrapper(json config) {

    auto start = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(start);
    std::string startTimeStr = std::ctime(&start_time);
    logToFileAndConsole("\nBenchmark Started at: " + startTimeStr);
    
    //LOADING CACHE WITH 1000 items 
    auto startLoad = std::chrono::system_clock::now();

    // Allocate the cache manager
    cache::CacheManager<int, std::string, bench::TbbBench> cm;
    loadCacheManager(cm, 1, 1000);
    auto finalLoadEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> load_elapsed_seconds = finalLoadEnd - startLoad;
    std::time_t end_load_time = std::chrono::system_clock::to_time_t(finalLoadEnd);

    std::cout << "\nFinished loading cache manager at " << std::ctime(&end_load_time)
        << "Elapsed time to load cache manager: " << load_elapsed_seconds.count() << "s"
        << std::endl << std::endl;
    
    
    //BENCHMARK
    int degreeOfParallelism = config["Milestone3"][0]["defaultVariables"][0]["degreeOfParallelism"];

    //vector of bStats structures to store stats and later aggregate
    std::vector<benchStats> bStats;

    //pushing to the bStat vector
    for(int i = 0; i < degreeOfParallelism; i++){
        benchStats stats;
        stats.sleepInterval = config["Milestone3"][0]["defaultVariables"][0]["sleepInterval"];
        bStats.push_back(stats);
    }

    //creating thread of workers and then calling benchMarkCacheManager
    std::vector<std::thread> bWorkerThreads;

    for(int i = 0; i < degreeOfParallelism; i++){
        bWorkerThreads.emplace_back(benchmarkCacheManager, std::ref(cm), std::ref(bStats[i]));
    }

    for(int i = 0; i < degreeOfParallelism; i++){
        bWorkerThreads[i].join();
    }


    benchStats aggregate;

    for(int i = 0; i < degreeOfParallelism; i++){

        //accumulating totat number of calls for each component function
        aggregate.numGets += bStats[i].numGets;
        aggregate.numAdds += bStats[i].numAdds;
        aggregate.numContains += bStats[i].numContains;
        aggregate.numRemoves += bStats[i].numRemoves;
        aggregate.numClear += bStats[i].numClear;

        //accumulating totat number of time for each component function
        aggregate.totTimeGets += bStats[i].totTimeGets;
        aggregate.totTimeAdd += bStats[i].totTimeAdd;
        aggregate.totTimeContains += bStats[i].totTimeContains;
        aggregate.totTimeRemove += bStats[i].totTimeRemove;
        aggregate.totTimeClear += bStats[i].totTimeClear;

        //getting min time value out of all threads
        if(bStats[i].minGets < aggregate.minGets){
            aggregate.minGets = bStats[i].minGets;
        }
        if(bStats[i].minAdds < aggregate.minAdds){
            aggregate.minAdds = bStats[i].minAdds;
        }
        if(bStats[i].minContains < aggregate.minContains){
            aggregate.minContains = bStats[i].minContains;
        }
        if(bStats[i].minRemoves < aggregate.minRemoves){
            aggregate.minRemoves = bStats[i].minRemoves;
        }
        if(bStats[i].minClear < aggregate.minClear){
            aggregate.minClear = bStats[i].minClear;
        }

        //getting max time value out of all threads
        if(bStats[i].maxGets > aggregate.maxGets){
            aggregate.maxGets = bStats[i].maxGets;
        }
        if(bStats[i].maxAdds > aggregate.maxAdds){
            aggregate.maxAdds = bStats[i].maxAdds;
        }
        if(bStats[i].maxContains > aggregate.minContains){
            aggregate.maxContains = bStats[i].maxContains;
        }
        if(bStats[i].maxRemoves > aggregate.maxRemoves){
            aggregate.maxRemoves = bStats[i].maxRemoves;
        }
        if(bStats[i].maxClear > aggregate.maxClear){
            aggregate.maxClear = bStats[i].maxClear;
        }

    }

    //calculating the average time of each component function
    aggregate.avgGets = aggregate.totTimeGets / aggregate.numGets;
    aggregate.avgAdds = aggregate.totTimeAdd  / aggregate.numAdds;
    aggregate.avgContains = aggregate.totTimeContains / aggregate.numContains;
    aggregate.avgRemoves = aggregate.totTimeRemove / aggregate.numRemoves;

    logToFileAndConsole("Aggregate Values Across All Threads");
    benchStatsToString(aggregate);
    logToFileAndConsole("WHATSS: " + std::to_string(aggregate.totTimeGets));
    // set the end time
    auto finalEnd = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = finalEnd - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(finalEnd);
    std::string endTimeStr = std::ctime(&end_time);
    

    logToFileAndConsole( "\nFinished Benchmark computation at " + endTimeStr +"\nElapsed Time: " + std::to_string(elapsed_seconds.count())+"s\n\n");
    
    // print out the cache manager results
    auto benchmarkResults = cm.benchmark();
    bench::printBenchmark(benchmarkResults);

    return;
}


/**
*
* staticRatio
*
* @brief run the benchmark with static method ratios
*
* @param    config          json configuration
*
* @return                   nothing, but output is sent to console and written to output file
*/
void staticRatio(json config) {
    logToFileAndConsole("\nProcessing staticRatio benchmark.\n\n");
    logToFileAndConsole("Configuration for benchmark run:\n");

    // Retrieve configured Duration
    int testDuration = config["Milestone3"][0]["defaultVariables"][0]["testDuration"];
    logToFileAndConsole("\ttestDuration: " + std::to_string(testDuration));

    // Retrieve configured Test Type
    std::string testType = config["Milestone3"][0]["defaultVariables"][0]["testType"];
    logToFileAndConsole("\ttestType: " + testType);

    // Retrieve configured Degree of Parallelism
    int degreeOfParallelism = config["Milestone3"][0]["defaultVariables"][0]["degreeOfParallelism"];
    logToFileAndConsole("\tdegreeOfParallelism: " + std::to_string(degreeOfParallelism));

    // Retrieve configured Sleep Interval
    int sleepInterval = config["Milestone3"][0]["defaultVariables"][0]["sleepInterval"];
    logToFileAndConsole("\tsleepInterval: " + std::to_string(sleepInterval));

    // Retrieve the method ratios
    int getItemRatio = config["Milestone3"][0]["defaultVariables"][0]["ratioOfMethods"][0]["getItem"];
    int addRatio = config["Milestone3"][0]["defaultVariables"][0]["ratioOfMethods"][0]["add"];
    int containsRatio = config["Milestone3"][0]["defaultVariables"][0]["ratioOfMethods"][0]["contains"];
    int removeRatio = config["Milestone3"][0]["defaultVariables"][0]["ratioOfMethods"][0]["remove"];
    int clearRatio = config["Milestone3"][0]["defaultVariables"][0]["ratioOfMethods"][0]["clear"];
    logToFileAndConsole("\tratioOfMethods: ");
    logToFileAndConsole("\t\tgetItem: " + std::to_string(getItemRatio));
    logToFileAndConsole("\t\tadd: " + std::to_string(addRatio));
    logToFileAndConsole("\t\tcontains: " + std::to_string(containsRatio));
    logToFileAndConsole("\t\tremove: " + std::to_string(removeRatio));
    logToFileAndConsole("\t\tclear: " + std::to_string(clearRatio));

    // call to the timing wrapper
    timeWrapper(config);

    logToFileAndConsole("\nFinished processing benchmark.\n\n");
}


/**
*
* benchmarkWrapper
*
* @brief main wrapper method for the benchmark
*
* @param    config          json configuration
*
* @return                   nothing, but output is sent to console and written to output file
*/
void benchmarkWrapper(json config) {
    // Retrieve configured Test Type
    std::string testType = config["Milestone3"][0]["defaultVariables"][0]["testType"];

    if (testType == "static") {
        staticRatio(config);
    }
}


/**
*
* main
*
* main function which does the following:
*   read config file for input file, output file, error file, hash table size and FIFO size
*   create a hash table
*   for each of the test case
*       process test cases - display results to console and write to output file
*       print out the hash table
*       clear out hash table
*
* @param    none
*
* @return   nothing, but output is written to console and files
*/

int main() {
    // Load the config file

    std::ifstream configFile(CONFIG_FILE);
    if (!configFile.is_open()) {
        std::cerr << "Error opening config file!" << std::endl;
        return 1;
    }

    json config;
    configFile >> config;

    // Retrieve file paths from the config
    std::string outputFilePath = config["Milestone3"][0]["files"][0]["outputFile"];
    std::string errorFilePath = config["Milestone3"][0]["files"][0]["errorLogFile"];

    

    // Open up the outfile and set the output file path using the setter
    //
    // Treating output file differently than input and config files because it's used in other files
    setOutFile(outputFilePath);

    // Get the output file
    std::ofstream& outFile = getOutFile();

    // Call the benchmark wrapper
    benchmarkWrapper(config);

    logToFileAndConsole("\n\nEnd of tests");

    // Close files
    configFile.close();
    outFile.close();

    return 0;
}
