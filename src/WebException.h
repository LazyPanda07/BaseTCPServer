#pragma once

#ifndef __WEB_EXCEPTION__
#define __WEB_EXCEPTION__

#include <string>
#include <stdexcept>

namespace web
{
	class WebException : public std::runtime_error
	{
	private:
		std::string data;

	public:
		WebException();

		WebException(const WebException& other) = default;

		WebException(WebException&& other) noexcept;

		const char* what() const noexcept override;

		~WebException() = default;
	};
}

#endif // !__WEB_EXCEPTION__