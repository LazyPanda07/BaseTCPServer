#pragma once

#ifndef __WEB_EXCEPTION__
#define __WEB_EXCEPTION__

#include <string>
#include <stdexcept>

namespace web
{
	namespace exceptions
	{
		class WebException : public std::runtime_error
		{
		private:
			std::string data;
			int errorCode;

		public:
			WebException();

			WebException(const WebException& other) = default;

			WebException(WebException&& other) noexcept = default;

			const char* what() const noexcept override;

			int getErrorCode() const noexcept;

			~WebException() = default;
		};
	}
}

#endif // !__WEB_EXCEPTION__