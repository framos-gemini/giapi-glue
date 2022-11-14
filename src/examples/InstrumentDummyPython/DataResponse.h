#ifndef DATARESPONSE_H_
#define DATARESPONSE_H_

#include <iostream>
#include <giapi/giapi.h>
#include <giapi/SequenceCommandHandler.h>

using namespace giapi;

namespace instDummy {

	class DataResponse {
	   private:
		  HandlerResponse::Response _response;
		  // TODO. It defines an error code class
		  int _errorCode;
		  std::string _msg;

	   public:
		  DataResponse () {
			  _response = HandlerResponse::Response::ERROR;
			  _msg = "";
			  _errorCode = -1;
		  }

		  DataResponse (HandlerResponse::Response response, std::string msg) {
			  _response = response;
			  _msg = msg;
			  _errorCode = -1;
		  }

		  virtual ~DataResponse() {
			  std::cout<< "cleaning DataResponse "<< endl;
		  }

		  inline HandlerResponse::Response getResponse() {
			  return _response;
		  }

		  inline std::string getMsg() {
			  return _msg;
		  }
	};
}
#endif
