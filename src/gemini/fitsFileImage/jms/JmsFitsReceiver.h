#ifndef JMSFITSRECEIVER_H_
#define JMSFITSRECEIVER_H_

#include <giapi/giapiexcept.h>
#include <giapi/GeminiUtil.h>
#include <util/jms/JmsConsumer.h>
#include <string>
#include <memory>
#include <log4cxx/logger.h>

namespace giapi {
   namespace gemini {
      namespace fitsFileImage {
         namespace jms {

            extern log4cxx::LoggerPtr logger;

            class JmsFitsReceiver: public util::jms::JmsConsumer {
            public:
               /**
                * Creates a new instance of the FITS file receiver
                * @throw CommunicationException
                * If there is an issue establishing the connection
                * with the messaging system.
                */
               static std::shared_ptr<JmsFitsReceiver> create() noexcept(false);

               /**
                * Starts listening for incoming FITS files
                * @param callback Function to be called when a FITS file is received
                * @return status::OK if subscription was successful, status::ERROR otherwise
                * @throw CommunicationException If there is an issue with the subscription
                */
               int startReceiving(void (*callback)(const FitsData&)) noexcept(false);

               /**
                * Stops listening for FITS files
                */
               void stopReceiving();

               /**
                * Callback function that will be called when a message is received
                * @param message The received JMS message
                */
               virtual void onMessage(const Message* message);

               virtual ~JmsFitsReceiver();

            private:
               /**
                * Constructor
                * @throw CommunicationException
                * If the initialization fails due to a connection error.
                */
               JmsFitsReceiver() noexcept(false);

               void (*_callback)(const FitsData&);
               bool _isReceiving;
            };

         }
      }
   }
}

#endif /* JMSFITSRECEIVER_H_ */ 