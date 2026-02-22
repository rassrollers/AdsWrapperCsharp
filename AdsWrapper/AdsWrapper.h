#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#include <msclr/marshal_cppstd.h>

#include <iostream>
#include "AdsLib.h"
#include "AdsDevice.h"
#include <string>

using namespace System;

namespace AdsWrapper {
	public ref class AdsHandler
	{
	private:
		AmsNetId* localAms;
		AmsNetId* remoteAms;
		std::string* remoteIp;
		AdsDevice* route;
	public:
		AdsHandler(String^ localIpAddr)
		{
			std::string nativeIp = msclr::interop::marshal_as<std::string>(localIpAddr);

			std::cout << "Setting local IP address to: " << nativeIp << std::endl;

			localAms = new AmsNetId(nativeIp + ".1.1");
			bhf::ads::SetLocalAddress(nativeIp);
		}

		void AddRemoteRoute(String^ remoteIpAddr, String^ routeName, String^ userName, String^ password)
		{
			std::string ip = msclr::interop::marshal_as<std::string>(remoteIpAddr);
			std::string rName = msclr::interop::marshal_as<std::string>(routeName);
			std::string uName = msclr::interop::marshal_as<std::string>(userName);
			std::string pwd = msclr::interop::marshal_as<std::string>(password);

			remoteIp = new std::string(ip);
			remoteAms = new AmsNetId(ip + ".1.1");

			std::cout << "Adding route to: " << remoteAms << " with route name: " << rName << std::endl;

			bhf::ads::AddRemoteRoute(ip, *remoteAms, ip, rName, uName, pwd);
		}

		void SetupRoute(UInt16 AmsPort)
		{
			std::cout << "Setting up route to: " << *remoteAms << " on port: " << AmsPort << std::endl;
			route = new AdsDevice(*remoteIp, *remoteAms, AmsPort);
		}

		void PrintState()
		{
			try
			{
				AdsDeviceState state = route->GetState();
				std::cout << "ADS State: " << state.ads << ", Device State: " << state.device << std::endl;
			}
			catch (const std::exception& ex)
			{
				std::cerr << "Error getting device state: " << ex.what() << std::endl;
				return;
			}
		}

		~AdsHandler()       // destructor
		{
			this->!AdsHandler();
		}

		!AdsHandler()       // finalizer
		{
			delete localAms;
			delete remoteAms;
			delete remoteIp;
			delete route;
		}
	};
}
