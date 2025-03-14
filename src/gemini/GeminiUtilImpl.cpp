#include "GeminiUtilImpl.h"
#include <cstdio>
#include <gemini/epics/jms/JmsEpicsManager.h>
#include <gemini/pcs/jms/JmsPcsUpdater.h>
#include <gemini/tcs/jms/JmsTcsFetcher.h>
#include <gemini/tcs/jms/JmsApplyOffset.h>
#include <gemini/epics/jms/JmsEpicsFetcher.h>
#include <gemini/fitsFileImage/jms/JmsFitsSender.h>
#include <gemini/fitsFileImage/jms/JmsFitsReceiver.h>
#include <gmp/ConnectionManager.h>
#include <gmp/GMPKeys.h>
#include <log4cxx/logger.h>

namespace giapi {

log4cxx::LoggerPtr GeminiUtilImpl::logger(log4cxx::Logger::getLogger("giapi.GeminiUtilImpl"));

pGeminiUtilImpl GeminiUtilImpl::INSTANCE(static_cast<GeminiUtilImpl *>(0));

GeminiUtilImpl::GeminiUtilImpl() noexcept(false) {
	
	_epicsMgr = JmsEpicsManager::create();
	_pcsUpdater = gemini::pcs::jms::JmsPcsUpdater::create();
	_tcsFetcher = gemini::tcs::jms::JmsTcsFetcher::create();
    _tcsApplyOffset = gemini::tcs::jms::JmsApplyOffset::create();
	_epicsFetcher = gemini::epics::JmsEpicsFetcher::create();
	_fitsSender = gemini::fitsFileImage::jms::JmsFitsSender::create();
    _fitsReceiver = gemini::fitsFileImage::jms::JmsFitsReceiver::create();
}

GeminiUtilImpl::~GeminiUtilImpl() {
	LOG4CXX_DEBUG(logger,  "Destroying Gemini Util Service");
	_epicsMgr.reset();
	_pcsUpdater.reset();
	_tcsFetcher.reset();
	_epicsFetcher.reset();
    _fitsSender.reset();
    _fitsReceiver.reset();
}

pGeminiUtilImpl GeminiUtilImpl::Instance() noexcept(false) {
	
	if (INSTANCE.get() == 0) {
		INSTANCE.reset(new GeminiUtilImpl());
	}
	return INSTANCE;
}

int GeminiUtilImpl::subscribeEpicsStatus(const std::string &name,
		pEpicsStatusHandler handler) noexcept(false) {
			
	LOG4CXX_INFO(logger, "Subscribe epics status " << name);
	return _epicsMgr->subscribeEpicsStatus(name, handler);
}

int GeminiUtilImpl::unsubscribeEpicsStatus(const std::string &name) {
	LOG4CXX_INFO(logger, "Unsubscribe epics status " << name);
	return _epicsMgr->unsubscribeEpicsStatus(name);
}

int GeminiUtilImpl::postPcsUpdate(double zernikes[], int size) {

	std::string str;
	for (int i = 0; i < size; i++) {
		char z[25];
		sprintf(z, "%.2f ", zernikes[i]);
		str.append(z);
	}

	LOG4CXX_INFO(logger, "postPCSUpdate: " << str);
	return _pcsUpdater->postPcsUpdate(zernikes, size);
}

int GeminiUtilImpl::getTcsContext(TcsContext& ctx, long timeout) const noexcept(false) {
	
	return _tcsFetcher->fetch(ctx, timeout);
}

int GeminiUtilImpl::tcsApplyOffset(const double p, const double q,
		                           const OffsetType offsetType, const long timeout)const noexcept(false) {
									
	return _tcsApplyOffset->sendOffset(p, q, offsetType,timeout);
}

int GeminiUtilImpl::tcsApplyOffset(const double p, const double q,
                                   const OffsetType offsetType, const long timeout,
                                   void (*callbackOffset)(int, std::string))const noexcept(false) {
									
	return _tcsApplyOffset->sendOffset(p, q, offsetType, timeout, callbackOffset );
}

pEpicsStatusItem GeminiUtilImpl::getChannel(const std::string &name, long timeout) noexcept(false)  {
	
	std::cout << "Destroying " << std::endl;
	return _epicsFetcher->getChannel(name, timeout);
}

int GeminiUtilImpl::sendFitsData(const FitsData& fitsData, const long timeout) const noexcept(false) {
    try {
        return _fitsSender->sendFitsData(fitsData, timeout);
    } catch (const CommunicationException& e) {
        LOG4CXX_ERROR(logger, "Problem sending FITS data: " << e.what());
        throw;
    }
}

int GeminiUtilImpl::sendFitsData(const FitsData& fitsData, 
                                const long timeout,
                                void (*callback)(int, std::string)) const noexcept(false) {
    try {
        return _fitsSender->sendFitsData(fitsData, timeout, callback);
    } catch (const CommunicationException& e) {
        LOG4CXX_ERROR(logger, "Problem sending FITS data: " << e.what());
        throw;
    }
}

int GeminiUtilImpl::receiveFitsFiles(void (*callback)(const FitsData&)) const noexcept(false) {
    try {
        return _fitsReceiver->startReceiving(callback);
    } catch (const GiapiException& e) {
        LOG4CXX_ERROR(logger, "Error starting FITS receiver: " << e.what());
        return status::ERROR;
    }
}

void GeminiUtilImpl::stopReceivingFitsFiles() const {
    _fitsReceiver->stopReceiving();
}

}
