#include "JmsFitsSender.h"
#include <gmp/GMPKeys.h>
#include <src/util/PropertiesUtil.h>
#include <src/util/StringUtil.h>
#include <fstream>
#include <thread>
#include <cms/BytesMessage.h>
#include <cms/TextMessage.h>
#include <cms/TemporaryQueue.h>
#include <log4cxx/logger.h>
#include <iostream>

using namespace gmp;
using namespace cms;

namespace giapi {
   namespace gemini {
      namespace fitsFileImage {
         namespace jms {

            JmsFitsSender::JmsFitsSender() noexcept(false)
                                          : JmsProducer(GMPKeys::GMP_FITS_DESTINATION) {
               instName = giapi::util::PropertiesUtil::Instance().getProperty("gmp.instrument");
               if(giapi::util::StringUtil::isEmpty(instName)) {
                  LOG4CXX_WARN(logger, "No instrument set in the gmp.properties file. Using dummyInst by default");
                  instName = "dummyInst";
               }
            }

            JmsFitsSender::~JmsFitsSender() {
            }

            std::shared_ptr<JmsFitsSender> JmsFitsSender::create() noexcept(false) {
               return std::shared_ptr<JmsFitsSender>(new JmsFitsSender());
            }

            int JmsFitsSender::sendFitsData(const FitsData& fitsData, const long timeout) const noexcept(false) {
                try {
                    // Create a bytes message
                    auto message = std::unique_ptr<BytesMessage>(_session->createBytesMessage());
                    std::cout<<"Writting the messageeeeeeeeeeeeee"<< std::endl;
                    // Write the FITS data to the message
                    message->writeBytes(fitsData.data);
                    // Send the message
                    _producer->send(message.get());
                    LOG4CXX_INFO(logger, "Successfully sent FITS data");
                    return status::OK;

                } catch (const CMSException& e) {
                    LOG4CXX_ERROR(logger, "Error sending FITS data: " << e.what());
                    throw CommunicationException("Error sending FITS data: " + std::string(e.what()));
                }
            }

            int JmsFitsSender::sendFitsData(const FitsData& fitsData, 
                                          const long timeout,
                                          void (*callback)(int, std::string)) const noexcept(false) {
                try {
                    // Create a temporary queue for receiving the response
                    //auto tempQueue = std::unique_ptr<TemporaryQueue>(_session->createTemporaryQueue());
                    
                    // Create a bytes message
                    auto message = std::unique_ptr<BytesMessage>(_session->createBytesMessage());

                    std::cout<<"2222 Writting the messageeeeeeeeeeeeee"<< std::endl;
                    // Write the FITS data to the message
                    message->writeBytes(fitsData.data);
                    //LOG4CXX_DEBUG(logger, "2 This is the message body length before sending: " << message->getBodyLength() << " bytes");
                    // Set the reply-to destination
                   // message->setJMSReplyTo(tempQueue.get());

                    // Create a consumer for the temporary queue
                    //auto consumer = std::unique_ptr<MessageConsumer>(_session->createConsumer(tempQueue.get()));

                    // Send the message
                    _producer->send(message.get());
                    LOG4CXX_INFO(logger, "Successfully sent FITS data, waiting for response");

                    // Start a new thread to handle the response
                    /*std::thread([callback, consumer = consumer.release(), tempQueue = tempQueue.release(), timeout]() {
                        handleCallback(callback, consumer, tempQueue, timeout);
                    }).detach();*/

                    return status::OK;

                } catch (const CMSException& e) {
                    LOG4CXX_ERROR(logger, "Error sending FITS data: " << e.what());
                    throw CommunicationException("Error sending FITS data: " + std::string(e.what()));
                }
            }

            void JmsFitsSender::handleCallback(void (*callback)(int, std::string),
                                             MessageConsumer* consumer,
                                             TemporaryQueue* queue,
                                             int timeout) {
                try {
                    // Wait for the response
                    std::unique_ptr<Message> response(consumer->receive(timeout));
                    
                    if (response) {
                        // Process the response and call the callback
                        // Here you would extract the status and message from the response
                        // For now, we'll just assume success
                        callback(status::OK, "FITS data processed successfully");
                    } else {
                        callback(status::ERROR, "Timeout waiting for response");
                    }

                } catch (const CMSException& e) {
                    callback(status::ERROR, std::string("Error receiving response: ") + e.what());
                }

                // Clean up
                try {
                    consumer->close();
                    delete consumer;
                    queue->destroy();
                    delete queue;
                } catch (const CMSException& e) {
                    LOG4CXX_ERROR(logger, "Error cleaning up temporary resources: " << e.what());
                }
            }

         }
      }
   }
} 