#pragma once

#ifndef __WEB_EXCEPTION__
#define __WEB_EXCEPTION__

#include <string>

namespace web
{
	class WebException
	{
	private:
		std::string data;

	public:
		WebException();

		WebException(const WebException& other) = default;

		WebException(WebException&& other) noexcept;

		const char* what() const noexcept;

		~WebException() = default;
	};
}

#endif // !__WEB_EXCEPTION__