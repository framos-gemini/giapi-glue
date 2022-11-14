/*
 * ScorpioDetectorController.cpp
 *
 *  Created on: Jul 15, 2021
 *      Author: vagrant
 */

#include "ScorpioDetectorController.h"
#include <iostream>

ScorpioDetectorController::ScorpioDetectorController():SequenceCommandHandler() {
	// TODO Auto-generated constructor stub

}

ScorpioDetectorController::~ScorpioDetectorController() {
	// TODO Auto-generated destructor stub
}

pSequenceCommandHandler ScorpioDetectorController::create()

{
		pSequenceCommandHandler instance( new ScorpioDetectorController() );
		return instance;
}

pHandlerResponse ScorpioDetectorController::handle( ActionId id, SequenceCommand sequenceCommand,
			                 	 	 	 	 	 	 Activity activity, pConfiguration config )
{
   if (config != NULL) {
			vector<string> keys = config->getKeys();
			vector<string>::iterator it = keys.begin();
			cout << "Scorpio Detector Controller wrong config " << endl;
			for (; it < keys.end(); it++) {
				cout << "{" << *it << " : " << config->getValue(*it) << "}" << std::endl;
			}
		}
   cout << "Detector controler handle called " << endl;
   return HandlerResponse::create(HandlerResponse::STARTED);

}
