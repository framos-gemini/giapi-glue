/**
 * Example application to send status updates to Gemini
 */

#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include <decaf/util/concurrent/CountDownLatch.h>

#include <giapi/GiapiErrorHandler.h>
#include <giapi/StatusUtil.h>
#include <giapi/GiapiUtil.h>
#include <src/util/TimeUtil.h>
#include <chrono>
#include <thread>

using namespace giapi;

void terminate(int signal) {
	std::cout << "Exiting... " << std::endl;
	exit(1);
}

int main(int argc, char **argv) {
    util::TimeUtil timer;
	double time;
	double throughput;
	int nReps = 1000;
	try {

		std::cout << "Starting Status Example" << std::endl;

		signal(SIGABRT, terminate);
		signal(SIGTERM, terminate);
		signal(SIGINT, terminate);


		decaf::util::concurrent::CountDownLatch lock(1);

		StatusUtil::createStatusItem("gmp:status1", type::INT);

		StatusUtil::createStatusItem("gmp:status2", type::INT);
		StatusUtil::createStatusItem("gmp:instdummy:sad:FW1.filterpos", type::FLOAT);
                float nFloat = 5.0;
        timer.startTimer();
		for (int i = 0; i < nReps; i++) {
			std::cout<<"value: "<< i << std::endl;
			StatusUtil::setValueAsInt("gmp:status1", i);
			StatusUtil::setValueAsInt("gmp:status2", nReps-i);
			StatusUtil::setValueAsFloat("gmp:instdummy:sad:FW1.filterpos", nFloat++);
			StatusUtil::postStatus();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
        timer.stopTimer();
    	time = timer.getElapsedTime(util::TimeUtil::MSEC)/1000.0;

		throughput = double(nReps * 2) / time;

		std::cout << "Elapsed Time: " << time << " [sec]" << std::endl;
		std::cout << "Throughput  : " << throughput << " [msg/sec]" << std::endl;


	} catch (GmpException &e) {
		std::cerr << e.getMessage() <<  ". Is the GMP up?" << std::endl;
	}
	return 0;
}

