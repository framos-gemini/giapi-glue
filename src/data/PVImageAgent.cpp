/* 
 * Image Display Handler using EPICS pvAccess
 *
 * Implementes an EPICS pvAccess display handler
 *
 *
 */

/* Include files */
#include <pv/clientFactory.h>
#include <pv/pvAccess.h>
#include <pv/event.h>
#include <pv/standardField.h>
#include <cfitsio/fitsio.h>
#include <giapi/PVImageHandler.h>
#include "PVImageAgent.h"

/*
 *+ 
 * FUNCTION NAME: PVImageAgent
 * 
 * INVOCATION: 
 * 
 * PARAMETERS: (">" input, "!" modified, "<" output) 
 * > display_name - name of PVImageRecord
 * 
 * FUNCTION VALUE: none
 * 
 * PURPOSE: Constructor for new PV Display
 * 
 * DESCRIPTION: 
 * Creates a PVImageRecord and starts a PV Display server
 * 
 * EXTERNAL VARIABLES: 
 * 
 * PRIOR REQUIREMENTS: 
 * 
 * DEFICIENCIES:  
 *
 *- 
 */

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;

namespace giapi {

  /*
    CLASS
    PVImageRecord
    
    DESCRIPTION  
    Concrete class for handling PV display transport (eg pvAccess client) for CICADA Displayer image updates
   
    KEYWORDS
    DISPLAYER
  */
  class PVImageRecord: public epics::pvDatabase::PVRecord  {
  public:
    POINTER_DEFINITIONS(PVImageRecord);

    //+ Class factory method for creating a new instance
    static PVImageRecordPtr create(std::string const & recordName, LoggerPtr lg) {
      StructureConstPtr header = getFieldCreate()->createFieldBuilder()->
        add("width", pvUInt) ->
        add("height", pvUInt) ->
        add("bitpix", pvInt) ->
        add("bzero", pvDouble) ->
        add("bscale", pvDouble) ->
        add("observingMode", pvInt) ->
        add("beam", pvString) ->
        add("obsSequenceNum", pvInt) ->
        add("cycle", pvInt) ->
        add("nCycles", pvInt) ->
        add("compress", pvBoolean) ->
        add("scale", pvInt) ->
        createStructure();

      StructureConstPtr subHeader = getFieldCreate()->createFieldBuilder()->
        add("xo", pvUInt) ->
        add("yo", pvUInt) ->
        add("x", pvUInt) ->
        add("y", pvUInt) ->
        add("w", pvUInt) ->
        add("h", pvUInt) ->
        add("lcut", pvDouble) ->
        add("hcut", pvDouble) ->
        add("clen", pvLong) ->
	createStructure();

      StructureConstPtr wcsHeader = getFieldCreate()->createFieldBuilder()->
        add("ctype1", pvString) ->
        add("cunit1", pvString) ->
        add("crval1", pvDouble) ->
        add("crpix1", pvDouble) ->
        add("cd1_1", pvDouble) ->
        add("cd1_2", pvDouble) ->
        add("ctype2", pvString) ->
        add("cunit2", pvString) ->
        add("crval2", pvDouble) ->
        add("crpix2", pvDouble) ->
        add("cd2_1", pvDouble) ->
        add("cd2_2", pvDouble) ->
        add("radecsys", pvString) ->
        add("equinox", pvDouble) ->
        add("mjd_obs", pvDouble) ->
	createStructure();

      StructureConstPtr closeHeader = getFieldCreate()->createFieldBuilder()->
        add("reserved", pvUInt) ->
        createStructure();

      UnionConstPtr data = getFieldCreate()->createFieldBuilder()->
	addArray("pix", pvUByte) ->
        addArray("upix", pvUShort) ->
	addArray("fpix", pvFloat) ->
	createUnion();

      StructureConstPtr subImage = getFieldCreate()->createFieldBuilder()->
	add("subHeader", subHeader) ->
	add("data", data) ->
	createStructure();

      UnionConstPtr image = getFieldCreate()->createFieldBuilder()->
	add("header", header) ->
	add("wcsHeader", wcsHeader) ->
	add("subImage", subImage) ->
	add("close", closeHeader) ->
	createUnion();

      StructureConstPtr pvImage = getFieldCreate()->createFieldBuilder()->
	  addNestedStructure("pvImage") ->
	    add("timeStamp",getStandardField()->timeStamp()) ->
            add("countIn", pvUInt) ->
            add("dtype", pvUInt) ->
	    add("image", image) ->
            add("countOut", pvUInt) ->
	  endNested()->
	createStructure();

      PVStructurePtr pvStructure = getPVDataCreate()->createPVStructure(pvImage);
      PVImageRecordPtr pvImageRecord(new PVImageRecord(recordName, pvStructure, lg));

      if (!pvImageRecord->init()) 
	pvImageRecord.reset();

      return pvImageRecord;
    };
    
    //+ Destructor
    virtual ~PVImageRecord() {
    };

    //+ Remove the record
    virtual void remove() {
      PVRecord::remove();
    };

    //+ Initialize the record
    virtual bool init() {
      initPVRecord();

      return setFields();
    };

    //+ Init fields when init has already been called
    bool initFields() {
      return setFields();
    };

    //+ Put a new value frame - updates frame dimensions
    // > imageHeader - frame header
    void putHeader(const PVImageHeader& imageHeader);

    //+ Put a new WCS header
    // > wcs - frame header
    void putWCSHeader(const WCSHeader& wcs);

    //+ Put new compression preferences
    void putCompressionPrefs(const PVImageCompressionPrefs& prefs);

    //+ Put new image data to the record - updates all fields in a group put
    // > imageData - the image data structure
    unsigned putImageData(const PVImageData& imageData);

    //+ Indicate frame is done - just setdtype to IMAGE_DONE
    void putDone();

    //+ Put value of bitpix
    // > tbitPix - bitpix
    void putBitPix(int tbitPix);

    //+ Getters for count fields
    unsigned int getCountIn() { 
      return countIn->get(); 
    };

    unsigned int getCountOut() { 
      return countOut->get(); 
    };

    uint16* getUpix(unsigned len, uint16* tpix) {
      shared_vector<const uint16> values;
      upix->getAs(values);
      for (size_t i=0; i<values.size(); i++) tpix[i] = values[i];
      return tpix;
    };

    float* getFpix(unsigned len, float* tpix) {
      shared_vector<const float> values;
      fpix->getAs(values);
      for (size_t i=0; i<values.size(); i++) tpix[i] = values[i];
      return tpix;
    };

    //+ Getter for top structure
    epics::pvData::PVStructurePtr getPVRecordStructure() {
      return getPVStructure();
    };

  private:
    // Constructor
    // > recordName - name of the record
    // > pvStructure - the pvData strucutre
    // > lg - log4cxx logger
    PVImageRecord(std::string const & recordName, epics::pvData::PVStructurePtr const & pvStructure, LoggerPtr lg): 
      PVRecord(recordName, pvStructure), logger(lg) {
    };
  
    //+ Sets local member vars
    bool setFields() {
      PVStructurePtr pvImage = getPVStructure();

      PVFieldPtr timestamp(pvImage->getSubField<PVField>("pvImage.timeStamp"));
      countIn = pvImage->getSubField<PVUInt>("pvImage.countIn");
      dtype = pvImage->getSubField<PVUInt>("pvImage.dtype");
      countOut = pvImage->getSubField<PVUInt>("pvImage.countOut");
      if (!timestamp || !pvTimeStamp.attach(timestamp) || !countIn || !countOut || !dtype) {
	throw std::invalid_argument("PVImageRecord: Bad PV Structure 1 for image");
      }

      imageU = pvImage->getSubField<PVUnion>("pvImage.image");

      // Header fields
      header = imageU->select<PVStructure>("header");
      width = header->getSubField<PVUInt>("width");
      height = header->getSubField<PVUInt>("height");
      bitpix = header->getSubField<PVInt>("bitpix");
      bzero = header->getSubField<PVDouble>("bzero");
      bscale = header->getSubField<PVDouble>("bscale");
      observingMode = header->getSubField<PVInt>("observingMode");
      beam = header->getSubField<PVString>("beam");
      obsSequenceNum = header->getSubField<PVInt>("obsSequenceNum");
      cycle = header->getSubField<PVInt>("cycle");
      nCycles = header->getSubField<PVInt>("nCycles");
      compress = header->getSubField<PVBoolean>("compress");
      scale = header->getSubField<PVInt>("scale");

      if (!width || !height || !bitpix || !bzero || !bscale || !observingMode || !beam || !obsSequenceNum || !cycle || !nCycles ||
	  !compress || !scale) {
	throw std::invalid_argument("PVImageRecord: Bad PV Structure header for image");
      }

      // WCS Header fields
      wcsHeader = imageU->select<PVStructure>("wcsHeader");
      ctype1 = wcsHeader->getSubField<PVString>("ctype1");
      cunit1 = wcsHeader->getSubField<PVString>("cunit1");
      crval1 = wcsHeader->getSubField<PVDouble>("crval1");
      crpix1 = wcsHeader->getSubField<PVDouble>("crpix1");
      cd1_1 = wcsHeader->getSubField<PVDouble>("cd1_1");
      cd1_2 = wcsHeader->getSubField<PVDouble>("cd1_2");
      ctype2 = wcsHeader->getSubField<PVString>("ctype2");
      cunit2 = wcsHeader->getSubField<PVString>("cunit2");
      crval2 = wcsHeader->getSubField<PVDouble>("crval2");
      crpix2 = wcsHeader->getSubField<PVDouble>("crpix2");
      cd2_1 = wcsHeader->getSubField<PVDouble>("cd2_1");
      cd2_2 = wcsHeader->getSubField<PVDouble>("cd2_2");
      radecsys = wcsHeader->getSubField<PVString>("radecsys");
      equinox = wcsHeader->getSubField<PVDouble>("equinox");
      mjd_obs = wcsHeader->getSubField<PVDouble>("mjd_obs");

      if (!ctype1 || !cunit1 || !crval1 || !crpix1 || !cd1_1 || !cd1_2 || !ctype2 || !cunit2 || !crval2 || !crpix2 || !cd2_1 || 
	  !cd2_2 || !radecsys || !equinox || !mjd_obs) {
	throw std::invalid_argument("PVImageRecord: Bad PV Structure wcs header for image");
      }

      // Subheader fields
      subImage = imageU->select<PVStructure>("subImage");
      subHeader = subImage->getSubField<PVStructure>("subHeader");
      xo = subHeader->getSubField<PVUInt>("xo");
      yo = subHeader->getSubField<PVUInt>("yo");
      x = subHeader->getSubField<PVUInt>("x");
      y = subHeader->getSubField<PVUInt>("y");
      w = subHeader->getSubField<PVUInt>("w");
      h = subHeader->getSubField<PVUInt>("h");
      lcut = subHeader->getSubField<PVDouble>("lcut");
      hcut = subHeader->getSubField<PVDouble>("hcut");
      clen = subHeader->getSubField<PVLong>("clen");

      if (!xo || !yo || !x || !y || !w || !h ||  !lcut || !hcut || !clen) {
	throw std::invalid_argument("PVImageRecord: Bad PV Structure subHeader for image");
      }

      // SubImage data fields
      dataU =  subImage->getSubField<PVUnion>("data");

      pix = dataU->select<PVUByteArray>("pix");
      upix = dataU->select<PVUShortArray>("upix");
      fpix = dataU->select<PVFloatArray>("fpix");

      if (!upix || !fpix || !pix) {
	throw std::invalid_argument("PVImageRecord: Bad PV Structure data for image");
      }
      return true;
    }

    LoggerPtr logger;
    PVImageCompressionPrefs compressionPrefs;
    
    PVTimeStamp pvTimeStamp;
    PVUIntPtr countIn;
    PVUIntPtr dtype;
    PVUIntPtr countOut;

    PVUnionPtr imageU;
    PVStructurePtr header;
    PVStructurePtr wcsHeader;
    PVStructurePtr subImage;
    PVStructurePtr subHeader;
    PVUnionPtr dataU;
    PVStructurePtr closeHeader;

    PVUIntPtr width;
    PVUIntPtr height;
    PVIntPtr bitpix;
    PVDoublePtr bzero;
    PVDoublePtr bscale;
    PVIntPtr observingMode;
    PVStringPtr beam;
    PVIntPtr obsSequenceNum;
    PVIntPtr cycle;
    PVIntPtr nCycles;
    PVBooleanPtr compress;
    PVIntPtr scale;

    PVStringPtr ctype1;
    PVStringPtr cunit1;
    PVDoublePtr crval1;
    PVDoublePtr crpix1;
    PVDoublePtr cd1_1;
    PVDoublePtr cd1_2;
    PVStringPtr ctype2;
    PVStringPtr cunit2;
    PVDoublePtr crval2;
    PVDoublePtr crpix2;
    PVDoublePtr cd2_1;
    PVDoublePtr cd2_2;
    PVStringPtr radecsys;
    PVDoublePtr equinox;
    PVDoublePtr mjd_obs;

    PVUIntPtr xo;
    PVUIntPtr yo;
    PVUIntPtr x;
    PVUIntPtr y;
    PVUIntPtr w;
    PVUIntPtr h;
    PVDoublePtr lcut;
    PVDoublePtr hcut;
    PVLongPtr clen;

    PVUByteArrayPtr pix;
    PVUShortArrayPtr upix;
    PVFloatArrayPtr fpix;

  };

  //+ Start a new image frame and put a new frame dimensions
  // > imageHeader - the frame header
  // > th - frame height
  // > tbitpix - bitpix
  void PVImageRecord::putHeader(const PVImageHeader& imageHeader) {
    beginGroupPut();
    TimeStamp ts;
    ts.getCurrent();
    pvTimeStamp.set(ts);
    countIn->put(1);
    dtype->put(PV_IMAGE_HEADER);

    // Header fields
    header = imageU->select<PVStructure>("header");
    header->getSubField<PVUInt>("width")->put(imageHeader.width);
    header->getSubField<PVUInt>("height")->put(imageHeader.height);
    header->getSubField<PVInt>("bitpix")->put(imageHeader.bitpix);
    bitpix->put(imageHeader.bitpix);
    header->getSubField<PVDouble>("bzero")->put(imageHeader.bzero);
    header->getSubField<PVDouble>("bscale")->put(imageHeader.bscale);
    header->getSubField<PVInt>("observingMode")->put(imageHeader.observingMode);
    std::string b(1, imageHeader.beam);
    header->getSubField<PVString>("beam")->put(b);
    header->getSubField<PVInt>("obsSequenceNum")->put(imageHeader.obsSequenceNum);
    header->getSubField<PVInt>("cycle")->put(imageHeader.cycle);
    header->getSubField<PVInt>("nCycles")->put(imageHeader.nCycles);
    header->getSubField<PVBoolean>("compress")->put(imageHeader.compress);
    compress->put(imageHeader.compress);
    imageU->set(header);

    countOut->put(getCountIn());
    process();
    endGroupPut();    
  }

  //+ Put a new WCS header
  void PVImageRecord::putWCSHeader(const WCSHeader& wcs) {
    beginGroupPut();    
    TimeStamp ts;
    ts.getCurrent();
    pvTimeStamp.set(ts);
    countIn->put(getCountIn() + 1);
    dtype->put(PV_WCS_DATA);

    wcsHeader = imageU->select<PVStructure>("wcsHeader");
    wcsHeader->getSubField<PVString>("ctype1")->put(wcs.ctype1);
    wcsHeader->getSubField<PVString>("cunit1")->put(wcs.cunit1);
    wcsHeader->getSubField<PVDouble>("crval1")->put(wcs.crval1);
    wcsHeader->getSubField<PVDouble>("crpix1")->put(wcs.crpix1);
    wcsHeader->getSubField<PVDouble>("cd1_1")->put(wcs.cd1[0]);
    wcsHeader->getSubField<PVDouble>("cd1_2")->put(wcs.cd1[1]);
    wcsHeader->getSubField<PVString>("ctype2")->put(wcs.ctype2);
    wcsHeader->getSubField<PVString>("cunit2")->put(wcs.cunit2);
    wcsHeader->getSubField<PVDouble>("crval2")->put(wcs.crval2);
    wcsHeader->getSubField<PVDouble>("crpix2")->put(wcs.crpix2);
    wcsHeader->getSubField<PVDouble>("cd2_1")->put(wcs.cd2[0]);
    wcsHeader->getSubField<PVDouble>("cd2_2")->put(wcs.cd2[1]);
    wcsHeader->getSubField<PVString>("radecsys")->put(wcs.radecsys);
    wcsHeader->getSubField<PVDouble>("equinox")->put(wcs.equinox);
    wcsHeader->getSubField<PVDouble>("mjd_obs")->put(wcs.mjd_obs);
    imageU->set(wcsHeader);

    countOut->put(getCountIn());
    process();
    endGroupPut();    
  };

  //+ Put new compression preferences
  void PVImageRecord::putCompressionPrefs(const PVImageCompressionPrefs& prefs) {
    compressionPrefs = prefs;
    beginGroupPut();    
    TimeStamp ts;
    ts.getCurrent();
    pvTimeStamp.set(ts);
    countIn->put(getCountIn() + 1);
    dtype->put(PV_COMPRESSION_DATA);
    scale->put(compressionPrefs.hcomp_scale);
    countOut->put(getCountIn());
    process();
    endGroupPut();    
  };

  //+ Put a new image data to the record - updates all fields in a group put
  // > imagedata - the image data for this transfer
  unsigned PVImageRecord::putImageData(const PVImageData& imageData) {
    unsigned n = imageData.width * imageData.height;
    beginGroupPut();
    TimeStamp ts;
    ts.getCurrent();
    pvTimeStamp.set(ts);
    countIn->put(getCountIn() + 1);
    dtype->put(PV_IMAGE_DATA);

    subImage = imageU->select<PVStructure>("subImage");
    subHeader = subImage->getSubField<PVStructure>("subHeader");
    subHeader->getSubField<PVUInt>("xo")->put(imageData.xo);
    subHeader->getSubField<PVUInt>("yo")->put(imageData.yo);
    subHeader->getSubField<PVUInt>("x")->put(imageData.x);
    subHeader->getSubField<PVUInt>("y")->put(imageData.y);
    subHeader->getSubField<PVUInt>("w")->put(imageData.width);
    subHeader->getSubField<PVUInt>("h")->put(imageData.height);
    subHeader->getSubField<PVDouble>("lcut")->put(imageData.lcut);
    subHeader->getSubField<PVDouble>("hcut")->put(imageData.hcut);

    dataU = subImage->getSubField<PVUnion>("data");
    imageU->set(subImage);
    if (compress->get()) {
      subHeader->getSubField<PVLong>("clen")->put(n);
      switch (bitpix->get()) {
      case SHORT_IMG: {
    	uint8* cpix = (uint8*) imageData.pix;
    	shared_vector<uint8> values(n);
    	for (size_t i=0; i<n; i++) values[i] = cpix[i];    
    	const shared_vector<const uint8> sv(freeze(values));
    	pix = dataU->select<PVUByteArray>("pix");
	pix->putFrom(sv);
      } break;
      case FLOAT_IMG: {
    	uint8* cpix = (uint8*) imageData.pix;
    	shared_vector<uint8> values(n);
    	for (size_t i=0; i<n; i++) values[i] = cpix[i];    
    	const shared_vector<const uint8> sv(freeze(values));
    	pix = dataU->select<PVUByteArray>("pix");
	pix->putFrom(sv);
      } break;
      default:
    	endGroupPut(); 
    	throw std::invalid_argument("PVImageRecord: Unsupported bitpix for image");
      }
    } else {
      switch (bitpix->get()) {
      case SHORT_IMG: {
	uint16* pix = (uint16*)imageData.pix;
	shared_vector<uint16> values(n);
	for (size_t i=0; i<n; i++) values[i] = pix[i];    
	const shared_vector<const uint16> sv(freeze(values));
	upix = dataU->select<PVUShortArray>("upix");
	upix->putFrom(sv);
	LOG4CXX_INFO(logger, "upix - put " << n << " values");
      } break;
      case FLOAT_IMG: {
	float* pix = (float*)imageData.pix;
	shared_vector<float> values(n);
	for (size_t i=0; i<n; i++) values[i] = pix[i];    
	const shared_vector<const float> sv(freeze(values));
	fpix = dataU->select<PVFloatArray>("fpix");
	fpix->putFrom(sv);
      } break;
      default:
	endGroupPut(); 
	throw std::invalid_argument("PVImageRecord: Unsupported bitpix for image");
      }
    }

    countOut->put(getCountIn());
    process();
    endGroupPut(); 
    
    LOG4CXX_INFO(logger, "PVImageRecord: Put " << getCountIn() << " x=" << imageData.x << ",y=" << imageData.y << ", pix0=" << 
		 ((bitpix->get() == SHORT_IMG)? (uint16)imageData.pix[0] : (float)imageData.pix[0]) << " pixn=" << 
		 ((bitpix->get() == SHORT_IMG)? (uint16)imageData.pix[n-1] : (float)imageData.pix[n-1]));
    return getCountIn();
  };


  //+ Indicate frame is done - just setdtype to IMAGE_DONE
  void PVImageRecord::putDone() {
    beginGroupPut();    
    TimeStamp ts;
    ts.getCurrent();
    pvTimeStamp.set(ts);
    countIn->put(getCountIn() + 1);
    dtype->put(PV_IMAGE_DONE);

    closeHeader = imageU->select<PVStructure>("close");
    imageU->select<PVStructure>("close")->getSubField<PVUInt>("reserved")->put(0);
    imageU->set(closeHeader);

    countOut->put(getCountIn());
    process();
    endGroupPut();    
  }

  //+ Put value of bitpix
  // > tbitpix - bitpix
  void PVImageRecord::putBitPix(int tbitpix) {
    bitpix->put(tbitpix);
    process();
  }    

  /*
   *+ 
   * Function NAME: PVImageAgent::PVImageAgent
   * 
   * INVOCATION: pvd = new PVImageAgent(const std::string n, LoggerPtr lg)
   * 
   * PARAMETERS: (">" input, "!" modified, "<" output) 
   * > n - name of display (channel)
   * > lg - logger to use
   * 
   * FUNCTION VALUE: 
   * 
   * PURPOSE: ctor
   * 
   * DESCRIPTION: This class implements a display broker between caller process and a display subsystem using pvAccess
   * 
   * EXTERNAL VARIABLES: 
   * 
   * PRIOR REQUIREMENTS: 
   * 
   * DEFICIENCIES: 
   *
   *- 
   */
  PVImageAgent::PVImageAgent(const std::string& n, const std::string& rn, LoggerPtr lg): 
    name(n), rpcChannelName(rn), logger(lg)
  {
    logger = lg;
    started = false;

    try {
      LOG4CXX_INFO(logger, "Creating PVImageAgent[" << name << "]");

      channelProvider = getChannelProviderLocal();

      // Create the transfer record
      pvImageRecord = PVImageRecord::create(name, logger);
      PVDatabase::getMaster()->addRecord(pvImageRecord);

      // Now for the RPC Service record
      pvImageRecordReq = PVImageRecord::create(rpcChannelName, logger);
      
    } catch (::epics::pvData::BaseException be) {
      LOG4CXX_ERROR(logger, "PVImageAgent[" << name << "]: exception during constructor " << be.what());    
    }
  }


  /*
   *+ 
   * Function NAME: PVImageAgent::~PVImageAgent
   * 
   * INVOCATION: delete pvd
   * 
   * PARAMETERS: (">" input, "!" modified, "<" output) 
   * 
   * FUNCTION VALUE: 
   * 
   * PURPOSE: dtor
   * 
   * DESCRIPTION: Destructor
   * 
   * EXTERNAL VARIABLES: 
   * 
   * PRIOR REQUIREMENTS: 
   * 
   * DEFICIENCIES: 
   *
   *- 
   */
  PVImageAgent::~PVImageAgent()
  {
    PVDatabase::getMaster()->removeRecord(pvImageRecord);
  }

  /*
   *+ 
   * Function NAME: PVImageAgent::startImageTransfer
   * 
   * INVOCATION: startImageTransfer(const PVImageHeader& imageHeader) 
   * 
   * PARAMETERS: (">" input, "!" modified, "<" output) 
   * > imageHeader - the meta data for the image transfer
   * 
   * FUNCTION VALUE: 
   * 
   * PURPOSE: Get ready to handle a new image
   * 
   * DESCRIPTION: Publishes a new image to PV Record. Also sets up an internal memory based FITS file for 
   * capturing the image for later recall if required
   * 
   * EXTERNAL VARIABLES: 
   * 
   * PRIOR REQUIREMENTS: 
   * 
   * DEFICIENCIES: 
   *
   *- 
   */
  void PVImageAgent::startImageTransfer(const PVImageHeader& imageHeader) {
    Lock guard(pvMutex);

    bitpix = imageHeader.bitpix;
    bzero = imageHeader.bzero;
    bscale = imageHeader.bscale;
    width = imageHeader.width;
    height = imageHeader.height;
    compress = imageHeader.compress;    

    newBuffer();

    // Now stream the new image info
    pvImageRecord->putHeader(imageHeader);

    started = true;
  }

  /*
   *+ 
   * Function NAME: PVImageAgent::transferWCS
   * 
   * INVOCATION: transferWCS(const WCSHeader& wcs) 
   * 
   * PARAMETERS: (">" input, "!" modified, "<" output) 
   * > wcs - the WCS data for the image transfer
   * 
   * FUNCTION VALUE: 
   * 
   * PURPOSE: transfer WCS data
   * 
   * DESCRIPTION: Publishes a new WCS header to PV Record.
   * 
   * EXTERNAL VARIABLES: 
   * 
   * PRIOR REQUIREMENTS: 
   * 
   * DEFICIENCIES: 
   *
   *- 
   */
  void PVImageAgent::transferWCS(const WCSHeader& wcs) {
    Lock guard(pvMutex);

    // Now stream the new WCS info
    pvImageRecord->putWCSHeader(wcs);
  }

  /*
   *+ 
   * Function NAME: PVImageAgent::transferImageData
   * 
   * INVOCATION: transferImageData(const PVImageData& imageData) 
   * 
   * PARAMETERS: (">" input, "!" modified, "<" output) 
   * > imageData - the structure containing the pixel data and meta data
   * 
   * FUNCTION VALUE: 
   * 
   * PURPOSE: Display the submimage using the pvRecord
   * 
   * DESCRIPTION: 
   * Simply uses the Epics v4 publishing tech to publish latest subimage to PV record
   * 
   * EXTERNAL VARIABLES: 
   * 
   * PRIOR REQUIREMENTS: 
   * 
   * DEFICIENCIES: 
   *
   *- 
   */
  unsigned PVImageAgent::transferImageData(const PVImageData& imageData) {
    Lock guard(pvMutex);

    // Must have called startImageTransfer for each new image
    if (!started) 
      THROW_EXCEPTION2(std::runtime_error, "PVImageAgent - must start image transfer with call to 'startImageTransfer'");
    
    // Send to pvAccess stream
    unsigned c = pvImageRecord->putImageData(imageData);

    // Update the in-memory buffer
    updateBuf(imageData, c);

    return c;
  }

  /*
   *+ 
   * Function NAME: PVImageAgent::transferImageDone
   * 
   * INVOCATION: transferImageDone
   * 
   * PARAMETERS: (">" input, "!" modified, "<" output) 
   * 
   * FUNCTION VALUE: 
   * 
   * PURPOSE: Signal image transfer is complete
   * 
   * DESCRIPTION: 
   * Indicate completion of this image transfer
   * 
   * EXTERNAL VARIABLES: 
   * 
   * PRIOR REQUIREMENTS: 
   * 
   * DEFICIENCIES: 
   *
   *- 
   */
  void PVImageAgent::transferImageDone() {
    Lock guard(pvMutex);

    if (!started) 
      THROW_EXCEPTION2(std::runtime_error, "PVImageAgent - must start image transfer with call to 'startImageTransfer'");
    pvImageRecord->putDone();  
    started = false;
  }


  // Set image compression preferences
  void PVImageAgent::setCompressionPrefs(const PVImageCompressionPrefs& prefs) {
    pvImageRecord->putCompressionPrefs(prefs);
  }

  // Gett for counter
  unsigned PVImageAgent::getCountIn() {
    return pvImageRecord->getCountIn();
  }

  // Create a new image frame in memory
  void PVImageBuffer::newBuffer() {
    // Ensure a bitpix has been set
    if (!bitpix)
      THROW_EXCEPTION2(std::runtime_error, "PVImageAgent - unsupported bitpix");
    
    buf.resize(width*height*BYTEPIX(bitpix));
    
    if (useFits) {
      // Create a new memory Fits file using CFITSIO
      switch (bitpix) {
      case SHORT_IMG:
	datatype = TSHORT;
	break;
      case FLOAT_IMG:
	datatype = TFLOAT;
	break;
      default:
	THROW_EXCEPTION2(std::runtime_error, "PVImageAgent - unsupported bitpix");
      }
      newMemFits(imageFitsMem, imageFits, datatype);
    }

    imageChunks.clear();
  } 
  
  //+ Returns address of offset into buf of pixel at x,y
  char* PVImageBuffer::get_addr(int x, int y, int xo, int yo, bool flip) {
    int16_t* ubuf = (int16_t*) &buf[0];
    float* fbuf = (float*) &buf[0];
    int yp;
    if (flip)
      yp = height-(y+yo)-1;
    else 
      yp = y+yo;
    switch (bitpix) {
    default:
    case SHORT_IMG: 
      return (char*) (ubuf+((width*yp)+x+xo));
    case FLOAT_IMG:
      return (char*) (fbuf+((width*yp)+x+xo));
    }
  }
  
  // Create a new in-memory Fits file of 2D size
  void PVImageBuffer::newMemFits(FitsMemPtr& fm, fitsfile* ff, const int dt) {
    int status;
    
    size_t fs = width*height * BYTEPIX(bitpix) + 2880;
    fs = ((fs - 1) / 2880 + 1) * 2880; 
    fm = new char[fs];
    
    // open this piece of memory as a new FITS file
    char* mem = fm;
    fits_create_memfile(&ff, (void **) &mem, &fs, IOBUFLEN, realloc, &status);
    if (!status) {
      fits_get_errstatus(status, errbuf);
      THROW_EXCEPTION2(std::runtime_error, std::string("PVImageAgent - error constructing memory FITS file: ") + std::string(errbuf));
    }
    
    // write the required header keywords 
    long dim[2];
    dim[0] = width;
    dim[1] = height;
    fits_create_img(ff, dt, 2, dim, &status);
    
    if (!status) {
      fits_get_errstatus(status, errbuf);
      THROW_EXCEPTION2(std::runtime_error, std::string("PVImageAgent - error constructing memory FITS image") + std::string(errbuf));
    }
  }

  // Update the in-memory buffer with a new chunk of data
  // > imageData - the new data
  // > c - the chunk number
  void PVImageBuffer::updateBuf(const PVImageData& imageData, unsigned c) {
    if (useFits) {
      // Update the in memory FITS file with the new data using Cfitsio - write row at a time because could be ROI
      int status = 0;
      int fpixel;
      char* p = (char*)imageData.pix;
      for (unsigned r=imageData.y+imageData.yo; (r<imageData.y+imageData.yo+imageData.height) && !status; r++) {
	fpixel = r * imageData.width + imageData.x + imageData.xo + 1; // First pixel to write
	fits_write_img(imageFits, datatype, fpixel, imageData.width, p, &status);
	p += imageData.width*BYTEPIX(bitpix);
      }
      if (!status) {
	fits_get_errstatus(status, errbuf);
	THROW_EXCEPTION2(std::runtime_error, std::string("PVImageAgent - error writing FITS image data") + std::string(errbuf));
      }
    }

    // Instead of using FITSIO
    int16_t *isrc = (int16_t*) imageData.pix;
    int16_t *idst = (int16_t*) get_addr(imageData.x, imageData.y, imageData.xo, imageData.yo);
    float *fsrc = (float*) imageData.pix;
    float *fdst = (float*) get_addr(imageData.x, imageData.y, imageData.xo, imageData.yo);
    
    // Copy data from delivered image chunk into full buffer for later retrieval. Adds bzero ??? why???
    // This is done in a rectangular band which may or may not be same width/height of full buffer
    for (unsigned i=0; i<imageData.height; i++) {
      for (unsigned j=0; j<imageData.width; j++) {
	switch (bitpix) {
	case SHORT_IMG:
	  isrc[j] += bzero;
	  idst[j] = isrc[j];
	  break;
	case FLOAT_IMG:
	  fsrc[j] += bzero;
	  fdst[j] = fsrc[j];
	  break;
	default:
	  THROW_EXCEPTION2(std::runtime_error, "PVImageAgent - Unsupported bitpix!");
	}
      }
      isrc += imageData.width;
      idst += width;
      fsrc += imageData.width;
      fdst += width;
    }
    
    // if (compress) {
    //   // Write a new compressed FITS file
    //   FitsMemPtr compressedFitsMem;
    //   fitsfile *compressedFits;
    
    //   newMemFits(compressedFitsMem, compressedFits, w, h, bitpix, datatype);
    
    //   xpix = (char*)pix;
    
    // }
    
    imageChunks[c] = new PVImageChunk(c, 0, 0, imageData.x, imageData.y, imageData.width, imageData.height);
  }
  
  /*
   *+ 
   * Function NAME: PVImageAgent::request
   * 
   * INVOCATION: rpc_client->request
   * 
   * PARAMETERS: (">" input, "!" modified, "<" output) 
   * 
   * FUNCTION VALUE: PV Structure
   * 
   * PURPOSE: Serve an image chunk addressed by chunk number 
   * 
   * DESCRIPTION: 
   * Simply uses the Epics v4 rpc server tech to provide a missing image record
   * 
   * EXTERNAL VARIABLES: 
   * 
   * PRIOR REQUIREMENTS: 
   * 
   * DEFICIENCIES: 
   *
   *- 
   */
  PVStructure::shared_pointer PVImageAgent::request(PVStructure::shared_pointer const & args)
    throw (RPCRequestException)
  {
    Lock guard(pvMutex);
    
    // get count field and check for existence
    PVScalar::shared_pointer countf = args->getSubField<PVScalar>("count");
    if (!countf) {
      LOG4CXX_ERROR(logger, "PV_Display: RPC service " << rpcChannelName << " - scalar 'count' field required");
      throw RPCRequestException(Status::STATUSTYPE_ERROR, "scalar 'count' field required");
    }
    
    // get the frame chunk count
    int count;
    try {
      count = countf->getAs<int>();
    } catch (std::exception &e)  {
      LOG4CXX_ERROR(logger, "PVImageAgent: RPC service[" << rpcChannelName << "] - failed to convert argument to int: " 
		    << e.what());
      throw RPCRequestException(Status::STATUSTYPE_ERROR, std::string("failed to convert argument to int: ") + e.what());
    }
    LOG4CXX_INFO(logger, "PVImageAgent: RPC service[" << rpcChannelName << "] request count=" << count);
    
    // create return structure and set data
    if (imageChunks.find(count) != imageChunks.end()) {
      PVImageChunk* chunk = imageChunks[count];
      
      switch (bitpix) {
      case SHORT_IMG: {
	uint16 *dfpix = (uint16*) get_addr(chunk->x, chunk->y, 0, 0);
	uint16 *pix = new uint16[chunk->height*chunk->width];
	
	// Get rectangular chunk from display_buffer
	int k = 0;
	for (int i=0; i<chunk->height; i++) {
	  for (int j=0; j<chunk->width; j++) {
	    pix[k++] = dfpix[j];
	  }
	  dfpix += width;
	}
	pvImageRecordReq->putBitPix(SHORT_IMG);
	PVImageData imageData(chunk->xo, chunk->yo, chunk->x, chunk->y, chunk->width, chunk->height, 0, 0, (char*)pix);
	pvImageRecordReq->putImageData(imageData);
	delete [] pix;
	break;
      }
      case FLOAT_IMG: {
	float *dfpix = (float*) get_addr(chunk->x, chunk->y, 0, 0);
	float *pix = new float[chunk->height*chunk->width];
	
	// Get rectangular chunk from display_buffer
	int k = 0;
	for (int i=0; i<chunk->height; i++) {
	  for (int j=0; j<chunk->width; j++) {
	    pix[k++] = dfpix[j];
	  }
	  dfpix += width;
	}
	
	pvImageRecordReq->putBitPix(FLOAT_IMG);
	PVImageData imageData(chunk->xo, chunk->yo, chunk->x, chunk->y, chunk->width, chunk->height, 0, 0, (char*)pix);
	pvImageRecordReq->putImageData(imageData);
	delete [] pix;
	break;
      }
      default:
	LOG4CXX_ERROR(logger, "PVImageAgent: RPC service[" << rpcChannelName << "] - Unsupported bitpix " << bitpix);    
	throw RPCRequestException(Status::STATUSTYPE_ERROR, std::string("Unsupported bitpix"));
      }
    } else {    
      LOG4CXX_ERROR(logger, "PVImageAgent: RPC service[" << rpcChannelName << "] - Failed to find image chunk " << count);    
      throw RPCRequestException(Status::STATUSTYPE_ERROR, std::string("Failed to find image chunk ") + std::to_string(count)+
				std::string(" #chunks=") + std::to_string(imageChunks.size()));
    }
    
    LOG4CXX_INFO(logger, "PVImageAgent: RPC service[" << rpcChannelName << "] returned chunk=" << count);
    return pvImageRecordReq->getPVRecordStructure();
  }
  
}


