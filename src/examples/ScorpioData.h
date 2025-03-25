
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <chrono>
#include <sstream>
#include <cstring>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>


using namespace std::chrono;

class ScorpioData {
public:

    std::string dataLabel;  // Used as filename
    uint64_t timestamp;     // Microsecond precision
    int _row;
    int _col;
    std::map<std::string, std::string> wcs;
    std::vector<float> data;  // 1024 x 1024 matrix (1D)

  

    ScorpioData() : timestamp(getCurrentTimestamp()), _row(1024), _col(1024), data(1024 * 1024, 0.0f) {}

    ScorpioData(const std::string& label, int row = 1024, int col = 1024)
        : dataLabel(label), timestamp(getCurrentTimestamp()),  _row(row), _col(col), data(row * col, 0.0f) {}

    ScorpioData(const std::string& label, uint64_t ts, int row = 1024, int col = 1024)
        : dataLabel(label), timestamp(ts), _row(row), _col(col), data(row * col, 0.0f) {}

    ScorpioData(const std::vector<unsigned char>& binaryData) {
        deserialize(binaryData);
    }


    void saveToAsciiFile(const std::string& filename) const {
        std::cout<<"################# file: "<< dataLabel << " _col: " << _col << " _row: " << _row << std::endl;
        std::ofstream out(filename);
        //out << std::fixed << std::setprecision(20);
        if (!out.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }
    
        // Write metadata
        out << "DataLabel: " << dataLabel << "\n";
        out << "Timestamp: " << timestamp << "\n";
        out << "WCS:\n";
        for (const auto& pair : wcs) {
            out << "  " << pair.first << ": " << pair.second << "\n";
        }
    
        // Write matrix dimensions and data
        out << "DataSize: " << data.size() << "\n";
        out << "Number of row: " << _row  << " number of column: " << _col << " \n";
        out << "Data\n";
        for (int row = 0; row < _row; ++row) {
            out << "row " << row << " \n";
            for (int col = 0; col < _col; ++col) {
                //std::cout<<"i: "<< row << " j: "<< col<< std::endl;
                out << data[row * _col + col];
                if (!out) {
                    std::cerr << "Stream failed at row=" << row
                              << ", col=" << col
                              << ", value=" << data[row * _col + col]
                              << std::endl;
                    break;
                }
                if (col != _col - 1) out << " ";
            }
            if (!out) {
                std::cerr << "Stream went bad after row " << row << std::endl;
            }
            out << "\n";
        }
    
        out.close();
    }

    std::vector<unsigned char> serialize() const {
        // The serialization will be
        // <datalabel numBytes><dataLabel_bytes><timestamp_numBytes><wcs numBytes><key_numBytes><key_bytes><value_numBytes><value_bytes>.....<matrix_numBytes><matrix_bytes>
        size_t totalSize = sizeof(uint32_t) + dataLabel.size() +
                            sizeof(uint32_t) * 2 +  // _row, _col
                            sizeof(uint64_t) +  // timestamp
                            sizeof(uint32_t);   // WCS map size

        for (const auto& kv : wcs) {
            totalSize += sizeof(uint32_t) + kv.first.size();
            totalSize += sizeof(uint32_t) + kv.second.size();
        }
    
        totalSize += data.size() * sizeof(float);
    
        std::vector<unsigned char> buffer(totalSize);
        size_t offset = 0;
    
        auto writeString = [&](const std::string& str) {
            uint32_t size = static_cast<uint32_t>(str.size());
            std::memcpy(buffer.data() + offset, &size, sizeof(size)); offset += sizeof(size);
            std::memcpy(buffer.data() + offset, str.data(), size);    offset += size;
        };
    
        writeString(dataLabel);
        std::memcpy(buffer.data() + offset, &timestamp, sizeof(timestamp)); offset += sizeof(timestamp);
    
        std::memcpy(buffer.data() + offset, &_row, sizeof(_row)); offset += sizeof(_row);
        std::memcpy(buffer.data() + offset, &_col, sizeof(_col)); offset += sizeof(_col);
    
        uint32_t mapSize = static_cast<uint32_t>(wcs.size());
        std::memcpy(buffer.data() + offset, &mapSize, sizeof(mapSize)); offset += sizeof(mapSize);
    
        for (const auto& kv : wcs) {
            writeString(kv.first);
            writeString(kv.second);
        }
    
        std::memcpy(buffer.data() + offset, data.data(), data.size() * sizeof(float));
    
        return buffer;
    }

    void deserialize(const std::vector<unsigned char>& buffer) {
        size_t offset = 0;

        auto readString = [&](std::string& str) {
            uint32_t size;
            std::memcpy(&size, buffer.data() + offset, sizeof(size)); offset += sizeof(size);
            str.assign(reinterpret_cast<const char*>(buffer.data() + offset), size); offset += size;
        };

        readString(dataLabel);
        std::memcpy(&timestamp, buffer.data() + offset, sizeof(timestamp)); offset += sizeof(timestamp);

        std::memcpy(&_row, buffer.data() + offset, sizeof(_row)); offset += sizeof(_row);
        std::memcpy(&_col, buffer.data() + offset, sizeof(_col)); offset += sizeof(_col);

        uint32_t mapSize;
        std::memcpy(&mapSize, buffer.data() + offset, sizeof(mapSize)); offset += sizeof(mapSize);

        wcs.clear();
        for (uint32_t i = 0; i < mapSize; ++i) {
            std::string key, value;
            readString(key);
            readString(value);
            wcs[key] = value;
        }

        size_t dataSize = static_cast<size_t>(_row) * _col;
        data.resize(dataSize);
        std::memcpy(data.data(), buffer.data() + offset, dataSize * sizeof(float));
    }

    static uint64_t getCurrentTimestamp() {
        
        return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    }
};


