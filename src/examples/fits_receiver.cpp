#include "ScorpioData.h"
#include <giapi/InstTransferData.h>
#include <giapi/giapiexcept.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <csignal>
#include <atomic>
#include <filesystem>
#include <cstring>
#include <map>

using namespace std;

map<string, unique_ptr<ScorpioData>> mapData;

// Structure to hold performance data
struct PerfData {
    std::string dataLabel;
    uint64_t fileSize;
    uint64_t networkTime;
    uint64_t deserializeTime;
};

std::atomic<bool> running(true);
std::vector<PerfData> perfResults;

// Save performance stats to file
void saveResultsToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    outFile << "Data Label; Data Size;Network Time (ms);Deserialize Time (ms)\n";
    for (const auto& data : perfResults) {
        outFile << data.dataLabel << ";"
                << data.fileSize << ";"
                << data.networkTime / 1000 << ";"
                << data.deserializeTime / 1000 << "\n";
    }
    outFile.close();
    cout << "Saved performance results to " << filename << endl;
}

// Ctrl+C signal handler
void signalHandler(int signal) {
    if (signal == SIGINT) {
        cout << "Ctrl+C detected, stopping receiver..."<< endl;
        running = false;
    }
}

void writeToFile(const std::string& filename, const std::vector<unsigned char>& buffer) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        cout << "Unable to write buffer to " << endl;
        return;
    }
    outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outFile.close();
    cout << "exampleLogger, Saved ScorpioData to " << filename<< endl;
}

// Handle received FITS binary data
void handleFitsData(const std::vector<unsigned char>& binaryData, u_int64_t tsDelayMessage) {
    try {
        uint64_t tsReceived = ScorpioData::getCurrentTimestamp();
        ScorpioData received(binaryData);
        uint64_t tsDeserialized = ScorpioData::getCurrentTimestamp();
        uint64_t deserTime = tsDeserialized - tsReceived;
        if (!mapData.count(received.dataLabel))
           mapData.insert({received.dataLabel, make_unique<ScorpioData>(received)});
        
        cout << "Received data: " << received.dataLabel 
                                    << ", size = " << received.data.size() << ", net = " << tsDelayMessage/1000 << " ms" 
                                    << ", deser = " << deserTime / 1000 << " ms"<<endl;

        perfResults.push_back({received.dataLabel, received.data.size() * sizeof(float), tsDelayMessage, deserTime});
    } catch (const std::exception& e) {
        cout << "exampleLogger, Exception during message processing: " << e.what() << endl;
    }
}

int main(int argc, char* argv[]) {
    try {
        string detName="detH";

        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << "<number of detector to subscriber>" << std::endl;
            return 1;
        }
        std::signal(SIGINT, signalHandler);
        int numberDet = atoi(argv[1]);

        cout << "Starting FITS receiver..."<< endl;

        std::vector<std::thread> threads;
        for (int i=0; i<numberDet; ++i) {
            string det = detName+ std::to_string(i);
            threads.emplace_back([det]() {
                giapi::InstTransferData::receiveImage(det, handleFitsData);
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        for (auto& thread : threads) {
            if (thread.joinable()) thread.join();
        }

        std::ostringstream filename;
        auto now = std::chrono::system_clock::now();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
                          now.time_since_epoch()).count();
        filename << "receiver_performance_" << micros << ".csv";
        saveResultsToFile(filename.str());
        
        //writeToFile("scorpio_data1_received.bin", data_received->serialize());
        //writeToFile("scorpio_data2_received.bin", data_received2->serialize());
        
        for (auto it = mapData.begin(); it != mapData.end(); ++it) {
            it->second->saveToAsciiFile("/tmp/"+it->second->dataLabel+"received.txt");           
        }
        

    } catch (const giapi::GiapiException& e) {
        cout << "GiapiException: " << e.what()<< endl;
        return 1;
    } catch (const std::exception& e) {
        cout << "Exception: " << e.what()<< endl;
        return 1;
    }

    return 0;
}
