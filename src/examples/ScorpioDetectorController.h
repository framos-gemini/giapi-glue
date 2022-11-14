/*
 * ScorpioDetectorController.h
 *
 *  Created on: Jul 15, 2021
 *      Author: vagrant
 */

#ifndef SCORPIODETECTORCONTROLLER_H_
#define SCORPIODETECTORCONTROLLER_H_

#include <giapi/giapi.h>
#include <giapi/SequenceCommandHandler.h>

using namespace giapi;
using namespace command;

class ScorpioDetectorController: public SequenceCommandHandler {
public:
	ScorpioDetectorController();
	virtual ~ScorpioDetectorController();

	static pSequenceCommandHandler create(); /* {
		pSequenceCommandHandler handler (new ScorpioDetectorController());
		return handler;
	}
	*/

	pHandlerResponse handle( ActionId id, SequenceCommand sequenceCommand,
			                 Activity activity, pConfiguration config ); /* {
		if (config != NULL) {
					vector<string> keys = config->getKeys();
					vector<string>::iterator it = keys.begin();
					cout << "Scorpio Detector Controller wrong config " << endl;
					for (; it < keys.end(); it++) {
						cout << "{" << *it << " : " << config->getValue(*it) << "}" << std::endl;
					}
				}
		cout << "Detector controler handle called " << endl;
	}
	*/
};

#endif /* SRC_EXAMPLES_SCORPIODETECTORCONTROLLER_H_ */
