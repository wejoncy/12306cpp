#include "stdafx.h"
#include "request.h"

Requests::Requests()
{
}

http_headers Requests::reset_headers(const std::wstring& referer)
{
	http_headers headers;
	headers.add(L"Host", L"kyfw.12306.cn");
	headers.add(L"Origin", L"https://kyfw.12306.cn");
	if (referer.size()) {
		headers.add(L"Referer", referer);
	}
	headers.add(L"Content-Type", L"application/x-www-form-urlencoded;charset=UTF-8");
	headers.add(L"User-Agent", L"Mozilla/5.0(WindowsNT6.1;Win64;x64)AppleWebKit/537.36(KHTML, likeGecko)Chrome/71.0.3578.98Safari/537.36");
	headers.add(L"X-Requested-With", L"XMLHttpRequest");
	return headers;
}

http_response Requests::post(const std::wstring & url, const http_headers & headers, 
	std::wstring & body_data, const utf16string & content_type)
{
	http_client client(url);
	http_request req(methods::POST);
	req.set_body(body_data, content_type);
	req.headers() = reset_headers(L"");
	http_response response = client.request(req).get();
	const http_headers& resheaders = response.headers();
	if (resheaders.find(L"Set-Cookie") != resheaders.end()) {
		_login_cookie = response.headers()[L"Set-Cookie"];
		update_cookie();
	}
	return response;
}
bool Requests::update_cookie()
{
	std::vector<std::wstring> cookie_vec = split(_login_cookie, L";");
	for (auto& c : cookie_vec)
	{
		if (c.empty()) {
			continue;
		}
		std::vector<std::wstring> vs = split(c, L", ");
		if (vs.back().find(L"ath=") != std::wstring::npos) {
			continue;
		}
		std::vector<std::wstring> cookie_map = split(vs.back(), L"=");
		if (cookie_map.size() != 2) {
			continue;
		}
		_login_cookie_map[cookie_map[0]] = cookie_map[1];
	}
	_login_cookie.clear();
	for (auto& m : _login_cookie_map)
	{
		_login_cookie.append(m.first);
		_login_cookie.append(L"=");
		_login_cookie.append(m.second);
		_login_cookie.append(L";");
	}
	_login_cookie.pop_back();
	return false;
}
bool Requests::load_cookie_login() {
	http_headers headers;
	headers.add(L"Host", L"kyfw.12306.cn");
	headers.add(L"Origin", L"https://kyfw.12306.cn");
	headers.add(L"Referer", L"https://kyfw.12306.cn/otn/login/init");
	headers.add(L"Content-Type", L"application/x-www-form-urlencoded;charset=UTF-8");
	headers.add(L"User-Agent", L"Mozilla/5.0(WindowsNT6.1;Win64;x64)AppleWebKit/537.36(KHTML,likeGecko)Chrome/71.0.3578.98Safari/537.36");

	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch());
	std::wstring url = L"https://kyfw.12306.cn/otn/HttpZF/logdevice?algID=WYEdoc45yu&hashCode=EhTtj7Znzyie6I21jpgekYReLAnA8fyGEB4VlIGbF0g&FMQw=0&q4f3=zh-CN&VPIf=1&custID=133&VEek=unknown&dzuS=20.0%20r0&yD16=0&EOQP=895f3bf3ddaec0d22b6f7baca85603c4&lEnu=3232235778&jp76=e8eea307be405778bd87bbc8fa97b889&hAqN=Win32&platform=WEB&ks0Q=2955119c83077df58dd8bb7832898892&TeRS=728x1366&tOHY=24xx768x1366&Fvje=i1l1o1s1&q5aJ=-8&wNLf=99115dfb07133750ba677d055874de87&0aew=" + headers[L"User-Agent"];
	url += L"&E3gR=abfdbb80598e02f8aa71b2b330daa098&timestamp=" + std::to_wstring(ms.count());
	http_client client(url);
	http_request req(methods::GET);
	req.headers() = headers;
	http_response response = client.request(req).get();
	std::wstring jres = response.extract_string().get();
	std::vector<std::wstring> v = split(jres, L"'");
	web::json::value j = web::json::value::parse(v[1]);
	_login_cookie = L";RAIL_EXPIRATION=" + j[L"exp"].as_string();
	_login_cookie += L";RAIL_DEVICEID=" + j[L"dfp"].as_string();
	update_cookie();
	return 0;
}
http_response Requests::get(const std::wstring & url, const http_headers & headers)
{
	http_client client(url);
	http_request req(methods::GET);
	req.headers() = headers;
	http_response response = client.request(req).get();
	const http_headers& resheaders = response.headers();
	if (resheaders.find(L"Set-Cookie") != resheaders.end()) {
		_login_cookie = response.headers()[L"Set-Cookie"];
		update_cookie();
	}
	return response;
}
