#define _WIN32_WINNT 0x0601

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

//****************************************************************************
//*                     write_message_to_string
//****************************************************************************
template <class Message, class DynamicBuffer>
auto
write_message_to_string(
	Message& message,
	DynamicBuffer& dynamic_buffer
) -> void
{
	beast::ostream(dynamic_buffer) << message;
}

//****************************************************************************
//*                     filter_start_line
//****************************************************************************
inline std::string filter_start_line(const std::string& req_message)
{
	// return first line of a request message
	return req_message.substr(0, req_message.find('\r'));
}

//****************************************************************************
//*                     main
//****************************************************************************
int main()
{
    std::cout << "file_body_experiment\n";
	std::cout << "====================\n";

	std::string path = "";
	path.append("./index.html");
	beast::error_code ec;
	http::file_body::value_type body;
	body.open(path.c_str(), beast::file_mode::scan, ec);
	// Handle the case where the file doesn't exist
	if (ec == beast::errc::no_such_file_or_directory)
	{
		std::cout << "return send(not_found(req.target()));\n";
		return EXIT_FAILURE;
	}
	// Handle an unknown error
	if (ec)
	{
		std::cout << "return send(server_error(ec.message()));\n";
		return EXIT_FAILURE;
	}
	// Cache the size since we need it after the move
	auto const size = body.size();

	// create a response message
	http::response<http::file_body> res{
		std::piecewise_construct,
		std::make_tuple(std::move(body)),
		std::make_tuple(http::status::ok, 11) };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, "text/html");
	res.content_length(size);
	res.keep_alive(true);

	// turn the response message into a string
	auto buff_res = beast::flat_buffer();
	write_message_to_string(res, buff_res);
	std::string responseLogmsg = filter_start_line(
		beast::buffers_to_string(buff_res.data()));

	return EXIT_SUCCESS;
}
