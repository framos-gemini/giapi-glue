/*
 * ApplyOffset.h
 *
 *  Created on: Aug 17, 2023
 *      Author: framos
 */

 #ifndef FITSFILE_H_
 #define FITSFILE_H_
 
 #include <tr1/memory>
 
 #include <giapi/giapi.h>
 
 #include <stdexcept>
 //Required for exception handling
 
 namespace giapi {
 
    namespace gemini {
    
       namespace fitsfile {
       
          class FitsFile {
          
             public:
 
                /**
                * @throw GiapiException
                * If there is an error sending the FITS file,
                *        possibly due to a timeout or connection failure.
                */
                int sendFitsFile(const std::string& fitsFileName, const long timeout) const noexcept(false)=0;

                /**
                * @throw GiapiException
                * If there is an error initiating the FITS file transfer,
                *        possibly due to connection issues.
                */
                int sendFitsFile(const std::string& fitsFileName, 
                                const long timeout,
                                void (*callback)(int, std::string)) const noexcept(false)=0;

                /**
                * @throw GiapiException
                * If there is an error starting the FITS receiver,
                *        possibly due to connection issues.
                */
                int receiveFitsFiles(void (*callback)(const FitsData&)) const noexcept(false)=0;
                        
                /**
                * Stop receiving FITS files.
                */
                void stopReceivingFitsFiles() const=0;

                /**
                * @throw GiapiException
                * If there is an error sending the FITS data,
                *        possibly due to a timeout or connection failure.
                */
                int sendFitsFile(const FitsData& fitsData, const long timeout) const noexcept(false)=0;

                /**
                * @throw GiapiException
                * If there is an error initiating the FITS data transfer,
                *        possibly due to connection issues.
                */
                int sendFitsFile(const FitsData& fitsData, 
                                const long timeout,
                                void (*callback)(int, std::string)) const noexcept(false)=0;

                    
                virtual ~FitsFile() {};
             
             
          };
       
       /**
        * A smart pointer definition for the TcsFetcher class.
        */
       typedef std::tr1::shared_ptr<ApplyOffset> pTcsOffset;
       
       }
    }
 }
 
 
 #endif /* SRC_GEMINI_FITSFILE_*/
 