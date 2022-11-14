/*
 * ScorpioDetector.h
 *
 *  Created on: Jul 15, 2021
 *      Author: vagrant
 */

#ifndef SCORPIODETECTOR_H_
#define SCORPIODETECTOR_H_

#include <giapi/giapi.h>
#include <giapi/SequenceCommandHandler.h>

using namespace giapi;
using namespace command;

class ScorpioDetector {
public:
	ScorpioDetector();
	virtual ~ScorpioDetector();
};

#endif /* SRC_EXAMPLES_SCORPIODETECTOR_H_ */
