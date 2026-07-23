#include "pch.h"
#include "NativeEtcDevice.h"
#include "NativeLogger.h"

NativeEtcDevice::NativeEtcDevice(const std::string& remoteIp, const std::string& remoteNetId)
{
    _remoteIp = remoteIp;
    _remoteAms = std::make_unique<AmsNetId>(remoteNetId);

    NativeLogger::Instance().Log(NativeLogger::LogLevel::Debug,
        "Creating EtherCAT Master device for remote AMS " + AdsWrapper::AmsToString(*_remoteAms) + " on port " + std::to_string(AMSPORT_R0_IO));
    _etcMaster = std::make_unique<bhf::ads::ECatAccess>(_remoteIp, *_remoteAms, AMSPORT_R0_IO);
}

NativeEtcDevice::~NativeEtcDevice() = default;

long NativeEtcDevice::ListAllEtcMasters(std::ostream& os)
{
    if (!_etcMaster) {
        throw std::runtime_error("ECatAccess not initialized. Call SetupEtcMaster first.");
    }
    return _etcMaster->ListECatMasters(os);
}

std::map<AmsNetId, std::vector<uint16_t>> NativeEtcDevice::GetECatSlaveAlStatus()
{
    if (!_etcMaster) {
        throw std::runtime_error("ECatAccess not initialized. Call SetupEtcMaster first.");
    }
    return _etcMaster->GetECatSlaveAlStatus();
}