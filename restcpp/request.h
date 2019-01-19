#pragma once
#include "stdafx.h"
using namespace pplx;
using namespace utility;
using namespace concurrency::streams;

using namespace web::http;
using namespace web::http::client;
using namespace web::json;
extern std::vector<std::wstring> split(const std::wstring& srcs, const std::wstring& delim = L"|");
class Requests {
private:
	std::wstring _login_cookie;
	std::unordered_map<std::wstring, std::wstring> _login_cookie_map;
public:
	Requests();
	http_headers reset_headers(const std::wstring& ferer);
	http_response post(const std::wstring& url, const http_headers& headers,
		std::wstring& body_data, const utf16string& content_type);
	http_response get(const std::wstring& url, const http_headers& headers);
	bool update_cookie();
	bool load_cookie_login();
};