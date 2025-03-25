
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <chrono>
#include <sstream>
#include <cstring>


using namespace std::chrono;

class ScorpioData3 {
public:

    ScorpioData3 (const std::string &label) : dataLabel(label) { }

    ScorpioData3 (const std::vector<unsigned char> &binaryData) {
        deserialize(binaryData);
    }
    
    ScorpioData3 (const std::string &label, uint64_t ts) : dataLabel(label), timestamp(ts) {}

    std::string dataLabel;

    uint64_t timestamp;

    std::vector<unsigned char> data;

    std::vector<unsigned char> serialize() const {
        std::vector<unsigned char> buffer;
        size_t totalSize = sizeof(uint32_t) + dataLabel.size() + sizeof(timestamp) + sizeof(uint32_t) + data.size();
        buffer.resize(totalSize);

        size_t offset = 0;

        // Serialize dataLabel size
        uint32_t labelSize = dataLabel.size();
        std::memcpy(buffer.data() + offset, &labelSize, sizeof(labelSize));
        offset += sizeof(labelSize);

        // Serialize dataLabel
        std::memcpy(buffer.data() + offset, dataLabel.data(), labelSize);
        offset += labelSize;

        // Serialize timestamp
        std::memcpy(buffer.data() + offset, &timestamp, sizeof(timestamp));
        offset += sizeof(timestamp);

        // Serialize data size
        uint32_t dataSize = data.size();
        std::memcpy(buffer.data() + offset, &dataSize, sizeof(dataSize));
        offset += sizeof(dataSize);

        // Serialize raw binary data
        std::memcpy(buffer.data() + offset, data.data(), dataSize);

        return buffer;
    }
    
    void deserialize(const std::vector<unsigned char> &binaryData) {
        size_t offset = 0;

        // Deserialize dataLabel size
        uint32_t labelSize;
        std::memcpy(&labelSize, binaryData.data() + offset, sizeof(labelSize));
        offset += sizeof(labelSize);

        // Deserialize dataLabel
        dataLabel.assign(reinterpret_cast<const char*>(binaryData.data() + offset), labelSize);
        offset += labelSize;

        // Deserialize timestamp
        std::memcpy(&timestamp, binaryData.data() + offset, sizeof(timestamp));
        offset += sizeof(timestamp);

        // Deserialize data size
        uint32_t dataSize;
        std::memcpy(&dataSize, binaryData.data() + offset, sizeof(dataSize));
        offset += sizeof(dataSize);

        // Deserialize raw binary data
        data.resize(dataSize);
        std::memcpy(data.data(), binaryData.data() + offset, dataSize);
    }

    static uint64_t getCurrentTimestamp() {
        
        return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    }
};


