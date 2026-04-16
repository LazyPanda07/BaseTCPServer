#pragma once

#include <string>
#include <stdexcept>

namespace web::exceptions
{
	/// @brief Network exception
	class WebServerException : public std::runtime_error
	{
	private:
		std::string data;
		std::string_view file;
		int errorCode;
		int line;

	private:
		WebServerException();

	public:
		WebServerException(int line, std::string_view file);

		WebServerException(const exceptions::WebServerException& other) = default;

		WebServerException(WebServerException&& other) noexcept = default;

		const char* what() const noexcept override;

		int getErrorCode() const noexcept;

		int getLine() const noexcept;

		std::string_view getFile() const noexcept;

		virtual ~WebServerException() = default;
	};
}

#define THROW_WEB_SERVER_EXCEPTION { throw web::exceptions::WebServerException(__LINE__, __FILE__); }
