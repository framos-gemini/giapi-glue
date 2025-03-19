#include "transferdata/ImgTransferDataImpl.h"

#include "transferdata/img_publisher/jms/JmsImgPublisher.h"
#include "transferdata/img_subscriber/jms/JmsImgSubscriber.h"

#include <iostream>

namespace giapi {

    // Define static members
    //transferdata::img_publisher::pImgPublisher ImgTransferDataImpl::_imgPublisher;
    //transferdata::img_subscriber::pImgSubscriber ImgTransferDataImpl::_imgSubscriber;
    std::map<std::string, transferdata::img_subscriber::pImgSubscriber> ImgTransferDataImpl::_subscribers;
    std::map<std::string, transferdata::img_publisher::pImgPublisher> ImgTransferDataImpl::_publishers;
    log4cxx::LoggerPtr ImgTransferDataImpl::logger(log4cxx::Logger::getLogger("giapi.ImgTransferDataImpl"));

    //pImgTransferImpl ImgTransferDataImpl::INSTANCE(static_cast<ImgTransferDataImpl *>(0));
    
    ImgTransferDataImpl::ImgTransferDataImpl() noexcept(false) { }
    

/**
 * @brief Sends an image via JmsImgPublisher.
 */
int ImgTransferDataImpl::sendImage(const std::string& detID, const std::vector<unsigned char>& binaryData, bool blocking) noexcept(false) {
    try {
        auto it = _publishers.find(detID);
        transferdata::img_publisher::pImgPublisher publisher;
        if (it == _publishers.end()) {
            publisher = giapi::transferdata::img_publisher::jms::JmsImgPublisher::create(detID);
            _publishers.insert({detID, std::move(publisher)});
        } else
            publisher = it->second;
        
        return publisher->sendImage(binaryData, blocking);

        //if (!_imgPublisher ) 
        //    _imgPublisher = giapi::transferdata::img_publisher::jms::JmsImgPublisher::create(detID);
        //return _imgPublisher->sendImage(binaryData, blocking);

    } catch (const std::exception& e) {
        std::cerr << "Error in sendImage: " << e.what() << std::endl;
        return -1;
    }
}

/**
 * @brief Receives an image asynchronously via ImgSubscriber.
 */
int ImgTransferDataImpl::receiveImage(const std::string& detID, void (*callback)(const std::vector<unsigned char>&)) noexcept(false) {
    auto it = _subscribers.find(detID);
    if (it != _subscribers.end()) {
        LOG4CXX_WARN(logger, "Subscription for " << detID << " already exists. Skipping duplicate subscription.");
        return 0;  // Avoid creating a duplicate subscriber
    }
    try {
        // Create an instance of the image subscriber
        //if (!_imgSubscriber)
        //    _imgSubscriber = giapi::transferdata::img_subscriber::jms::JmsImgSubscriber::create(detID);
        //return _imgSubscriber->receiveImage(detID, callback);
        LOG4CXX_INFO(logger, "Creating new subscription for detector ID: " << detID);
        auto subscriber = transferdata::img_subscriber::jms::JmsImgSubscriber::create(detID);
        int error = subscriber->receiveImage(detID, callback);
        _subscribers.insert({detID, std::move(subscriber)});  // Store the subscriber
        return error;

    } catch (const std::exception& e) {
        std::cerr << "Error in receiveImage (callback mode): " << e.what() << std::endl;
        return -1;
    }
}
int ImgTransferDataImpl::receiveImage(const std::string& detID, void (*callback)(const std::vector<unsigned char>&, u_int64_t)) noexcept(false) {
    auto it = _subscribers.find(detID);
    if (it != _subscribers.end()) {
        LOG4CXX_WARN(logger, "Subscription for " << detID << " already exists. Skipping duplicate subscription.");
        return 0;  // Avoid creating a duplicate subscriber
    }
    try {
        // Create an instance of the image subscriber
        //if (!_imgSubscriber)
        //    _imgSubscriber = giapi::transferdata::img_subscriber::jms::JmsImgSubscriber::create(detID);
        //return _imgSubscriber->receiveImage(detID, callback);
        LOG4CXX_INFO(logger, "Creating new subscription for detector ID: " << detID);
        auto subscriber = transferdata::img_subscriber::jms::JmsImgSubscriber::create(detID);
        int error = subscriber->receiveImage(detID, callback);
        _subscribers.insert({detID, std::move(subscriber)});  // Store the subscriber
        return error;

    } catch (const std::exception& e) {
        std::cerr << "Error in receiveImage (callback mode): " << e.what() << std::endl;
        return -1;
    }
}

}