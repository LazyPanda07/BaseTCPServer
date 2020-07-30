#include "ClientData.h"

using namespace std;

namespace web
{
	void ClientData::insert(string&& ip, SOCKET clientSocket) noexcept
	{
		readWriteLock.lock();

		data.insert(make_pair(move(ip), clientSocket));

		readWriteLock.unlock();
	}

	SOCKET& ClientData::operator [] (const std::string& ip)
	{
		return data.at(ip);
	}

	const SOCKET& ClientData::operator [] (const std::string& ip) const
	{
		return data.at(ip);
	}

	void ClientData::erase(const string& ip) noexcept
	{
		readWriteLock.lock();

		if (data.find(ip) != end(data))
		{
			data.erase(ip);
		}

		readWriteLock.unlock();
	}

	vector<pair<string, SOCKET>> ClientData::getClients() noexcept
	{
		vector<pair<string, SOCKET>> result;

		readWriteLock.lock();

		result.reserve(data.size());

		for (const auto& i : data)
		{
			result.emplace_back(i.first, i.second);
		}

		readWriteLock.unlock();

		return result;
	}
}