#ifndef FITSRECEIVER_H_
#define FITSRECEIVER_H_

#include <string>
#include <giapi/giapi.h>
#include <giapi/giapiexcept.h>

namespace giapi {

/**
 * This class provides the mechanisms for receiving FITS files sent through
 * the GMP messaging system.
 */
class FitsReceiver {
public:
    /**
     * Start receiving FITS files. When a FITS file is received, the callback function
     * will be called with the file data and metadata.
     * 
     * @param callback Function to be called when a FITS file is received.
     *        The callback receives:
     *        - const unsigned char* data: The FITS file data
     *        - size_t size: Size of the FITS file data
     *        - const std::string& filename: Original filename of the FITS file
     * 
     * @return status::OK if the receiver was started successfully,
     *         status::ERROR if there was a problem starting the receiver
     * 
     * @throws GiapiException if there is an error accessing the GMP to
     *         start receiving files
     */
    static int startReceiving(void (*callback)(const unsigned char*, size_t, const std::string&)) noexcept(false);

    /**
     * Stop receiving FITS files.
     */
    static void stopReceiving();

private:
    FitsReceiver();
    virtual ~FitsReceiver();
};

}

#endif /* FITSRECEIVER_H_ */ 