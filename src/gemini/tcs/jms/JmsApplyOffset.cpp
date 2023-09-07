/*
 * JmsApplyOffset.cpp
 *
 *  Created on: Aug 17, 2023
 *      Author: framos
 */

#include "JmsApplyOffset.h"
#include <gmp/GMPKeys.h>
#include <src/util/PropertiesUtil.h>
#include <src/util/StringUtil.h>

using namespace gmp;

namespace giapi {

namespace gemini {

namespace tcs {

namespace jms {

JmsApplyOffset::JmsApplyOffset() throw (CommunicationException) :
                                 JmsProducer(GMPKeys::GMP_TCS_OFFSET_DESTINATION) {
	instName = giapi::util::PropertiesUtil::Instance().getProperty("gmp.instrument");
	if(giapi::util::StringUtil::isEmpty(instName)) {
		LOG4CXX_WARN(logger, "Not instrument set in the gmp.properties file. The dummyInst name is used by default" << instName);
		std::cout << "Not instrument set in the gmp.properties file. The dummyInst name is used by default" << instName;
		instName = "dummyInst";
	}
}

JmsApplyOffset::~JmsApplyOffset() {
	// TODO Auto-generated destructor stub
}

pTcsOffset JmsApplyOffset::create() throw (CommunicationException) {
	pTcsOffset tcsOffset(new JmsApplyOffset());
	return tcsOffset;
}

int JmsApplyOffset::sendOffset(const double p, const double q,
				   const OffsetType offsetType, const long timeout) throw (CommunicationException, TimeoutException) {

	BytesMessage * rMsg = NULL;
	int wasOffsetApplied = 0;
	try {
		//Create a message to do the request.
		rMsg = _session->createBytesMessage();
		//create temporary objects to get the answer
		TemporaryQueue * tmpQueue = _session->createTemporaryQueue();
		MessageConsumer * tmpConsumer = _session->createConsumer(tmpQueue);

		//define the destination for the service to provide an answer
		rMsg->setCMSReplyTo(tmpQueue);

		//send the request
		rMsg->writeDouble(p);
		rMsg->writeDouble(q);
		rMsg->writeInt(offsetType);
		std::cout<<"instName: " << instName << " offsetType: " << offsetType << std::endl;
		rMsg->setStringProperty("instName", instName);
		_producer->send(rMsg);
		//delete the request, not needed anymore
		delete rMsg;

		//and wait for the response, timing out if necessary.
		Message *reply = (timeout > 0) ? tmpConsumer->receive(timeout)
		    			: tmpConsumer->receive();

		tmpConsumer->close();
		delete tmpConsumer;

		tmpQueue->destroy();
		delete tmpQueue;

		if (reply != NULL) {
			const BytesMessage* bytesMessage = dynamic_cast<const BytesMessage*> (reply);
			if (bytesMessage == NULL)
				return status::ERROR;

			//it gets if the offset was applied
			wasOffsetApplied = bytesMessage->readBoolean();

		} else { //timeout .Throw an exception
				throw TimeoutException("Time out while waiting for TCS Offset executiong");
			}
		} catch (CMSException &e) {
			if (rMsg != NULL) {
				delete rMsg;
			}
			throw CommunicationException("Problem applying the Offset in the TCS.  " + e.getMessage());
		}
		return wasOffsetApplied;
}

} // jms    namespace
} // tcs    namespace
} // gemini namespace
} // giapi  namespace
