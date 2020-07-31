#pragma once

#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <WinSock2.h>

namespace web
{
	class ClientData
	{
	private:
		std::unordered_multimap<std::string, SOCKET> data;
		std::mutex readWriteLock;

	public:
		ClientData() = default;

		void insert(std::string&& ip, SOCKET clientSocket) noexcept;

		std::vector<SOCKET> operator [] (const std::string& ip);

		const std::vector<SOCKET> operator [] (const std::string& ip) const;

		void erase(const std::string& ip) noexcept;

		std::vector<std::pair<std::string, SOCKET>> getClients() noexcept;

		~ClientData() = default;
	};
}