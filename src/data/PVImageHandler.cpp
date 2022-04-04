/**
 * As an enhancement to GIAPI a new protocol has been devised to
 * provide instrument builders with the capability of communicating
 * image data for real-time display. This capability makes use of the
 * recent developments in EPICS with the pvAccess protocol. This new
 * GIAPI protocol hides the use of pvAccess and uses the EPICS
 * libraries in a transparent way. It is devised using the
 * publish-subscribe data model where image streams are identified
 * using an instrument specific name. Interested clients can subscribe
 * to a specific image stream and receive image updates that can then
 * be utilised - typically using a real-time display application
 * (again specific to the instrument).

 * In addition to the publish/subscribe model, the most recent image
 * transfer is held in memory by the library and can be fetched using
 * a pvAccess RPC service. Each image transfer is identified by a
 * transfer number which is used in the RPC call to fetch the desired
 * part of the image as an index.
 */

#include <algorithm>
#include <epicsThread.h>
#include <giapi/PVImageHandler.h>
#include <giapi/StatusUtil.h>
#include <pv/ntscalar.h>
#include <pv/rpcServer.h>
#include "PVImageAgent.h"

using namespace epics::pvData;
using namespace epics::nt;
using namespace epics::pvDatabase;
using namespace epics::pvAccess;

namespace giapi {
  
  log4cxx::LoggerPtr PVImageHandler::logger(log4cxx::Logger::getLogger("giapi.PVImageHandler"));
  //  giapi::PVServerPtr giapi::PVServer::INSTANCE(static_cast<PVServer *>(0));
  
  // use Impl derived class to hide internal operation from public interface
  class PVImageHandlerImpl: public PVImageHandler {
  public:
    PVImageHandlerImpl(const std::string& n):PVImageHandler() {
      name = n;

      LOG4CXX_INFO(logger, "Starting the RPC Server for " << name);

      // Create the RPC Service
      rpcServer.reset(new RPCServer());

      // Need to publish rpc server port - use GIAPI status record named "rpcChannelName+.port"
      rpcChannelName = name + "Req";
      rpcPortName = rpcChannelName + ".port";

      if ((StatusUtil::createStatusItem(rpcPortName, type::INT)) != status::OK) {
        LOG4CXX_ERROR(logger, "PVImageHandlerImpl - failed to create RPC port status item, name=" << rpcPortName);
      }

      if (StatusUtil::setValueAsInt(rpcPortName, rpcServer->getServer()->getServerPort()) != status::OK) {
        LOG4CXX_ERROR(logger, "PVImageHandlerImpl - failed to set RPC port value for " << rpcPortName << " port=" << 
		      rpcServer->getServer()->getServerPort());
      }
      if (StatusUtil::postStatus(rpcPortName) != status::OK) {
	LOG4CXX_ERROR(logger, "PVImageHandlerImpl - Exception during postStatus for " << rpcPortName);   
      }

      LOG4CXX_INFO(logger, "RPC Service for [" << name << "] port=" << rpcServer->getServer()->getServerPort());
      
      LOG4CXX_INFO(logger, "Creating the PVImage Agent");

      // Create the agent for doing the transfers
      pvImageAgent = PVImageAgent::create(n, rpcChannelName, logger);

      // Register the imageReq service
      rpcServer->registerService(rpcChannelName, pvImageAgent);
      LOG4CXX_INFO(logger, "PV_Display_HandlerImpl[" << name << "]: registered RPC service " << rpcChannelName);

      LOG4CXX_INFO(logger, "Starting the PV Server");

      // Now start the PVA server singleton and publish the port number
      serverPort = pvImageAgent->startPVAserver();

      // Need to publish server port - use GIAPI status record named "channelName+.port"
      std::string portName = name + ".server_port";

      if ((StatusUtil::createStatusItem(portName, type::INT)) != status::OK) {
        LOG4CXX_ERROR(logger, "PVImageHandlerImpl - failed to create server port status item, name=" << portName);
      }

      if (StatusUtil::setValueAsInt(portName, serverPort) != status::OK) {
        LOG4CXX_ERROR(logger, "PVImageHandlerImpl - failed to set PV server port value for " << portName << " port=" << 
		      serverPort);
      }
      if (StatusUtil::postStatus(portName) != status::OK) {
	LOG4CXX_ERROR(logger, "PVImageHandlerImpl - Exception during postStatus for " << portName);   
      }

      LOG4CXX_INFO(logger, "PV Server started on port " << serverPort << " giapi name " << portName);
    };

    virtual ~PVImageHandlerImpl() {
      rpcServer->unregisterService(rpcChannelName);
      LOG4CXX_INFO(logger, "PVImageHandlerImpl dtor - unregisterService");
      pvImageAgent->stopPVAserver();
    };

    

    std::string name;
    std::unique_ptr<RPCServer> rpcServer;
    PVImageAgentPtr pvImageAgent;    
    std::string rpcChannelName;
    std::string rpcPortName;
    int serverPort;
  };

  // downcasting methods
  inline PVImageHandlerImpl * GetImpl(PVImageHandler* ptr) { return (PVImageHandlerImpl *)ptr; }
  inline const PVImageHandlerImpl * GetImpl(const PVImageHandler* ptr) { return (const PVImageHandlerImpl *)ptr; }

  // Create an instance for public use. Ptr will get destroyed automatically when no longer referenced
  pPVImageHandler PVImageHandler::create(const std::string& name) {
    pPVImageHandler handler(new PVImageHandlerImpl(name));
    return handler;
  }

  PVImageHandler::PVImageHandler() {
  }
  
  PVImageHandler::~PVImageHandler() {
  }
    
  void PVImageHandler::startImageTransfer(const PVImageHeader& imageHeader) throw (ImageTransferException) {
    PVImageHandlerImpl *p = GetImpl(this);
    p->pvImageAgent->startImageTransfer(imageHeader);
  }

  void PVImageHandler::transferWCS(const WCSHeader& wcsHeader) throw (ImageTransferException) {
    PVImageHandlerImpl *p = GetImpl(this);
    p->pvImageAgent->transferWCS(wcsHeader);
  }

  unsigned PVImageHandler::transferImageData(const PVImageData& imageData) throw (ImageTransferException){
    PVImageHandlerImpl *p = GetImpl(this);
    return p->pvImageAgent->transferImageData(imageData);
  }
  
  void PVImageHandler::transferImageDone() throw (ImageTransferException) {
    PVImageHandlerImpl *p = GetImpl(this);
    p->pvImageAgent->transferImageDone();
  }

  void PVImageHandler::setImageCompressionPrefs(const PVImageCompressionPrefs& imageCompressionPrefs) {
    PVImageHandlerImpl *p = GetImpl(this);
    p->pvImageAgent->setCompressionPrefs(imageCompressionPrefs);
  };


  unsigned PVImageHandler::getCountIn() {
    PVImageHandlerImpl *p = GetImpl(this);
    return p->pvImageAgent->getCountIn();
  }

  int PVImageHandler::getServerPort() {
    PVImageHandlerImpl *p = GetImpl(this);
    return p->serverPort;
  }


}
