#include <giapi/InstTransferData.h>
#include "transferdata/ImgTransferDataImpl.h"

namespace giapi {

    int InstTransferData::receiveImage(const std::string& detID, void (*callback)(const std::vector<unsigned char>&)) noexcept(false) {
        return ImgTransferDataImpl::receiveImage(detID, callback, false);
    }

    int InstTransferData::receiveImage(const std::string& detID, void (*callback)(const std::vector<unsigned char>&, uint64_t)) noexcept(false) {
        return ImgTransferDataImpl::receiveImage(detID, callback, false);
    }

    int InstTransferData::receiveImage(const std::string& detID, void (*callback)(const std::vector<unsigned char>&), const bool ack) noexcept(false) {
        return ImgTransferDataImpl::receiveImage(detID, callback, ack);
    }

    int InstTransferData::receiveImage(const std::string& detID, void (*callback)(const std::vector<unsigned char>&, uint64_t), const bool ack) noexcept(false) {
        return ImgTransferDataImpl::receiveImage(detID, callback, ack);
    }


    int InstTransferData::sendImage(const std::string& detID, const std::vector<unsigned char>& binaryData, bool blocking) noexcept(false) {
        return ImgTransferDataImpl::sendImage(detID, binaryData, blocking);
    }


} // namespace giapi
