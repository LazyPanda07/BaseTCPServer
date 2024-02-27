#include "ClientData.h"

#include <algorithm>

using namespace std;

namespace web
{
	void ClientData::insert(string&& ip, SOCKET clientSocket) noexcept
	{
		unique_lock<mutex> lock(readWriteLock);

		data.emplace(move(ip), clientSocket);
	}

	vector<SOCKET> ClientData::operator [] (const std::string& ip)
	{
		vector<SOCKET> result;

		shared_lock<mutex> lock(readWriteLock);

		auto range = data.equal_range(ip);

		for_each(range.first, range.second, [&result](const auto& data) { result.push_back(data.second); });

		return result;
	}

	void ClientData::erase(const string& ip) noexcept
	{
		unique_lock<mutex> lock(readWriteLock);

		if (data.find(ip) != end(data))
		{
			data.erase(ip);
		}
	}

	vector<pair<string, SOCKET>> ClientData::getClients() noexcept
	{
		vector<pair<string, SOCKET>> result;

		unique_lock<mutex> lock(readWriteLock);

		result.reserve(data.size());

		for (const auto& i : data)
		{
			result.emplace_back(i);
		}

		return result;
	}
}
