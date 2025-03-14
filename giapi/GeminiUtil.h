#ifndef GEMINIINTERACTIONUTIL_H_
#define GEMINIINTERACTIONUTIL_H_
#include <string>
#include <vector>

#include <giapi/EpicsStatusHandler.h>
#include <giapi/giapi.h>
#include <giapi/giapiexcept.h>

#include <stdexcept>
//Required for exception handling


namespace giapi {
/**
 * Structure to hold FITS file data
 */
struct FitsData {
    std::vector<unsigned char> data;  // The FITS file data
    
    FitsData() {}
    
    FitsData(const unsigned char* rawData, size_t size) {
        data.assign(rawData, rawData + size);
    }
};

/**
 * Provides the mechanisms for the instrument to interact with other
 * Gemini Principal Systems.
 */
class GeminiUtil {
public:
	/**
	 * Register a handler to receive updates when the specified EPICS status
	 * item is updated.
	 * <p/>
	 * This method is called with the <code>name</code> of the EPICS status
	 * item and a <code>handler</code> that will be called when the EPICS
	 * system publishes an update.
	 *
	 * @param name Name of the EPICS status item that will be monitored.
	 * @param handler Handler that will be called when an update is
	 *        published.The most recently registered epics status handler will
	 *        be used.
	 *
	 * @return status::OK if the subscription was successful,
	 *         status::ERROR if there is a problem with the subscription
	 *         (for instance, the epics channel name is invalid)
	 *
	 * @throws GiapiException if there is an error accessing the GMP to
	 *         subscribe this handler to monitor the given EPICS channel item
	 */
	static int subscribeEpicsStatus(const std::string &name,
			pEpicsStatusHandler handler) noexcept(false);

	/**
	 * Unregister any handlers that might be associated to the given EPICS
	 * status item.
	 *
	 * @param name Name of the EPICS status item that no longer will be
	 * monitored
	 *
	 * @return status::OK if the deregistration was successful, otherwise
	 *         returns status::ERROR
	 *
	 * @throws GiapiException if there is an error accessing the GMP to
	 *         unregister to receive updates for the given epics status item.
	 */
	static int unsubscribeEpicsStatus(const std::string &name) noexcept(false);

	/**
	 * Offload wavefront corrections to the Primary Control System (PCS).
	 * These wavefront sensor updates are in the form of slowly changing
	 * set of Zernike coefficients.
	 *
	 * @param zernikes array of zernike coefficients
	 * @param size zernike coefficients' array size.
	 *
	 * @return status::OK if the zernikes were offloaded to the PCS
	 *         or status::ERROR if there was a problem in the offload
	 *
	 * @throws GiapiException if there is an error accessing the GMP to
	 *         post the PCS update
	 */
	static int postPcsUpdate(double zernikes[], int size) noexcept(false);

	/**
	 * Provides the TCS Context information at the time of the call.
	 * The TCS Context provides information about the TCS that
	 * are needed to perform WCS conversions.
	 *
	 * @param ctx Reference to the <code>TcsContext</code> structure.
	 *        The content of this structure will be filled up by
	 *        this call.
	 * @param timeout time in milliseconds to wait for the TCS context to be
	 *        retrieved. If not specified, the call will block until the
	 *        GMP replies back.
	 *
	 * @return status::OK if the TcsContext was filled up properly.
	 *         status::ERROR if there was an error getting the TcsContext
	 *
	 * @throws GiapiException if there is an error accessing the GMP to
	 *         obtain the TCS Context, or a timeout occurs.
	 */
	static int getTcsContext(TcsContext& ctx, long timeout) noexcept(false);

       /** Function that allows an offset to be applied to the TCS. There are two types 
	 * of the offsets that instruments should indicate. For example, offsets applied
	 * during the acquisition and offsets applied during the Slow Guiding Correction. 
	 * 
	 * @param p value of the P offset. The unit must to be in arcsecs. 
         * @param q value of the Q offset. The unit must to be in arcsecs. 
         * @param offsetType Type of the offset to be applied. 
         *                      ACQ(0)         -> Adquistion. 
         *                      SLOWGUIDING(1) -> Slow Guiding Correction.
	 * @param timeout time in milliseconds to wait for the TCS to execute the offset. 
	 *
	 * @return status::OK if the offset was applied properly.
	 *         status::ERROR if there was an error applying the offset
	 *
	 * @throws GiapiException if there is an error accessing the GMP to apply the offset
         *                           to the TCS, or a timeout occurs. 
	 */
	static int tcsApplyOffset(const double p, const double q, const OffsetType offsetType, const long timeout) noexcept(false);

       /** Function that allows an offset to be applied to the TCS. There are two types 
	 * of the offsets that instruments should indicate. For example, offsets applied
	 * during the acquisition and offsets applied during the Slow Guiding Correction. 
	 * 
	 * @param p value of the P offset. The unit must to be in arcsecs. 
         * @param q value of the Q offset. The unit must to be in arcsecs. 
         * @param offsetType Type of the offset to be applied. 
         *                      ACQ(0)         -> Adquistion. 
         *                      SLOWGUIDING(1) -> Slow Guiding Correction.
         * @param callbackOffset Callback function to be called after 
         *                        the TCS offset has been applied 
	 * @param timeout time in milliseconds to wait for the TCS to execute the offset. 
	 *
	 * @return status::OK if the offset was applied properly.
	 *         status::ERROR if there was an error applying the offset
	 *
	 * @throws GiapiException if there is an error accessing the GMP to apply the offset
         *                           to the TCS, or a timeout occurs. 
	 */
	static int tcsApplyOffset(const double p, const double q,
                              const OffsetType offsetType, const long timeout,
                              void (*callbackOffset)(int, std::string)) noexcept(false);

	/**
	 * Provides a pointer to an EpicsStatus item containing the latest channel
	 * information available
	 *
	 * @param name Name of the EPICS status item that will be retrieved  
	 * @param timeout time in milliseconds to wait for the TCS context to be
	 *        retrieved. If not specified, the call will block until the
	 *        GMP replies back.
	 *
	 * @return a smart pointer to an EpicsStatusItem with the latest known values
	 *
	 * @throws GiapiException if there is an error accessing the GMP
	 *         or a timeout occurs.
	 */
	static pEpicsStatusItem getChannel(const std::string &name, long timeout) noexcept(false);

	/**
	 * Sends a FITS file through the GMP messaging system.
	 * 
	 * @param fitsFileName Path to the FITS file to be sent
	 * @param timeout time in milliseconds to wait for the file transfer to complete.
	 *        If not specified, the call will block until the GMP replies back.
	 *
	 * @return status::OK if the file was sent successfully,
	 *         status::ERROR if there was an error during the transfer
	 *
	 * @throws GiapiException if there is an error accessing the GMP to
	 *         send the file, or a timeout occurs.
	 */
	static int sendFitsFile(const std::string& fitsFileName, const long timeout) noexcept(false);

	/**
	 * Sends a FITS file through the GMP messaging system with asynchronous callback.
	 * 
	 * @param fitsFileName Path to the FITS file to be sent
	 * @param timeout time in milliseconds to wait for the file transfer to complete
	 * @param callback Function to be called after the file transfer completes
	 *        The callback receives a status code and a message string
	 *
	 * @return status::OK if the file transfer was initiated successfully,
	 *         status::ERROR if there was an error initiating the transfer
	 *
	 * @throws GiapiException if there is an error accessing the GMP to
	 *         send the file
	 */
	static int sendFitsFile(const std::string& fitsFileName, 
                           const long timeout,
                           void (*callback)(int, std::string)) noexcept(false);

        /**
         * Start receiving FITS files through the GMP messaging system.
         * When a FITS file is received, the callback function will be called.
         * 
         * @param callback Function to be called when a FITS file is received.
         *        The callback receives a FitsData structure containing the file data
         * 
         * @return status::OK if the receiver was started successfully,
         *         status::ERROR if there was a problem starting the receiver
         * 
         * @throws GiapiException if there is an error accessing the GMP to
         *         start receiving files
         */
        static int receiveFitsFiles(void (*callback)(const FitsData&)) noexcept(false);

        /**
         * Stop receiving FITS files.
         */
        static void stopReceivingFitsFiles();

        /**
         * Sends FITS data through the GMP messaging system.
         * 
         * @param fitsData The FITS data to be sent
         * @param timeout time in milliseconds to wait for the data transfer to complete.
         *        If not specified, the call will block until the GMP replies back.
         *
         * @return status::OK if the data was sent successfully,
         *         status::ERROR if there was an error during the transfer
         *
         * @throws GiapiException if there is an error accessing the GMP to
         *         send the data, or a timeout occurs.
         */
        static int sendFitsData(const FitsData& fitsData, const long timeout) noexcept(false);

        /**
         * Sends FITS data through the GMP messaging system with asynchronous callback.
         * 
         * @param fitsData The FITS data to be sent
         * @param timeout time in milliseconds to wait for the data transfer to complete
         * @param callback Function to be called after the data transfer completes
         *        The callback receives a status code and a message string
         *
         * @return status::OK if the data transfer was initiated successfully,
         *         status::ERROR if there was an error initiating the transfer
         *
         * @throws GiapiException if there is an error accessing the GMP to
         *         send the data
         */
        static int sendFitsData(const FitsData& fitsData, 
                              const long timeout,
                              void (*callback)(int, std::string)) noexcept(false);

private:
	GeminiUtil();
	virtual ~GeminiUtil();
};

// Pointer callback function declaration. 
typedef void (*callbackOffset)(int, std::string);

}

#endif /*GEMINIINTERACTIONUTIL_H_*/
