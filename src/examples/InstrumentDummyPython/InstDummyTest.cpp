
#include <iostream>
#include <signal.h>

#include <decaf/util/concurrent/CountDownLatch.h>

#include <giapi/CommandUtil.h>
#include <giapi/SequenceCommandHandler.h>
#include <giapi/giapi.h>
#include "InstCmdHandler.h"
#include "DataResponse.h"


using namespace giapi;
using namespace std;
#include <decaf/util/concurrent/CountDownLatch.h>

using namespace instDummy;


DataResponse callback(giapi::command::ActionId actID, giapi::command::SequenceCommand seq, giapi::command::Activity act, giapi::pConfiguration conf)
{
    std::string str= "";
    cout<< "the callback function has been called " << endl;
    DataResponse data = DataResponse(HandlerResponse::COMPLETED, str);
	return data;
}

int main (int argc, char *argv[])
{
	decaf::util::concurrent::CountDownLatch lock(1);

    try
    {

        /*
         *  Subscribe to all Sequence Commands (except Apply), using all
         *  of the same class (cAllSeqCmds):
         *      TEST, REBOOT, INIT, DATUM, PARK, VERIFY, END_VERIFY, GUIDE,
         *      END_GUIDE, OBSERVE, END_OBSERVE, PAUSE, CONTINUE, STOP,
         *      ABORT
         *  When a command comes in it executes the "handle" method.
         */
    	pSequenceCommandHandler seqHandler = instDummy::InstCmdHandler::create(callback);

        for (int cmd = giapi::command::TEST;
        		 cmd != giapi::command::ENGINEERING; cmd++)
        {
        	CommandUtil::subscribeSequenceCommand( (giapi::command::SequenceCommand) cmd, command::SET_PRESET_START, seqHandler );
        }
        // It will wait forever
        int val =-1;
        int actionId = -1;
        while (true)
        {
        	cout << "Waiting: ";
        	cin >> val;
        	cout << "actionID: ";
        	cin >> actionId;
        	if (val == 3)
        	{
        		CommandUtil::postCompletionInfo(actionId, HandlerResponse::create(HandlerResponse::COMPLETED));
        	}
        }
        //lock.await();
    }
    catch (GiapiException &s )
    {
        cout<< "GiapiException happened" << endl;
        return -1;
    }
    catch (...)
    {
        cout << "It is not a GiapiException " << endl;
        return -1;
    }


} /* end of main() */
