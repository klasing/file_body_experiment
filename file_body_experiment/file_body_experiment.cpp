#define _WIN32_WINNT 0x0601

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/http/basic_file_body.hpp>
#include <boost/beast/http/serializer.hpp>
#include <iostream>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

//****************************************************************************
//*                     foo
//****************************************************************************
class foo
{
public:
	friend std::ostream& operator<<(std::ostream&, const foo&);
};
std::ostream& operator<<(std::ostream& os, const foo& oFoo)
{
	os << "bla\n";
	return os;
}

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
	// show the traditional output stream operator<< overloading
	foo oFoo;
	//std::cout << oFoo;

	// create a <empty_body> request
	http::request<http::empty_body> eb_req;
	eb_req.method(http::verb::get);
	eb_req.target("/");
	eb_req.version(11);
	eb_req.set(http::field::server, "http://www.localhost.com:8080");
	eb_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

	// turn the response message into a string
	auto buff_req = beast::flat_buffer();
	write_message_to_string(eb_req, buff_req);
	std::string requestLogmsg = filter_start_line(
		beast::buffers_to_string(buff_req.data()));
	std::cout << "request....: " << requestLogmsg << std::endl;

	// do work for a <file_body> response
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

	// create a <file_body> response message
	http::response<http::file_body> fb_res{
		std::piecewise_construct,
		std::make_tuple(std::move(body)),
		std::make_tuple(http::status::ok, 11) };
	fb_res.set(http::field::server, "http://www.localhost.com:8080");
	fb_res.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
	fb_res.set(http::field::content_type, "text/html");
	fb_res.content_length(size);
	fb_res.keep_alive(true);

	// turn the response message into a string
	auto buff_res = beast::flat_buffer();
	write_message_to_string(fb_res, buff_res);
	std::string responseLogmsg = filter_start_line(
		beast::buffers_to_string(buff_res.data()));
	std::cout << "response...: " << responseLogmsg << std::endl;

	//std::cout << "HTTP/"
	//	// not considering other protocols than HTTP/1.0 and HTTP/1.1
	//	<< ((fb_res.base().version() == 10) ? "1.0" : "1.1") << ' '
	//	<< fb_res.base().result_int() << ' '
	//	<< fb_res.base().result() << '\n';


	return EXIT_SUCCESS;
}
//http::header<false, http::fields>& hdr_type = fb_res.base();
//std::cout << "bla: " << std::to_string(fb_res.base().version()) << std::endl;
// this calls the operator<<, which has to be implemented, duh......
//std::cout << "bla: " << fb_res << std::endl;
//for (auto const& field : fb_res)
//	std::cout << field.name() << " = " << field.value() << "\n";
