#include "JmsFitsReceiver.h"
#include <gmp/GMPKeys.h>
#include <src/util/PropertiesUtil.h>
#include <src/util/StringUtil.h>
#include <cms/BytesMessage.h>
#include <log4cxx/logger.h>

using namespace gmp;
using namespace cms;

namespace giapi {
   namespace gemini {
      namespace fitsFileImage {
         namespace jms {

            log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("giapi.JmsFitsReceiver"));

            JmsFitsReceiver::JmsFitsReceiver() noexcept(false)
                : JmsConsumer(GMPKeys::GMP_FITS_DESTINATION), _callback(nullptr), _isReceiving(false) {
            }

            JmsFitsReceiver::~JmsFitsReceiver() {
                if (_isReceiving) {
                    stopReceiving();
                }
            }

            std::shared_ptr<JmsFitsReceiver> JmsFitsReceiver::create() noexcept(false) {
                return std::shared_ptr<JmsFitsReceiver>(new JmsFitsReceiver());
            }

            int JmsFitsReceiver::startReceiving(void (*callback)(const FitsData&)) noexcept(false) {
                if (_isReceiving) {
                    LOG4CXX_WARN(logger, "Already receiving FITS files");
                    return status::ERROR;
                }

                if (!callback) {
                    LOG4CXX_ERROR(logger, "Callback function cannot be null");
                    return status::ERROR;
                }

                try {
                    _callback = callback;
                    startConsumer();
                    _isReceiving = true;
                    LOG4CXX_INFO(logger, "Started receiving FITS files");
                    return status::OK;
                } catch (const CommunicationException& e) {
                    LOG4CXX_ERROR(logger, "Failed to start FITS receiver: " << e.what());
                    return status::ERROR;
                }
            }

            void JmsFitsReceiver::stopReceiving() {
                if (!_isReceiving) {
                    return;
                }

                try {
                    stopConsumer();
                    _isReceiving = false;
                    _callback = nullptr;
                    LOG4CXX_INFO(logger, "Stopped receiving FITS files");
                } catch (const CommunicationException& e) {
                    LOG4CXX_ERROR(logger, "Error while stopping FITS receiver: " << e.what());
                }
            }

            void JmsFitsReceiver::onMessage(const Message* message) {
                if (!_isReceiving || !_callback) {
                    return;
                }

                try {
                    const BytesMessage* bytesMessage = dynamic_cast<const BytesMessage*>(message);
                    if (bytesMessage == nullptr) {
                        LOG4CXX_ERROR(logger, "Received message is not a BytesMessage");
                        return;
                    }
                    LOG4CXX_DEBUG(logger, "Received message body length: " << bytesMessage->getBodyLength() << " bytes");
                    // Get the message body size and data
                    const int size = bytesMessage->getBodyLength();
                    std::vector<unsigned char> buffer(size);
                    bytesMessage->readBytes(buffer.data(), size);

                    // Create FitsData structure and call the callback
                    FitsData fitsData(buffer.data(), size);
                    _callback(fitsData);
                    LOG4CXX_INFO(logger, "Successfully processed FITS file");

                } catch (const CMSException& e) {
                    LOG4CXX_ERROR(logger, "Error processing received FITS message: " << e.what());
                } catch (const std::exception& e) {
                    LOG4CXX_ERROR(logger, "Error processing received FITS file: " << e.what());
                }
            }

         }
      }
   }
} 