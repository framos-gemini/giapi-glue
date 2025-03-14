#include <giapi/GeminiUtil.h>
#include <giapi/giapiexcept.h>
#include <gemini/fitsFileImage/jms/JmsFitsReceiver.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <iostream>
#include <fstream>
#include <string>
#include <csignal>
#include <thread>
#include <cstring>

using namespace giapi;
using namespace giapi::gemini::fitsFileImage::jms;

// Logger for this example
static log4cxx::LoggerPtr exampleLogger(log4cxx::Logger::getLogger("giapi.examples.fits_receiver"));

// Global flag to control the program
static bool running = true;

// Signal handler
void signalHandler(int signum) {
    LOG4CXX_INFO(exampleLogger, "Received signal " << signum << ". Stopping...");
    running = false;
}

// Callback function to handle received FITS data
void handleFitsData(const FitsData& fitsData) {
    static int fileCounter = 0;
    std::string filename = "received_fits_" + std::to_string(fileCounter++) + ".fits";
    
    try {
        LOG4CXX_DEBUG(exampleLogger, "Received FITS data with size: " << fitsData.data.size());
        LOG4CXX_DEBUG(exampleLogger, "Data pointer: " << (void*)fitsData.data.data());
        
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile.is_open()) {
            LOG4CXX_INFO(exampleLogger, "Failed to create file: " << filename);
            return;
        }

        LOG4CXX_DEBUG(exampleLogger, "Attempting to write " << fitsData.data.size() << " bytes to file");
        
        // Try writing in smaller chunks to identify where it fails
        const size_t chunkSize = 1024 * 1024; // 1MB chunks
        size_t totalWritten = 0;
        
        while (totalWritten < fitsData.data.size()) {
            size_t remaining = fitsData.data.size() - totalWritten;
            size_t currentChunk = std::min(chunkSize, remaining);
            
            if (!outFile.write(reinterpret_cast<const char*>(fitsData.data.data() + totalWritten), currentChunk)) {
                LOG4CXX_ERROR(exampleLogger, "Failed to write chunk at offset " << totalWritten << ". Error: " << strerror(errno));
                outFile.close();
                return;
            }
            totalWritten += currentChunk;
        }
        
        outFile.close();
        LOG4CXX_INFO(exampleLogger, "Saved FITS data to file: " << filename);
    } catch (const std::exception& e) {
        LOG4CXX_INFO(exampleLogger, "Error saving FITS file: " << e.what());
    }
}

int main(int argc, char* argv[]) {
    try {
        // Configure logging
        log4cxx::BasicConfigurator::configure();

        // Set up signal handlers
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // Create FITS receiver
        auto receiver = JmsFitsReceiver::create();

        // Start receiving FITS data
        LOG4CXX_INFO(exampleLogger, "Starting to receive FITS data...");
        int result = receiver->startReceiving(handleFitsData);

        if (result != status::OK) {
            LOG4CXX_ERROR(exampleLogger, "Failed to start receiving FITS data");
            return 1;
        }

        LOG4CXX_INFO(exampleLogger, "Press Ctrl+C to stop...");

        // Keep the program running
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Stop receiving FITS data
        receiver->stopReceiving();
        LOG4CXX_INFO(exampleLogger, "Stopped receiving FITS data");

    } catch (const GiapiException& e) {
        LOG4CXX_ERROR(exampleLogger, "Error: " << e.what());
        return 1;
    }

    return 0;
} 