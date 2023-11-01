#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
typedef websocketpp::client<websocketpp::config::asio_client> wsclient;

class websocket {
public:
	websocket(std::string uri);
	~websocket();
	void on_open(wsclient* c, websocketpp::connection_hdl hdl);
	void on_message(websocketpp::connection_hdl, wsclient::message_ptr msg);
	void on_disconnect(wsclient* c, websocketpp::connection_hdl hdl);
	void send(std::string message);
	std::string get_status() const;
private:
	void connect();
	wsclient m_endpoint;
	std::string m_uri;
	websocketpp::connection_hdl m_hdl;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
	std::string m_status;
};

extern websocket ws;
extern std::string lastFromWs;