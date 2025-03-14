#include <giapi/GeminiUtil.h>
#include "GeminiUtilImpl.h"

namespace giapi
{

GeminiUtil::GeminiUtil()
{
}

GeminiUtil::~GeminiUtil()
{
}


int GeminiUtil::subscribeEpicsStatus(const std::string &name,
		pEpicsStatusHandler handler) noexcept(false){
			
	return GeminiUtilImpl::Instance()->subscribeEpicsStatus(name, handler);
}

int GeminiUtil::unsubscribeEpicsStatus(const std::string &name) noexcept(false){
	return GeminiUtilImpl::Instance()->unsubscribeEpicsStatus(name);
}

int GeminiUtil::postPcsUpdate(double zernikes[],
		int size) noexcept(false) {
	return GeminiUtilImpl::Instance()->postPcsUpdate(zernikes, size);
}

int GeminiUtil::getTcsContext(TcsContext& ctx, long timeout) noexcept(false) {
	return GeminiUtilImpl::Instance()->getTcsContext(ctx, timeout);
}

int GeminiUtil::tcsApplyOffset(const double p, const double q, const OffsetType offsetType, const long timeout) noexcept(false) {
	return GeminiUtilImpl::Instance()->tcsApplyOffset(p, q, offsetType, timeout);
}

int GeminiUtil::tcsApplyOffset(const double p, const double q,
                              const OffsetType offsetType, const long timeout,
                              void (*callbackOffset)(int, std::string)) noexcept(false) {
								

	return GeminiUtilImpl::Instance()->tcsApplyOffset(p, q, offsetType, timeout, callbackOffset);

}

int GeminiUtil::sendFitsData(const FitsData& fitsData, const long timeout) noexcept(false) {
    return GeminiUtilImpl::Instance()->sendFitsData(fitsData, timeout);
}

int GeminiUtil::sendFitsData(const FitsData& fitsData, 
                            const long timeout,
                            void (*callback)(int, std::string)) noexcept(false) {
    return GeminiUtilImpl::Instance()->sendFitsData(fitsData, timeout, callback);
}

int GeminiUtil::receiveFitsFiles(void (*callback)(const FitsData&)) noexcept(false) {
    return GeminiUtilImpl::Instance()->receiveFitsFiles(callback);
}

void GeminiUtil::stopReceivingFitsFiles() {
    GeminiUtilImpl::Instance()->stopReceivingFitsFiles();
}

pEpicsStatusItem GeminiUtil::getChannel(const std::string &name, long timeout) noexcept(false)  {
	
  return GeminiUtilImpl::Instance()->getChannel(name, timeout);
}

}


