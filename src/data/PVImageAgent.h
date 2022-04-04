/*
 * FILENAME PVImageAgent.h
 * 
 * PURPOSE
 *
 * Interface for an EPICS pvAccess display handler
 * 
 */
#ifndef _PVDISPLAYAGENT_H
#define _PVDISPLAYAGENT_H

/*---------------------------------------------------------------------------
  Include files 
*/
#include <cfitsio/fitsio.h>
#include <pv/pvIntrospect.h>
#include <pv/pvData.h>
#include <pv/pvAccess.h>
#include <pv/pvDatabase.h>
#include <pv/serverContext.h>
#include <pv/channelProviderLocal.h>
#include <pv/clientFactory.h>

// include log4cxx header files.
#include "log4cxx/logger.h"

using namespace log4cxx;

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;

inline int BYTEPIX(int bp) { return (abs(bp)/8); };

namespace giapi {
  // Typedefs
  enum PV_Record_Type { 
    PV_IMAGE_HEADER = 0, 
    PV_IMAGE_DATA = 1, 
    PV_IMAGE_DONE = 2,
    PV_COMPRESSION_DATA = 3,
    PV_WCS_DATA = 4
  };

  struct PVImageHeader;
  struct PVImageData;
  struct PVImageCompressionPrefs;

  /*--------------------------------------------------------------------------
    Class declarations 
    -------------------------------------------------------------------------*/

  // Containers

  struct PVImageChunk {
    PVImageChunk(unsigned s, unsigned xot, unsigned yot, unsigned xt, unsigned yt, unsigned w, unsigned h): 
      seq(s), xo(xot), yo(yot), x(xt), y(yt), width(w), height(h) { };
    int seq; 
    int xo;
    int yo;
    int x;
    int y;
    int width;
    int height;
  };
  
  /*
    CLASS
    PVImageBuffer
    
    DESCRIPTION  
    Holds data for display frame in memory. Used to build display frame in memory buffer
    prior to uploading to actual display sub-system. Uses CFITSIO.
    
    KEYWORDS
    Image Buffer
  */
  class PVImageBuffer {
    typedef char* FitsMemPtr;

  private:
    std::vector<char> buf;
    FitsMemPtr imageFitsMem;
    fitsfile *imageFits;
    int datatype;

    void newMemFits(FitsMemPtr& fm, fitsfile* ff, const int dt);

  protected:
    int bitpix;
    double bzero;
    double bscale;
    bool unsigned_data;
    unsigned width;
    unsigned height;
    bool compress;
    bool useFits;
    std::map<int, giapi::PVImageChunk*, std::less<int> > imageChunks; //+ Map of each image chunk delivered
    char errbuf[255]; // CFITSIO error message buffer

  public:

    //+ Constructor
    PVImageBuffer() {
      bitpix = 0;
      bzero = 0;
      bscale = 1.0;
      width = 0;
      height = 0;
      compress = false;
      useFits = false;
      imageFitsMem = NULL;
    };

    virtual ~PVImageBuffer() {
      if (imageFitsMem != NULL)
	delete [] imageFitsMem;
    };
  
    // Create a new image frame in memory
    void newBuffer();

    // Update the in-memory buffer with a new chunk of data
    // > imageData - the new data
    void updateBuf(const PVImageData& imageData, unsigned c);

    //+ returns address of buf
    char* get_buf() { 
      return &buf[0]; 
    };
  
    //+ Returns address of offset into buf of pixel at x,y
    char* get_addr(int x, int y, int xo, int yo, bool flip=false);
  };

  class PVImageRecord;
  typedef std::tr1::shared_ptr<PVImageRecord> PVImageRecordPtr;

  /**
   * Singleton class for starting the PVAServer
   */
  class PVServer {
  private:
    static ServerContext::shared_pointer pvCtx;
    PVServer() {
      PVServer::pvCtx = startPVAServer(PVACCESS_ALL_PROVIDERS, 0, true, true);
    };
  public: 
    static PVServer& getInstance() {
      static PVServer pvServer;
      return pvServer;
    };
    int getServerPort() {
      return PVServer::pvCtx->getServerPort(); 
    };
  };


  /*
    CLASS
    PVImageAgent
    
    DESCRIPTION  
    Implementation class for handling PV display transport
   
    KEYWORDS
    DISPLAYER
  */
  class PVImageAgent;
  typedef std::tr1::shared_ptr<PVImageAgent> PVImageAgentPtr;

  class PVImageAgent: public RPCService, PVImageBuffer {
  public:
    POINTER_DEFINITIONS(PVImageAgent);

    //+ Constructor
    // > n - name
    // > rn - rpc channel name
    // > rs - rpc server
    // > lg - logger
    static PVImageAgent::shared_pointer create(const std::string& n, const std::string& rn,  LoggerPtr lg) {
      return PVImageAgentPtr(new PVImageAgent( n, rn, lg));
    };

    virtual ~PVImageAgent();

    void startImageTransfer(const PVImageHeader& imageHeader);

    void transferWCS(const WCSHeader& wcsHeader);

    unsigned transferImageData(const PVImageData& imageData);

    void transferImageDone();

    void setCompressionPrefs(const PVImageCompressionPrefs& prefs);

    unsigned getCountIn();


    //+ Handle PV RPC request
    PVStructure::shared_pointer request(PVStructurePtr const & pvArguments)
      throw (RPCRequestException);
    
    // Method available to CC modules to start the server
    int startPVAserver() {
      // Start the EPICS V4 server - singleton
      try {
	channelProvider = epics::pvDatabase::getChannelProviderLocal();
	int port = handle_server(true);
	LOG4CXX_DEBUG(logger, "EPICSV4 server started on port " << port);
	return port;
      } catch (::epics::pvData::BaseException be) {
	THROW_EXCEPTION2(std::runtime_error, "PVImageAgent - Failed EPICSV4 startup");
      }
    };

    // Stop PVAserver
    void stopPVAserver() {
      handle_server(false);
    };

  private:
    PVImageAgent(const std::string& n, const std::string& rn,  LoggerPtr lg);

    //+ Only start the PVA server once for this process
    static int handle_server(bool start) {
      static bool started = false;
      static ServerContext::shared_pointer pvCtx;
      
      if (start) {
	if (!started) {
	  pvCtx = startPVAServer(PVACCESS_ALL_PROVIDERS, 0, true, true);
	  started = true;
	}
	return pvCtx->getServerPort();
      } else { // stop
	if (started) {
	  pvCtx.reset();     
	  started = false;
	}
	return 0;
      }
    };

    std::string name;
    std::string rpcChannelName;
    LoggerPtr logger;

    Mutex pvMutex;
    PVImageRecordPtr pvImageRecord;
    PVImageRecordPtr pvImageRecordReq;
    ChannelProviderLocalPtr channelProvider;

    bool started;
  };

}
#endif //PVDISPLAYAGENT_H
