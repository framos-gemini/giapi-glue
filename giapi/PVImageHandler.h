#ifndef PVIMAGEHANDLER_H_
#define PVIMAGEHANDLER_H_

#include <string>
#include <memory>

#include <giapi/giapi.h>
#include <giapi/giapiexcept.h>
#include <log4cxx/logger.h>
#ifdef READONLY
#undef READONLY
#endif
#include <fitsio.h>

namespace giapi {

  /**
   * An image transfer exception. These exceptions are raised when
   * some communication between the GIAPI library and the Gemini
   * image display client fails.
   */
  class ImageTransferException: public GiapiException {
  public:
    
    /**
     * Default constructor
     */
    ImageTransferException() throw();
    
    /**
     * Constructor that will take as argument the message
     * used to describe the reason why the exception was thrown
     */
    ImageTransferException(const std::string& message) throw();
  };
  
  /**
   * Structure for holding WCS info
   */
  struct WCSHeader {
    char ctype1[8];
    char cunit1[8];
    double crval1;
    double crpix1;
    double cd1[2];
    char ctype2[8];
    char cunit2[8];
    double crval2;
    double crpix2;
    double cd2[2];
    char radecsys[8];
    double equinox;
    double mjd_obs;
  };

  /**
   * Structure holding image meta data
   */
  struct PVImageHeader {
    PVImageHeader(const std::string& n, const unsigned w, const unsigned h, const int bp, const double bz=0.0, const double bs=1.0, const int om = 0,
		  const char b = 'A', const int os = 1, const int c = 1, const int nc = 1):
      imageName(n), width(w), height(h), bitpix(bp), bzero(bz), bscale(bs), observingMode(om), beam(b), obsSequenceNum(os), cycle(c), nCycles(nc)  {
	compress = false;
    };

    std::string imageName;           // name of this image
    unsigned width;                  // frame width 
    unsigned height;                 // frame height
    int bitpix;                      // the FITS bitpix
    double bzero;                    // the FITS bzero
    double bscale;                   // the FITS bscale
    
    int observingMode;               // Code describing observing mode - instrument specific. eg "Stare" for stare mode observation
    char beam;                       // the current telescope beam - in ABBA style observation
    int obsSequenceNum;              // Index of observation in observation cycle ABBA, 0..N-1, eg first A is sequence 1, 2nd A is 4, 2nd B is sequence 3, etc
    int cycle;                       // the observation cycle in a mult-cycle observation (eg ABBA followed by ABBA etc)
    int nCycles;                     // the number of cycles in the full observation

    bool compress;                   // compress data prior to transfer - must have set compression prefs using 'setCompressionPrefs'
    WCSHeader wcs;                   // Image WCS data
  };

  /**
   * Structure holding image pixel data
   */
  struct PVImageData {
    PVImageData(unsigned txo, unsigned tyo, unsigned tx, unsigned ty, unsigned w, unsigned h, double lc, double hc, char* p):
      xo(txo), yo(tyo), x(tx), y(ty), width(w), height(h), lcut(lc), hcut(hc), pix(p) {
    };
    
    unsigned xo;                     // x offset of subframe
    unsigned yo;                     // y offset of subframe
    unsigned x;                      // x position of subimage
    unsigned y;                      // y position of subimage
    unsigned width;                  // width of subimage
    unsigned height;                 // height of subimage
    double lcut;                     // lower range of data for display cut
    double hcut;                     // upper range of data for display cut
    char* pix;                       // pointer to pixel data
  };

  /**
   * Structure holding CFITSIO image compression prefs
   */
  enum CompressionCodec {
    CODEC_NONE = -1,
    CODEC_RICE = 11,
    CODEC_GZIP1 = 21,
    CODEC_GZIP2 = 22,
    CODEC_PLIO = 31,
    CODEC_HCOMPRESS = 41,
    CODEC_BZIP2 = 51
  };
  
  struct PVImageCompressionPrefs {
    CompressionCodec codec;          // requested image compression algorithm 
    long tilesize[MAX_COMPRESS_DIM]; // requested tiling size (array)
    float quantize_level;            // requested quantize level
    int quantize_method;             // requested  quantizing method
    int dither_seed;                 // starting offset into the array of random dithering
    int lossy_int_compress;          // lossy compress integer image as if float image?
    int huge_hdu;                    // use '1Q' rather then '1P' variable length arrays
    float hcomp_scale;               // requested HCOMPRESS scale factor 
    int hcomp_smooth;                // requested HCOMPRESS smooth parameter
  };

  
  class PVImageHandler;

  /**
   * Definition of a smart pointer to a PVImage  handler.
   */
  typedef std::unique_ptr<PVImageHandler> pPVImageHandler;

  /**
   * This class provides the mechanisms to update Gemini real-time display clients with image information
   * required to integrate the data acquisition process inside the instrument with the OCS.
   */
  class PVImageHandler {
  public:
    static log4cxx::LoggerPtr logger;

    /**
     * Create a Ptr to new instance of self
     * 
     * @param name - unique name of the image handler stream
     *
     * @returns An STL unique_ptr to a new instance of a PVImageHandler
     */
    static pPVImageHandler create(const std::string& name);

    virtual ~PVImageHandler();

    /**
     * Send a new image transfer header using EPICS pvAccess
     *
     * @param imageHeader - meta data for this image transfer
     *
     * @throws ImageTransferException if there is an error in EPICS pvAccess
     */
    void startImageTransfer(const PVImageHeader& imageHeader) throw (ImageTransferException);
    
    /**
     * Send a new image WCS header using EPICS pvAccess
     *
     * @param wcsHeader - WCS data for this image transfer
     *
     * @throws ImageTransferException if there is an error in EPICS pvAccess
     */
    void transferWCS(const WCSHeader& wcsHeader) throw (ImageTransferException);

    /**
     * Send a new image data segment using the Epics pvAccess protocol
     *
     * @param imageData - structure containing image data
     *
     * @returns count of image segment transferred, 0=start, 1=first chunk, 2=2nd chunk ... n=last chunk
     * @throws ImageTransferException if there is an error in EPICS pvAccess
     */
    unsigned transferImageData(const PVImageData& imageData) throw (ImageTransferException);
    
    /**
     * Send indicator that image transfer has completed using EPICS pvAccess
     *
     * @throws ImageTransferException if there is an error in EPICS pvAccess
     */
    void transferImageDone() throw (ImageTransferException);
    
    /**
     * Set compression preferences (CFITSIO compatible)
     * 
     * @param imageCompressionPrefs - selected image compression prefs
     */
    void setImageCompressionPrefs(const PVImageCompressionPrefs& imageCompressionPrefs);

    /**
     * Returns the current image chunk counter
     * @returns counter
     */
    unsigned getCountIn();

    /*
     * Return the PVA server port number
     *
     * @returns PVA server port number
     */
    int getServerPort();
    
  protected:
    PVImageHandler();
  };

}

#endif /*PVIMAGEHANDLER_H_*/
