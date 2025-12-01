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

    //LOADING CACHE WITH 1000 items (threaded to practice)

    auto startLoad = std::chrono::system_clock::now();

    // Allocate the cache manager
    cache::CacheManager<int, std::string, bench::TbbBench> cm;

    std::thread threadOneLoad(loadCacheManager, std::ref(cm), 1, 1000);
    //tested and 1 thread ended up being faster likely because of the locks and waiting
    //so technically dont really need threading for loading but already tried/practiced 
    //std::thread threadTwoLoad(loadCacheManager, std::ref(cm), 251, 500);
    //std::thread threadThreeLoad(loadCacheManager, std::ref(cm), 501, 750);
    //std::thread threadFourLoad(loadCacheManager, std::ref(cm), 751, 1000);

    threadOneLoad.join();
    //threadTwoLoad.join();
    //threadThreeLoad.join();
    //threadFourLoad.join();

    auto finalLoadEnd = std::chrono::system_clock::now();

    std::chrono::duration<double> load_elapsed_seconds = finalLoadEnd - startLoad;
    std::time_t end_load_time = std::chrono::system_clock::to_time_t(finalLoadEnd);

    std::cout << "\nFinished loading cache manager at " << std::ctime(&end_load_time)
        << "Elapsed time to load cache manager: " << load_elapsed_seconds.count() << "s"
        << std::endl << std::endl;


    //we are going to make a function to load 1000 items into the cache manager instead of doing 
    //this that we have below 
    // sample test load of the cache
    /*     //helped in creating cache load and testing
    for (auto key = 0; key <= testSize; ++key) {
        std::string value = "Test value for key: " + std::to_string(key);
        cm.add(key, value);
        logToFileAndConsole("Added key: " + std::to_string(key) + "; value: " + value);
    }
    */


    // set the start time
    auto start = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    // output some helpful comments to the console
    std::cout << "Starting computation at " << std::ctime(&start_time);

    // write out the head line for file
    logToFileAndConsole("threadId\t\tend time\t\titer #\titer\tavg\t\tmin\t\tmax\t\t");

    // need to write out the data for each timed iteration in the following format:
    // 
    // threadId    end time    iter#   avg     min     max     
    // 
    // <threadId1> <time1>         1   1.2     0.9     1.4
    // <threadId2> <time2>         2   1.1     0.7     1.2
    // ...

    int testSize = config["Milestone3"][0]["defaultVariables"][0]["testSize"];
    
   


    

    

    // output some helpful comments to the console
    // add the load time calc and output here
    std::cout << "Starting computation at " << std::ctime(&start_time);

    // write out the head line for file
    logToFileAndConsole("threadId\tend time\titer #\t\tavg\t\tmin\t\tmax\t\t");

    // after loading the cache, spawn threads and start the static ratio test as discussed in Canvas
    // use the same output format as in milestone2 (method, timeWrapper, is left in this example).  
    //     Add thread ID as the first column.

    // call the specific function to time
    for (int i = 0; i < 10; i++) {
        float average = 0.0;
        float min = 0.0;
        float max = 0.0;
        int threadId = 1;

        // add more functions here
        getItemTest(config, cm);

        // write out the current values for this iteration
        auto curIterEnd = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = curIterEnd - start;
        //std::time_t iterEndTime = std::chrono::system_clock::to_time_t(curIterEnd);
        std::string timeString = "00:00:00";

        logToFileAndConsole(std::to_string(threadId) + "\t" + timeString + "\t" + std::to_string(i) + "\t\t" + std::to_string(average) + "\t" + std::to_string(min) + "\t" + std::to_string(max));
    }

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
