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

std::mutex m;

//need this bc I added my method(s) first and it uses logToFileAndConsole that appears further down
void logToFileAndConsole(std::string message);


struct benchStats{

    //time taken to complete one benchmark in seconds
    float elapsedTime;

    //number of calls to each function
    int numGets = 0;
    int numContains = 0;
    int numAdds = 0;
    int numRemoves = 0;

    //average, min and max amount of time Gets function takes
    float avgGets = 0;
    float minGets = 0;
    float maxGets = 0;

    //average, min and max amount of time Contains function takes
    float avgContains = 0;
    float minContains = 0;
    float maxContains = 0;
    
    //average, min and max amount of time Adds function takes
    float avgAdds = 0;
    float minAdds = 0;
    float maxAdds = 0;

    //average, min and max amount of time Removes function takes
    float avgRemoves = 0;
    float minRemoves = 0;
    float maxRemoves = 0;

    int sleepInterval = 0;
};




void benchStatsToString(benchStats &stats){

    if(stats.elapsedTime){
        logToFileAndConsole("\nElapsed time of this benchmark: " + std::to_string(stats.elapsedTime) + "\n");
    }

    logToFileAndConsole("AVERAGE Time of Component Method Computations");
    logToFileAndConsole("Get Item: " + std::to_string(stats.avgGets));
    logToFileAndConsole("Add: " + std::to_string(stats.avgAdds));
    logToFileAndConsole("Contains: " + std::to_string(stats.avgContains));
    logToFileAndConsole("Remove: " + std::to_string(stats.avgRemoves));

    logToFileAndConsole("\nMIN Time of Component Method Computations\n");
    logToFileAndConsole("Get Item: " + std::to_string(stats.minGets));
    logToFileAndConsole("Add: " + std::to_string(stats.minAdds));
    logToFileAndConsole("Contains: " + std::to_string(stats.minContains));
    logToFileAndConsole("Remove: " + std::to_string(stats.minRemoves));

    logToFileAndConsole("\nMAX Time of Component Method Computations\n");
    logToFileAndConsole("Get Item: " + std::to_string(stats.maxGets));
    logToFileAndConsole("Add: " + std::to_string(stats.maxAdds));
    logToFileAndConsole("Contains: " + std::to_string(stats.maxContains));
    logToFileAndConsole("Remove: " + std::to_string(stats.maxRemoves));


    logToFileAndConsole("\nNumber of calls for each component method");
    logToFileAndConsole("Get Item: " + std::to_string(stats.numGets));
    logToFileAndConsole("Add: " + std::to_string(stats.numAdds));
    logToFileAndConsole("Contains: " + std::to_string(stats.numContains));
    logToFileAndConsole("Remove: " + std::to_string(stats.numRemoves));

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
    //std::cout << "\nThread id = " << std::this_thread::get_id() << std::endl;
    std::stringstream stream;
    stream << std::this_thread::get_id();
    std::string threadId = stream.str();
    //logToFileAndConsole("\nThread id = " + threadId);

    for (auto key = startRange; key <= endRange; ++key) {
        std::string value = "Test value for key: " + std::to_string(key);
        cm.add(key, value);
        //logToFileAndConsole("Added key: " + std::to_string(key) + "; value: " + value);
    }

    //std:: cout << "Finised population wiht thread: " <<  std::this_thread::get_id() << std::endl;
    //logToFileAndConsole( "Finised population wiht thread: " + threadId);


}


void benchmarkCacheManager(cache::CacheManager<int, std::string, bench::TbbBench> &cm, benchStats &stats) {
   
    //std::lock_guard<std::mutex> lock (m); 
    std::cout << "entered benchmark function\n\n";
    auto start = std::chrono::system_clock::now();

    //setup for accumalative time of each component function (which will be used for computing avg time for each)
    std::chrono::duration<double> getTotTime;
    std::chrono::duration<double> containsTotTime;
    std::chrono::duration<double> addTotTime;
    std::chrono::duration<double> removeTotTime;

    //setup for finding min time of each component method
    float getMinTime = std::numeric_limits<float>::infinity();;
    float containsMinTime = std::numeric_limits<float>::infinity();;
    float addMinTime = std::numeric_limits<float>::infinity();;
    float removeMinTime = std::numeric_limits<float>::infinity();;

    //setup for finding min time of each component method
    float getMaxTime = 0;
    float containsMaxTime = 0;
    float addMaxTime = 0;
    float removeMaxTime = 0;
    

    for (int i = 1; i < 10; i ++) {

        for (int j = 1; j < 4; j++){

            auto getTimeNL1 = std::chrono::system_clock::now();
            cm.getItem(1);
            auto getEndNL1 = std::chrono::system_clock::now();
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


        auto addTime = std::chrono::system_clock::now();
        cm.add(1, "stuff");
        auto addEnd = std::chrono::system_clock::now();
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
            auto getTimeNL2 = std::chrono::system_clock::now();
            cm.getItem(1);
            auto getEndNL2 = std::chrono::system_clock::now();
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
    
    }

    auto End = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = End - start;

    //set duration of one benchmark 
    stats.elapsedTime = elapsed_seconds.count();

    //set average time of each component method
    stats.avgGets = getTotTime.count() / stats.numGets;
    stats.avgAdds = addTotTime.count() / stats.numAdds;
    stats.avgContains = containsTotTime.count() / stats.numContains;
    stats.avgRemoves = removeTotTime.count() / stats.numRemoves;

    //set mins
    stats.minGets = getMinTime;
    stats.minAdds = addMinTime;
    stats.minContains = containsMinTime;
    stats.minRemoves = removeMinTime;

    //set maxs
    stats.maxGets = getMaxTime;
    stats.maxAdds = addMaxTime;
    stats.maxContains = containsMaxTime;
    stats.maxRemoves = removeMaxTime;

    benchStatsToString(stats);

    
    //std::this_thread::sleep_for(stats.sleepInterval);

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
    
    benchStats stats;
    stats.sleepInterval = config["Milestone3"][0]["defaultVariables"][0]["sleepInterval"];
    benchmarkCacheManager(cm, stats);
    std::cout << "ELAPSED TIME: " << stats.elapsedTime << std::endl;

    // set the start time
    auto start = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    // output some helpful comments to the console
    std::cout << "Starting computation at " << std::ctime(&start_time);

    // write out the head line for file
//logToFileAndConsole("threadId\t\tend time\t\titer #\titer\tavg\t\tmin\t\tmax\t\t");

    // need to write out the data for each timed iteration in the following format:
    // 
    // threadId    end time    iter#   avg     min     max     
    // 
    // <threadId1> <time1>         1   1.2     0.9     1.4
    // <threadId2> <time2>         2   1.1     0.7     1.2
    // ...

//    int testSize = config["Milestone3"][0]["defaultVariables"][0]["testSize"];
    
    

    // output some helpful comments to the console
    // add the load time calc and output here
//std::cout << "Starting computation at " << std::ctime(&start_time);

    // write out the head line for file
//logToFileAndConsole("threadId\tend time\titer #\t\tavg\t\tmin\t\tmax\t\t");

    // after loading the cache, spawn threads and start the static ratio test as discussed in Canvas
    // use the same output format as in milestone2 (method, timeWrapper, is left in this example).  
    //     Add thread ID as the first column.

    /*// call the specific function to time
    for (int i = 0; i < 10; i++) {
        float average = 0.0;
        float min = 0.0;
        float max = 0.0;
        int threadId = 1;

        // add more functions here
        //getItemTest(config, cm);

        // write out the current values for this iteration
        auto curIterEnd = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = curIterEnd - start;
        std::time_t iterEndTime = std::chrono::system_clock::to_time_t(curIterEnd);
        std::string timeString = "00:00:00";

        logToFileAndConsole(std::to_string(threadId) + "\t" + timeString + "\t" + std::to_string(i) + "\t\t" + std::to_string(average) + "\t" + std::to_string(min) + "\t" + std::to_string(max));
    }
        */

    logToFileAndConsole("\n\n");

    // set the end time
    auto finalEnd = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = finalEnd - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(finalEnd);

    std::cout << "Finished computation at " << std::ctime(&end_time)
        << "Elapsed time: " << elapsed_seconds.count() << "s"
        << std::endl << std::endl;

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

    // Retrieve default numberOfIterations
    //int numberOfIterations = config["Milestone3"][0]["defaultVariables"][0]["numberOfIterations"];

    // Retrieve default readsize
    //int readSize = config["Milestone3"][0]["defaultVariables"][0]["readSize"];


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
