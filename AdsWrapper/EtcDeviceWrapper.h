#pragma once

#include "AdsDefines.h"

using namespace System;

class NativeEtcDevice;

namespace AdsWrapper
{
	public ref class EtcDeviceWrapper
	{
	public:
        /// <summary>
        /// Initializes the EtherCAT master access on the remote device.
        /// Must be called before ListAllEtcMasters or GetECatSlaveAlStatus.
        /// </summary>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
		EtcDeviceWrapper(String^ remoteIp, String^ remoteNetId);

        /// <summary>
        /// Managed destructor. Releases managed and unmanaged resources.
        /// </summary>
		~EtcDeviceWrapper();

        /// <summary>
        /// Finalizer. Releases unmanaged resources.
        /// </summary>
		!EtcDeviceWrapper();

        /// <summary>
        /// Checks if the object has been disposed and throws if true.
        /// </summary>
        /// <exception cref="ObjectDisposedException">Thrown if object is already disposed</exception>
        void CheckDisposed();

        /// <summary>
        /// Lists all EtherCAT masters on the remote device.
        /// </summary>
        /// <returns>String containing the list of EtherCAT masters</returns>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
        String^ ListAllEtcMasters();

        /// <summary>
        /// Gets the AL status of all EtherCAT slaves across all masters.
        /// </summary>
        /// <returns>Array of ECatMasterSlaveStatus, one entry per EtherCAT master</returns>
        /// <exception cref="ObjectDisposedException">Thrown if object is disposed</exception>
        array<ECatMasterSlaveStatus>^ GetECatSlaveAlStatus();

	private:
		NativeEtcDevice* _native;
        bool _disposed = false;
	};
}