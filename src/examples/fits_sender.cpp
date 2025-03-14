#include <giapi/GeminiUtil.h>
#include <giapi/giapiexcept.h>
#include <gemini/fitsFileImage/jms/JmsFitsSender.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>

using namespace giapi;
using namespace giapi::gemini::fitsFileImage::jms;

// Logger for this example
static log4cxx::LoggerPtr exampleLogger(log4cxx::Logger::getLogger("giapi.examples.fits_sender"));

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " <fits_file_path>" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        // Configure logging
        log4cxx::BasicConfigurator::configure();

        // Read the FITS file
        std::string fitsPath = argv[1];
        std::ifstream file(fitsPath, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
           LOG4CXX_ERROR(exampleLogger, "ERROR: Unable to open FITS file: " << fitsPath);
           perror("File open error");  // System-level error
           return 1;
        }

        std::streamsize size = file.tellg();
        if (size <= 0) {
            LOG4CXX_ERROR(exampleLogger, "ERROR: FITS file appears empty or unreadable.");
            return 1;
        }

        file.seekg(0, std::ios::beg);
        std::vector<unsigned char> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
           LOG4CXX_ERROR(exampleLogger, "ERROR: Failed to read FITS file data.");
           std::cerr << "ifstream state: fail=" << file.fail() << ", bad=" << file.bad() << ", eof=" << file.eof() << std::endl;
           return 1;
        }

        LOG4CXX_INFO(exampleLogger, "Successfully read FITS file: " << fitsPath);


        // Get file size
	/*
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read file content
        std::vector<unsigned char> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            LOG4CXX_INFO(exampleLogger, "Failed to read FITS file");
            return 1;
        }
        */
        // Create FitsData structure
        FitsData fitsData;
        fitsData.data = buffer;

        // Create FITS sender
        auto sender = JmsFitsSender::create();

        // Send the FITS data
        LOG4CXX_INFO(exampleLogger, "Sending FITS file: " << fitsPath);
        LOG4CXX_DEBUG(exampleLogger, "Sending FITS data of size: " << fitsData.data.size() << " bytes");
        int result = sender->sendFitsData(fitsData, 5000); // 5 second timeout

        sleep(2);
        if (result == status::OK) {
            LOG4CXX_INFO(exampleLogger, "Successfully sent FITS file");
        } else {
            LOG4CXX_INFO(exampleLogger, "Failed to send FITS file");
        }

    } catch (const GiapiException& e) {
        LOG4CXX_INFO(exampleLogger, "Error: " << e.what());
        return 1;
    }

    return 0;
} 
