#include "ClientData.h"

using namespace std;

namespace web
{
	void ClientData::insert(string&& ip, SOCKET clientSocket) noexcept
	{
		unique_lock<shared_mutex> lock(readWriteLock);

		data.insert(make_pair(move(ip), clientSocket));
	}

	vector<SOCKET> ClientData::operator [] (const std::string& ip)
	{
		vector<SOCKET> result;

		shared_lock<shared_mutex> lock(readWriteLock);

		auto range = data.equal_range(ip);

		for (auto it = range.first; it != range.second; it++)
		{
			result.push_back(it->second);
		}

		return result;
	}

	void ClientData::erase(const string& ip) noexcept
	{
		unique_lock<shared_mutex> lock(readWriteLock);

		if (data.find(ip) != end(data))
		{
			data.erase(ip);
		}
	}

	vector<pair<string, SOCKET>> ClientData::getClients() noexcept
	{
		vector<pair<string, SOCKET>> result;

		unique_lock<shared_mutex> lock(readWriteLock);

		result.reserve(data.size());

		for (const auto& i : data)
		{
			result.emplace_back(i.first, i.second);
		}

		return result;
	}
}
