#include "ScorpioData.h"
#include <giapi/InstTransferData.h>
#include <giapi/giapiexcept.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace std;
struct PerfData {
    std::string dataLabel;
    uint64_t fileSize;
    uint64_t serializationT;
    uint64_t networkTime;
};

std::vector<PerfData> perfResults;

void saveResultsToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    outFile << "Data Label; Data Size;Serialization Time (ms);Network Time (ms)\n";
    for (const auto& data : perfResults) {
        outFile << data.dataLabel << ";"
                << data.fileSize << ";"
                 << data.serializationT / 1000 << ";"
                << data.networkTime / 1000 << "\n";
    }
    outFile.close();
    cout<< "Saved performance results to " << filename<< endl;
}

void writeToFile(const std::string& filename, const std::vector<unsigned char>& buffer) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        cout<< "Unable to write buffer to " << filename<<endl;
        return;
    }
    outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outFile.close();
    cout<< "Saved ScorpioData to " << filename <<endl;
}

ScorpioData generateRandomScorpioData(const std::string& labelPrefix, int row, int col) {
    std::ostringstream oss;
    oss << labelPrefix;
    std::string label = oss.str();

    ScorpioData data(label, row, col);

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (auto& val : data.data) {
        val = dist(rng);
    }

    // Example WCS metadata
    data.wcs["radesys"] = "ICRS";
    data.wcs["equinox"] = "2000.0";
    data.wcs["ctype1"] = "RA--TAN";
    data.wcs["ctype2"] = "DEC-TAN";
    data.wcs["wat001"] = "This is a WCS";
    data.wcs["wat002"] = "";
    data.wcs["wat003"] = "";
    data.wcs["crval1"] = std::to_string(dist(rng));
    data.wcs["crval2"] = std::to_string(dist(rng));
    data.wcs["crpix1"] = std::to_string(row/2);
    data.wcs["crpix2"] = std::to_string(col/2);
    data.wcs["cd1_1"]  = std::to_string(dist(rng));
    data.wcs["cd1_2"]  = std::to_string(dist(rng));
    data.wcs["cd2_1"]  = std::to_string(dist(rng));
    data.wcs["cd2_2"]  = std::to_string(dist(rng));
    data.wcs["lonpole"] = "180";

    return data;
}

int main(int argc, char* argv[]) {
    if (argc > 7 || argc < 5) {
        cout<<"Argc " << argc << endl;
        std::cerr << "Usage1: " << argv[0] << " <num_messages> <number of detectors> <num data rows> <num data columns> [num data rows] [num data columns]" << std::endl;
        std::cerr << "Usage2: " << argv[0] << " <num_messages> <detector label> <num data rows> <num data columns>" << std::endl;
        return 1;
    }
    
    int numMessages = -1;
    int numDetectos = 1;
    int numRows = -1;
    int numCols = -1;
    int numRows2 = -1;
    int numCols2 = -1;
    string detLabel = "detH";

    try{
        numMessages = std::stoi(argv[1]);
        numDetectos = std::stoi(argv[2]);
        numRows2 = numRows = std::stoi(argv[3]);
        numCols2 = numCols = std::stoi(argv[4]);
	if (argc > 5) {
           numRows2     = std::stoi(argv[5]);
           numCols2     = std::stoi(argv[6]);
	}
    } catch (...) {
	try {
	   detLabel = argv[2];
           numRows2 = numRows = std::stoi(argv[3]);
           numCols2 = numCols = std::stoi(argv[4]);
        }catch (...) {

           cerr << "The second to seventh parameters must be a number" << endl;
           std::cerr << "Usage2: " << argv[0] << " <num_messages> <detector label> <num data rows> <num data columns>" << std::endl;
           //std::cerr << "Usage: " << argv[0] << " <num_messages> <number of detectors> <num data rows> <num data columns> [num data rows] [num data columns]" << std::endl;
           return 1;
	}
    }

    vector<ScorpioData> vectData;

    cout<< "Preparing ScorpioData structures. Number of Detectos: "<< numDetectos << endl;

    for (int i=0; i < numDetectos; ++i) {
	string dLabel = (numDetectos > 1) ? detLabel+to_string(i) : detLabel;
        ScorpioData dataTmp = (i%2 == 0) ? 
                              generateRandomScorpioData(dLabel, numRows, numCols) :
                              generateRandomScorpioData(dLabel, numRows2, numCols2);
        dataTmp.dataSerialized = dataTmp.serialize();
        cout<<"Created the " << dataTmp.dataLabel << " detector "<< endl;
        vectData.push_back(dataTmp);
    }

    cout<< "Starting FITS transmission loop..."<<endl;

    for (int i = 0; i < numMessages; ++i) {
        int nDet = i % numDetectos;
        ScorpioData& selected = vectData[nDet];

        std::ostringstream oss;
        //oss << (isEven ? "detH1" : "detH2") << "_" << (isEven ? i / 2 : i / 2);
        //selected.dataLabel = oss.str();

        uint64_t tsStartSerialization = ScorpioData::getCurrentTimestamp();
        //selected.timestamp = tsStartSerialization;
        std::vector<unsigned char> payload = selected.serialize();
        uint64_t tsEndSerialization = ScorpioData::getCurrentTimestamp();
        giapi::InstTransferData::sendImage(selected.dataLabel, payload, false);
        uint64_t tsEnd = ScorpioData::getCurrentTimestamp();

        perfResults.push_back({selected.dataLabel, payload.size(), tsEndSerialization - tsStartSerialization, tsEnd - tsEndSerialization});
        cout<< "Msg: " << i << " dataLabel: " << selected.dataLabel << " (" << payload.size() << " bytes)" 
                     << "Serialization took: " << (tsEndSerialization - tsStartSerialization) / 1000 << " ms\n"<<endl;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_r(&now_time, &local_tm);
    std::ostringstream filename;
    filename << "sender_performance_" << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S") << ".csv";
    saveResultsToFile(filename.str());
    for (auto it : vectData)
        it.saveToAsciiFile("/tmp/"+it.dataLabel +"_sent.txt");
    
    cout<< "All FITS messages sent successfully."<<endl;
    return 0;
}
