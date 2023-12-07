#ifndef INSTCMDHANDLER_H_
#define INSTCMDHANDLER_H_

#include <iostream>
#include <giapi/giapi.h>
#include <giapi/SequenceCommandHandler.h>
#include <unistd.h>
#include "DataResponse.h"
unsigned int microsecond = 1000000;


using namespace giapi;
using namespace std;

namespace instDummy {

   // This pointer function will be the python function callback which will be called when a subscribed command is received. 
   typedef DataResponse (*callback_def)(giapi::command::ActionId, giapi::command::SequenceCommand, giapi::command::Activity, giapi::pConfiguration);

   class InstCmdHandler: public giapi::SequenceCommandHandler {

	private:
		/*Private attributes and methods */
		callback_def callback_f;

	protected:

	public:

		virtual giapi::pHandlerResponse handle(giapi::command::ActionId id,
						       giapi::command::SequenceCommand sequenceCommand,
						       giapi::command::Activity activity,
						       giapi::pConfiguration config) {

			//decaf::util::concurrent::CountDownLatch lock(1);
			if (config != NULL) {
				vector<string> keys = config->getKeys();
				vector<string>::iterator it = keys.begin();
				cout << "Instrument Configuration parameters received" << endl;
				for (; it < keys.end(); it++) {
					cout << "{" << *it << " : " << config->getValue(*it) << "}" << std::endl;
				}
			}
			
			switch (sequenceCommand)
			{
				case (giapi::command::REBOOT):
					cout << "The REBOOT command has been received. REBOOT the observation" << endl;
					break;
				case (giapi::command::INIT):
					cout << "The INIT command has been received. INIT the observation" << endl;
					break;
				case (giapi::command::DATUM):
					cout << "The DATUM command has been received. DATUM the observation" << endl;
					break;
				case (giapi::command::PARK):
					cout << "PARK " << endl;
					break;
				case (giapi::command::VERIFY):
					cout << "VERIFY" << endl;
					break;
				case (giapi::command::END_VERIFY):
					cout << "END_VERIFY" << endl;
					break;
				case (giapi::command::GUIDE):
					cout << "The GUIDE command has been received. GUIDING the observation" << endl;
					break;
				case (giapi::command::END_GUIDE):
					cout << "The END_GUIDE command has been received. finishing the observation" << endl;
					break;
				case (giapi::command::APPLY):
					cout << "Then APPLY command has been received" << endl;
					//return HandlerResponse::create(HandlerResponse::ERROR);
					break;
				case (giapi::command::OBSERVE):
					cout << "The OBSERVE command has been received. OBSERVE the observation" << endl;
					break;
				case (giapi::command::END_OBSERVE):
					cout << "The END_OBSERVE command has been received. END_OBSERVE the observation" << endl;
					break;
				case (giapi::command::PAUSE):
					cout << "The PAUSE command has been received. PAUSING the observation" << endl;
					break;
				case (giapi::command::CONTINUE):
					cout << "The CONTINUE command has been received.  Exposinnnnnggggg... " << endl;
					cout << "action id: " << id << " Activity: " << activity << endl;
					break;
				case (giapi::command::STOP):
					cout << "The STOP command has been received. STOPPED the observation " << endl;
					break;
				case (giapi::command::STOP_CYCLE):
					cout << "The STOP_CYCLE command has been received. The next cycle will not started" << endl;
					break;
				case (giapi::command::ABORT):
					cout << "The ABORT command has been received. ABORTING the observation" << endl;
					break;
				case (giapi::command::ENGINEERING):
					cout << "The ENGINEERING command has been received. ENGINEERING method starting " << endl;
					break;
				default:
					cout << "NOT analyzed yet" << endl;
					return HandlerResponse::create(HandlerResponse::ERROR);
			}
			DataResponse data = callback_f(id, sequenceCommand, activity, config);
			cout << "data: " << data.getResponse() << "  msg: " << data.getMsg() << endl;
			return HandlerResponse::create(data.getResponse());
		}

		//################################################################################################

		static giapi::pSequenceCommandHandler create(DataResponse (*callback)(giapi::command::ActionId,
				                                     giapi::command::SequenceCommand,
				                                     giapi::command::Activity,
													 giapi::pConfiguration)) {
			pSequenceCommandHandler handler(new InstCmdHandler(callback));
			return handler;
		}

		//################################################################################################

		InstCmdHandler(DataResponse (*callback)(giapi::command::ActionId,
													  giapi::command::SequenceCommand,
													  giapi::command::Activity,
													  giapi::pConfiguration))
		{
			callback_f = callback;
		}

		virtual ~InstCmdHandler() {}

	};

}

#endif /* INSTCMDHANDLEr*/
