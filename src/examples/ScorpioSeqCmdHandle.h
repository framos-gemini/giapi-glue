#ifndef SCORPIOSEQCMDHANDLE_H_
#define SCORPIOSEQCMDHANDLE_H_

#include <iostream>
#include <giapi/giapi.h>
#include <giapi/SequenceCommandHandler.h>
#include<unistd.h>
unsigned int microsecond = 1000000;


using namespace giapi;
using namespace std;


class ScorpioSeqCmdHandle: public giapi::SequenceCommandHandler {

private:
	/*Private attributes and methods */

public:

	virtual giapi::pHandlerResponse handle(giapi::command::ActionId id,
			                               giapi::command::SequenceCommand sequenceCommand,
			                               giapi::command::Activity activity,
										   giapi::pConfiguration config) {

		decaf::util::concurrent::CountDownLatch lock(1);
		if (config != NULL) {
			vector<string> keys = config->getKeys();
			vector<string>::iterator it = keys.begin();
			cout << "Scorpio Configuration " << endl;
			for (; it < keys.end(); it++) {
				cout << "{" << *it << " : " << config->getValue(*it) << "}" << std::endl;
			}
		}
		// It is a continue command. Is there to verify if the CONTINUE cmd its Action Id will be the Observe id.
		// Hay dos aproximaciones, cuando se comanda el Observe despues del primer nodding A, el instrument controller
		// envia un WAITING con el ID del Observ, por lo tanto, el cotinue deberia de enviar el mismo ID.
		// Otra solucion es crear un nuevo comando llamado OBSSERVE_NOD y y cuando se termine el punto (A o B)
		// se envie un END_NOD.
		switch (sequenceCommand)
		{
		    case (giapi::command::REBOOT):
				cout << "The REBOOT command has been received. REBOOT the observation" << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::INIT):
				cout << "The INIT command has been received. INIT the observation" << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::DATUM):
				cout << "The DATUM command has been received. DATUM the observation" << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::PARK):
				cout << "PARK " << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
		    case (giapi::command::VERIFY):
			    cout << "VERIFY" << endl;
			    return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::END_VERIFY):
				cout << "END_VERIFY" << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
		    case (giapi::command::GUIDE):
				cout << "The GUIDE command has been received. GUIDING the observation" << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::END_GUIDE):
				cout << "The END_GUIDE command has been received. finishing the observation" << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::APPLY):
				cout << "The APPLY command has been received. This class shouldn't receive this command" << endl;
				return HandlerResponse::create(HandlerResponse::ERROR);
			case (giapi::command::OBSERVE):
				cout << "The OBSERVE command has been received. STARTING the observation, id: " <<  id << endl;

			    //METEMOS COLA
				//EL OTRO PROCESO SACA DE LA COLA Y ENVIA A LA OBSERVACION EMPEZAR
			    // ENVIAR WAITING CON ID
			    // cONTINUE ID
			    // END_OBSERVE
				return HandlerResponse::create(HandlerResponse::STARTED);
		    case (giapi::command::END_OBSERVE):
			    cout << "The END_OBSERVE command has been received. END_OBSERVE the observation" << endl;
			    return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::PAUSE):
				cout << "The PAUSE command has been received. PAUSING the observation" << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
			case (giapi::command::CONTINUE):
				cout << "The CONTINUE command has been received.  Exposinnnnnggggg... " << endl;
			    cout << "action id: " << id << " Activity: " << activity << endl;
				return HandlerResponse::create(HandlerResponse::COMPLETED);
			case (giapi::command::STOP):
				cout << "The STOP command has been received. STOPPED the observation " << endl;
				return HandlerResponse::create(HandlerResponse::STARTED);
		    case (giapi::command::STOP_CYCLE):
			    cout << "The STOP_CYCLE command has been received. The next cycle will not started" << endl;
			    return HandlerResponse::create(HandlerResponse::COMPLETED);



			case (giapi::command::ABORT):
				cout << "The ABORT command has been received. ABORTING the observation" << endl;
				return HandlerResponse::create(HandlerResponse::COMPLETED);
			case (giapi::command::ENGINEERING):
				cout << "The ENGINEERING command has been received. ENGINEERING method starting " << endl;
				return HandlerResponse::create(HandlerResponse::COMPLETED);
			default:
				cout << "NOT analyzed yet" << endl;
				return HandlerResponse::create(HandlerResponse::ERROR);
		}
	}

	static giapi::pSequenceCommandHandler create() {
		pSequenceCommandHandler handler(new ScorpioSeqCmdHandle());
		return handler;
	}

	virtual ~ScorpioSeqCmdHandle() {}

};

#endif /*MYHANDLER_H_*/
