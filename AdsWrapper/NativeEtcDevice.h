#pragma once

#include "ECatAccess.h"
#include "AdsDefines.h"

class NativeEtcDevice
{
public:
	/// <summary>
	/// Initializes the EtherCAT master access on the remote device.
	/// Must be called before ListAllEtcMasters or GetECatSlaveAlStatus.
	/// </summary>
	NativeEtcDevice(const std::string& remoteIp, const std::string& remoteNetId);

	~NativeEtcDevice();

	/// <summary>
	/// Lists all EtherCAT masters to the provided output stream.
	/// </summary>
	/// <returns>Status code (0 on success)</returns>
	long ListAllEtcMasters(std::ostream& os);

	/// <summary>
	/// Gets the AL status of all EtherCAT slaves across all masters.
	/// </summary>
	/// <returns>Map of AmsNetId to vector of slave AL statuses</returns>
	std::map<AmsNetId, std::vector<uint16_t>> GetECatSlaveAlStatus();

private:
	std::unique_ptr<bhf::ads::ECatAccess> _etcMaster;
	std::unique_ptr<AmsNetId> _remoteAms;   ///< Remote AMS NetId
	std::string _remoteIp;  ///< Remote IP address
};
