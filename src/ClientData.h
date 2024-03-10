#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#ifndef __LINUX__
#include <WinSock2.h>
#endif

#ifdef __LINUX__
#ifndef WINDOWS_STYLE_DEFINITION
#define WINDOWS_STYLE_DEFINITION

#define closesocket close
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define DWORD uint32_t

#endif // WINDOWS_STYLE_DEFINITION
#endif // __LINUX__

namespace web
{
	class ClientData
	{
	private:
		std::unordered_map<std::string, std::vector<SOCKET>> data;
		mutable std::mutex dataMutex;

	public:
		ClientData() = default;

		void add(const std::string& ip, SOCKET socket);

		void remove(const std::string& ip, SOCKET socket);

		size_t getNumberOfClients() const;

		size_t getNumberOfConnections() const;

		~ClientData() = default;
	};
}
