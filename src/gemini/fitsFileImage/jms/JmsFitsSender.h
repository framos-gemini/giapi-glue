#ifndef JMSFITSSENDER_H_
#define JMSFITSSENDER_H_

#include <giapi/giapiexcept.h>
#include <giapi/GeminiUtil.h>
#include <util/jms/JmsProducer.h>
#include <string>
#include <log4cxx/logger.h>

namespace giapi {
   namespace gemini {
      namespace fitsFileImage {
         namespace jms {

            extern log4cxx::LoggerPtr logger;

            class JmsFitsSender: public util::jms::JmsProducer {
            public:
               /**
                * Creates a new instance of the FITS file sender
                * @throw CommunicationException
                * If there is an issue establishing the connection
                * with the messaging system.
                */
               static std::shared_ptr<JmsFitsSender> create() noexcept(false);

               /**
               * @throw GiapiException
               * If there is an error sending the FITS data,
               *        possibly due to a timeout or connection failure.
               */
               int sendFitsData(const FitsData& fitsData, const long timeout) const noexcept(false);

               /**
                  * @throw GiapiException
                  * If there is an error initiating the FITS data transfer,
                  *        possibly due to connection issues.
                  */
               int sendFitsData(const FitsData& fitsData, 
                                 const long timeout,
                                 void (*callback)(int, std::string)) const noexcept(false);

               virtual ~JmsFitsSender();

            private:
               /**
                * Constructor
                * @throw CommunicationException
                * If the initialization fails due to a connection error.
                */
               JmsFitsSender() noexcept(false);

               /**
                * Callback handler for asynchronous operations
                */
               static void handleCallback(void (*callback)(int, std::string),
                                       MessageConsumer* consumer,
                                       TemporaryQueue* queue,
                                       int timeout);

               std::string instName;
            };

         }
      }
   }
}

#endif /* JMSFITSSENDER_H_ */ 