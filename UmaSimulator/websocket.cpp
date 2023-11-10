#include <codecvt>
#include "websocket.h"

std::string lastFromWs;
websocket::websocket(std::string uri)
	: m_status("Connecting")
	, m_uri(uri)
{
	if (uri == "")
		return;
	m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
	m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
	m_endpoint.init_asio();
	m_endpoint.start_perpetual();
	m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&wsclient::run, &m_endpoint);

	connect();
}
websocket::~websocket() {
	if (m_uri == "")
		return;
	m_endpoint.stop_perpetual();

	websocketpp::lib::error_code ec;
	m_endpoint.close(m_hdl, websocketpp::close::status::going_away, "", ec);
	if (ec) {
		std::cout << "> Error closing connection: " << ec.message() << std::endl;
	}

	m_thread->join();
}
void websocket::on_open(wsclient* c, websocketpp::connection_hdl hdl) {
	m_status = "Open";
}
void websocket::on_message(websocketpp::connection_hdl, wsclient::message_ptr msg) {
	if (msg->get_opcode() == websocketpp::frame::opcode::text) {
		const std::string json = msg->get_payload();
		lastFromWs = json;
	}
	else {
		std::cout << websocketpp::utility::to_hex(msg->get_payload()) << std::endl;
	}
}
void websocket::on_disconnect(wsclient* c, websocketpp::connection_hdl hdl) {
	m_status = "Lost";
	auto con = c->get_con_from_hdl(hdl);
	std::cout 
		<< "与URA的websocket连接已断开："
		<< con->get_ec().message()
		<< std::endl;
	connect();
}
void websocket::connect() {

	websocketpp::lib::error_code ec;
	wsclient::connection_ptr con = m_endpoint.get_connection(m_uri, ec);
	m_hdl = con->get_handle();
	if (ec) {
		std::cout << "> Connect initialization error: " << ec.message() << std::endl;
		return;
	}

	con->set_open_handler(websocketpp::lib::bind(
		&websocket::on_open,
		this,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_message_handler(websocketpp::lib::bind(
		&websocket::on_message,
		this,
		websocketpp::lib::placeholders::_1,
		websocketpp::lib::placeholders::_2
	));
	con->set_fail_handler(websocketpp::lib::bind(
		&websocket::on_disconnect,
		this,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_close_handler(websocketpp::lib::bind(
		&websocket::on_disconnect,
		this,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));

	m_endpoint.connect(con);
}
void websocket::send(std::wstring message) {
	websocketpp::lib::error_code ec;
	static std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
	std::string s = conv.to_bytes(message);

	m_endpoint.send(m_hdl, s, websocketpp::frame::opcode::text, ec);
	if (ec) {
		std::cout << "> Error sending websocket message: " << ec.message() << std::endl;
		return;
	}
}
std::string websocket::get_status() const {
	return m_status;
}