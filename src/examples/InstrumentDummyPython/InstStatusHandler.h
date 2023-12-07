#ifndef INSTSTATUSHANDLER_H_
#define INSTSTATUSHANDLER_H_


/**                                                         
 * Example application to send status updates to Gemini     
 */                                                         
                                                            
#include <iostream>                                         
#include <stdlib.h>                                         
#include <signal.h>                                         
#include <sys/time.h>                                       
                                                            
#include <giapi/GiapiErrorHandler.h>                        
#include <giapi/EpicsStatusHandler.h>                       
#include <giapi/GeminiUtil.h>                               
#include <giapi/GiapiUtil.h>      

using namespace giapi;

using namespace std;  

namespace instDummy {
    
   typedef void (*callback_def2)(int, giapi::pEpicsStatusItem);

   class InstStatusHandler: public giapi::EpicsStatusHandler {
      private:
         callback_def2 callback_f;
      public:
         virtual void channelChanged(giapi::pEpicsStatusItem item) 
         {
            //cout << " The  " << item->getName() << " changed, calling to callback_f" << endl;
            callback_f(item->getType(), item);
         }
         static giapi::pEpicsStatusHandler create(void (*callback)(int, giapi::pEpicsStatusItem)) {
            pEpicsStatusHandler handler(new InstStatusHandler(callback));
	    return handler;
         }

	 InstStatusHandler(void (*callback)(int, pEpicsStatusItem)) {
            callback_f = callback;
	 }

	 virtual ~InstStatusHandler() {}
   };

}

#endif /* InstStatusHandler */
