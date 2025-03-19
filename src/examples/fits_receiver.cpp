#include "ScorpioData.h"
#include <giapi/InstTransferData.h>
#include <giapi/giapiexcept.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <csignal>
#include <atomic>

// Structure to hold timing data
struct PerfData {
    std::string dataLabel;
    uint64_t fileSize;
    uint64_t networkTime;
    uint64_t deserializeTime;
};

// Atomic flag to detect Ctrl+C
std::atomic<bool> running(true);

// Store results
std::vector<PerfData> perfResults;

// Logger for this example
static log4cxx::LoggerPtr exampleLogger(log4cxx::Logger::getLogger("giapi.examples.fits_receiver"));

std::string convertTimestampToHuman(uint64_t timestamp) {
    using namespace std::chrono;

    // Convert uint64_t (microseconds) to std::chrono::system_clock::time_point
    microseconds usTimestamp(timestamp);
    system_clock::time_point tp(usTimestamp);

    // Convert to time_t (seconds)
    std::time_t timeT = system_clock::to_time_t(tp);
    
    // Convert to local time structure
    std::tm localTm;
    localtime_r(&timeT, &localTm);

    // Format into human-readable string
    std::ostringstream oss;
    oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S") << "."  << (timestamp % 1000000);  // Append microseconds

    return oss.str();
}

void createFitsFile(ScorpioData data) {
    std::string filename = "received_" + data.dataLabel + ".fits";
        
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        LOG4CXX_DEBUG(exampleLogger, "Failed to create file: " << filename);
        return;
    }


    LOG4CXX_DEBUG(exampleLogger, "Attempting to write " << data.data.size() << " bytes to file");
    
    // Try writing in smaller chunks to identify where it fails
    const size_t chunkSize = 1024 * 1024; // 1MB chunks
    size_t totalWritten = 0;
    
    while (totalWritten < data.data.size()) {
        size_t remaining = data.data.size() - totalWritten;
        size_t currentChunk = std::min(chunkSize, remaining);
        
        if (!outFile.write(reinterpret_cast<const char*>(data.data.data() + totalWritten), currentChunk)) {
            LOG4CXX_ERROR(exampleLogger, "Failed to write chunk at offset " << totalWritten << ". Error: " << strerror(errno));
            outFile.close();
            return;
        }
        totalWritten += currentChunk;
    }
    
    outFile.close();
}

void handleFitsData2(const std::vector<unsigned char>& binaryData, u_int64_t tsDelayMessage) {
   
    try {
        uint64_t tsReceived = ScorpioData::getCurrentTimestamp();
        std::cout<< "###### Arrived to the binary data, the length is " << binaryData.size() << " NetworkDelay: " << tsDelayMessage/1000 << " ms" << std::endl;
        ScorpioData data(binaryData);
        uint64_t ts2 = ScorpioData::getCurrentTimestamp();
        uint64_t tsDeserialization = ts2 - tsReceived;
        LOG4CXX_DEBUG(exampleLogger, "Time spent in Deserialize: " << tsDeserialization / 1000   << " ms");
        //createFitsFile(data);
        uint64_t ts3 = ScorpioData::getCurrentTimestamp();
        LOG4CXX_DEBUG(exampleLogger, "Successfully saved FITS file, Spent in saving the fits file: " << (ts3 - ts2) / 1000 << " ms");
        // Store results
        perfResults.push_back({data.dataLabel, data.data.size(), tsDelayMessage, tsDeserialization});
        std::cout<< ";;;;;;;;;;;;; Processing everything " << (ts3 - tsReceived) / 1000 << " ms" << std::endl;

    // Log performance
        
    } catch (const std::exception& e) {
        LOG4CXX_ERROR(exampleLogger, "Error processing FITS data: " << e.what());
    }
    
} 

// Callback function to handle received FITS data
void handleFitsData(const std::vector<unsigned char>& binaryData) {
    std::cout<< "###### Arrived to the binary data, the length is " << binaryData.size() << std::endl;
    ScorpioData data(binaryData);
    uint64_t ts2 = ScorpioData::getCurrentTimestamp();
    uint64_t tsDeserialization = ts2 - data.timestamp;
    LOG4CXX_DEBUG(exampleLogger, "Time spent in Deserialize: " << tsDeserialization / 1000   << " ms");
    createFitsFile(data);
    uint64_t ts3 = ScorpioData::getCurrentTimestamp();
    LOG4CXX_DEBUG(exampleLogger, "Successfully saved FITS file, Spent in saving the fits file: " << (ts3 - ts2) / 1000 << " ms");
    // Store results
    perfResults.push_back({data.dataLabel, data.data.size(), data.timestamp/1000, tsDeserialization});
    std::cout<< ";;;;;;;;;;;;; Processing everything " << (ts3 - data.timestamp) / 1000 << " ms" << std::endl;
}

// Signal handler for Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        LOG4CXX_INFO(exampleLogger, "Ctrl+C detected! Saving results and exiting...");
        running = false; // Set flag to exit loop
    }
}

// Function to save results to a CSV file
void saveResultsToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    outFile << "Data Label; Data Size;Network Time (ms);Deserialize Time (ms)\n";
    for (const auto& data : perfResults) {
        outFile << data.dataLabel << ";"
                << data.fileSize << ";"
                << (data.networkTime)/1000 << ";"
                << (data.deserializeTime) / 1000 << "\n";
    }
    outFile.close();
    LOG4CXX_INFO(exampleLogger, "Saved performance results to " << filename);
}

// Thread function to subscribe to a detector
void subscribeToDetector(const std::string& detID) {
    try {
        LOG4CXX_INFO(exampleLogger, "Starting subscription for: " << detID);
        giapi::InstTransferData::receiveImage(detID, handleFitsData2);
    } catch (const std::exception& e) {
        LOG4CXX_ERROR(exampleLogger, "Error in subscription for " << detID << ": " << e.what());
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <fits_file_1> ...  <fits_file_n>" << std::endl;
        return 1;
    }
    try {
        // Configure logging
        //log4cxx::BasicConfigurator::configure();
        
        LOG4CXX_INFO(exampleLogger, "Starting FITS receiver...");
        LOG4CXX_INFO(exampleLogger, "Waiting for FITS files from scorpio...");
        
        // Register signal handler for Ctrl+C
        std::signal(SIGINT, signalHandler);
        std::vector<std::thread> threads;
        
        // Subscribe to each detector in a separate thread
        for (int i = 1; i < argc; i++) {
            threads.emplace_back(subscribeToDetector, std::string(argv[i]));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Small delay for better scheduling
        }

        // Keep the program running
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Gracefully exit all threads
        LOG4CXX_INFO(exampleLogger, "Stopping subscriptions...");
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        /*

        // Subscribe to receive FITS data from scorpio
        std::thread thread1(subscribeToDetector, "detH1");
        std::cout<<"Sleeping" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::thread thread2(subscribeToDetector, "detH2");
        
       // List of detector IDs
        std::vector<std::string> detectorIDs = {"detH1", "detH2"};
        //subscribeToDetector(std::string(argv[1]));
        //if (argc > 2)
        //    subscribeToDetector(std::string(argv[2]));
        
        //subscribeToDetector("detH2");
        // Launch separate threads for each detector subscription
        //std::vector<std::thread> threads;
        //for (const auto& detID : detectorIDs) {
        //    threads.emplace_back(subscribeToDetector, detID);
        //}
        
        // Keep the program running
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout<<"Going to finish the threads" << std::endl;
        */
        // Join threads before exiting
        //for (auto& thread : threads) {
        //    if (thread.joinable()) {
        //        thread.join();
        //    }
        //}
        std::cout<<"Saving results" << std::endl;
        // Save performance results before exiting
        saveResultsToFile("receiver_performance_" + std::string(convertTimestampToHuman(ScorpioData::getCurrentTimestamp())) + ".csv");
        
    } catch (const giapi::GiapiException& e) {
        LOG4CXX_ERROR(exampleLogger, "GiapiException: " << e.what());
        return 1;
    } catch (const std::exception& e) {
        LOG4CXX_ERROR(exampleLogger, "Exception: " << e.what());
        return 1;
    }
    
    return 0;
} 