#pragma once

#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <future>

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
		std::unordered_multimap<std::string, std::pair<SOCKET, std::future<void>>> data;
		std::mutex readWriteLock;

	public:
		ClientData() = default;

		void insert(std::string&& ip, SOCKET clientSocket, std::future<void>&& servingFunction) noexcept;

		std::vector<std::pair<SOCKET, std::future<void>>> operator [] (const std::string& ip);

		void erase(const std::string& ip) noexcept;

		std::vector<std::pair<std::string, SOCKET>> getClients() noexcept;

		~ClientData() = default;
	};
}
