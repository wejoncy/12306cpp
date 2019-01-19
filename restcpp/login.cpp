#include "login.h"
#include "stdafx.h"
#include <chrono>
#include <windows.h>
#include "login.h"

extern std::vector<std::wstring> split(const std::wstring& srcs,  const std::wstring& delim);
Login::Login() {}
bool Login::login_init()
{
	std::wcout << L"正在登陆。。。\n";
    http_client client(URL_LOGIN_INIT);
    http_headers headers = _sesstion.reset_headers(L"");
    http_request req(methods::GET);
    req.headers() = headers;
    http_response response = client.request(req).get();
    status_code status = response.status_code();
    assert(status == 200);
    _login_cookie = response.headers()[L"Set-Cookie"];
	update_cookie(_login_cookie);
	load_cookie();
	std::wcout << L"登陆初始化\n";
    return true;
}
bool Login::download_captcha_image()
{
	std::wcout << L"准备下载验证码\n";
	std::wstring url = URL_GETPASSCODENEW;
	if (_auto_is_choosed) {
		url = URL_GETPASSCODENEW_BASE64;
	}
	http_client client(url);
	uri_builder builder;
	builder.append_query(U("login_site"), U("E"));
	builder.append_query(U("module"), U("login"));
	builder.append_query(U("rand"), U("sjrand"));
	double drand = rand() * 1.000000 / RAND_MAX;
	std::wostringstream osUrl;
	osUrl.precision(17);
	osUrl << drand;
	builder.append_query(osUrl.str());
	auto path_query_fragment = builder.to_string();
	http_request req(methods::GET);
	req.headers() = _sesstion.reset_headers(L"");
	req.headers().add(L"Cookie", _login_cookie);
	req.set_request_uri(path_query_fragment);
	http_response response = client.request(req).get();
	_login_cookie = response.headers()[L"Set-Cookie"];
	update_cookie(_login_cookie);

	if (_auto_is_choosed) {
		if (response.headers()[L"Content-Type"].find(L"application/json") == std::wstring::npos) {
			std::wcout << L"访问太频繁了,等一会儿吧\n";
			Sleep(1000);
			return false;
		}
		auto jres = response.extract_json().get().as_object();
		_captcha_img_base64 = jres[(L"image")].as_string();
	}
	else {
		if (response.headers()[L"Content-Type"].find(L"image/jpeg") == std::wstring::npos) {
			std::wcout << L"访问太频繁了,等一会儿吧\n";
			Sleep(1000);
			return false;
		}
		 FILE* fp = fopen("./captcha.jpg",  "wb");
		 assert(fp != NULL);
		 auto bodyStream = response.body();
		 streams::stringstreambuf sbuffer;
		 auto& target = sbuffer.collection();

		 bodyStream.read_to_end(sbuffer).get();
		 fwrite(target.c_str(),  1,  target.size(),  fp);
		 fclose(fp);
	}
	std::wcout << L"下载验证码成功\n";
	Sleep(1000);
	return true;
}
bool Login::load_cookie() {
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
	std::wstring url = URL_LOGDEVICE + std::to_wstring(ms.count());
    http_client client(url);
    http_request req(methods::GET);
    req.headers() = _sesstion.reset_headers(L"https://kyfw.12306.cn/otn/login/init");
    http_response response = client.request(req).get();
    std::wstring jres = response.extract_string().get();
    std::vector<std::wstring> v = split(jres, L"'");
    web::json::value j = web::json::value::parse(v[1]);
    _login_cookie = L";RAIL_EXPIRATION=" + j[L"exp"].as_string();
    _login_cookie += L";RAIL_DEVICEID=" + j[L"dfp"].as_string();
	update_cookie(_login_cookie);
    return 0;
}
void Login::set_auto_choose(bool flag)
{
	_auto_is_choosed = flag;
}
bool Login::update_cookie(const std::wstring& ck)
{
	_login_cookie = ck;
	std::vector<std::wstring> cookie_vec = split(_login_cookie,  L";");
	for (auto& c : cookie_vec)
	{
		if (c.empty()) {
			continue;
		}
		std::vector<std::wstring> vs = split(c,  L", ");
        if (vs.back().find(L"ath=") != std::wstring::npos) {
            continue;
        }
		std::vector<std::wstring> cookie_map = split(vs.back(),  L"=");
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

bool Login::captcha_check()
{
	std::wcout << L"准备校验验证码\n";
    http_client client(URL_CAPTCHA_CHECK);
    http_request req(methods::POST);
    std::string param;
    if (!_auto_is_choosed) {
        param = captcha_hand_choose();
    }
    else {
		std::wcout << L"选择自动校验\n";
        param = captcha_auto_choose(_captcha_img_base64);
    }
    req.set_body(param,  "application/x-www-form-urlencoded;charset=UTF-8");
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    req.headers() = _sesstion.reset_headers(L"https://kyfw.12306.cn/otn/login/init");;
	req.headers().add(L"Cookie", _login_cookie);
	Sleep(1000);
	http_response response;
	try {
		response = client.request(req).get();
	}
	catch (web::http::http_exception &ex) {
		std::wcout << ex.what();
		return false;
	}
    _login_cookie = response.headers()[L"Set-Cookie"];
	update_cookie(_login_cookie);
    if (response.headers()[L"Content-Type"] == L"text/html")
    {
        std::wcout << L"captcha_check:稍等一会，再来一次吧\n";
		Sleep(1000);
        return false;
    }
    try
    {
        const web::json::value& jres = response.extract_json().get();
        const web::json::object& jsonv = jres.as_object();
        if (jsonv.size() == 0) {
            std::wcout << "jsonv is emtpy" << std::endl;
			Sleep(1000);
            return false;
        }
		std::wcout << jsonv.at(L"result_message").as_string() << std::endl;
        const web::json::value& result_code = jsonv.at(L"result_code");
        if (result_code.is_integer()) {
            if (result_code.as_integer() != 4) {
				Sleep(1000);
                return false;
            }
        }
        else if (result_code.is_string()) {
            if (result_code.as_string() != L"4") {
				Sleep(1000);
                return false;
            }
        }
    }
    catch (web::http::http_exception &ex){
        std::wcout << ex.what() << std::endl;;
    }
	Sleep(1000);
    return true;
}
bool Login::ready_login() {
	std::wcout << L"准备登陆。。。\n";
    http_client client(URL_LOGIN_USERPASS);
    http_request req(methods::POST);
	FILE *fp = fopen("config.json", "rb");
	std::wstring username;
	std::wstring passwd;
	if (fp == NULL) {
		std::wcout << L"输入你的12306用户名:";
		std::wcin >> username;
		std::wcout << L"\n输入你的12306密码:";
		std::wcin >> passwd;
		std::wcout << std::endl;
		fp = fopen("config.json", "wb");
		web::json::value config;
		config[L"username"] = web::json::value::string(username);
		config[L"passwd"] = web::json::value::string(passwd);
		fwrite(config.serialize().c_str(), 1, config.serialize().size()*2, fp);
		fclose(fp);
	}
	else {
		fseek(fp, 0, SEEK_END);
		int32_t lSize = ftell(fp);
		rewind(fp);
		std::shared_ptr<wchar_t> buf(new wchar_t[lSize + 2]);
		int32_t result = fread(buf.get(), 1, lSize, fp);
		std::wstring wjs(buf.get(), result/2);
		web::json::value jconf = web::json::value::parse(wjs);
		username = jconf.at(L"username").as_string();
		passwd = jconf.at(L"passwd").as_string();
		fclose(fp);
	}
    std::string param = "username=" + utility::conversions::utf16_to_utf8(username);
	param += "&password=" + utility::conversions::utf16_to_utf8(passwd);
    param += "&appid=otn";
    req.set_body(param, "application/x-www-form-urlencoded;charset=UTF-8");
    req.headers() = _sesstion.reset_headers(L"https://kyfw.12306.cn/otn/login/init");
	req.headers().add(L"Cookie", _login_cookie);
    http_response response = client.request(req).get();
    _login_cookie = response.headers()[L"Set-Cookie"];
    update_cookie(_login_cookie);
    status_code status = response.status_code();
    assert(status == 200);
    if (response.headers()[L"Content-Type"] == L"text/html")
    {
        std::wcout << L"ready_login :稍等一会，再来一次吧\n";
		Sleep(1000);
        return false;
    }

    const web::json::value& jres = response.extract_json().get();
    const web::json::object& jsonv = jres.as_object();
    if (jsonv.size() == 0) {
        std::wcout << "jsonv is emtpy" << std::endl;
		Sleep(1000);
        return false;
    }
	std::wcout << jsonv.at(L"result_message").as_string() << std::endl;

    const web::json::value& result_code = jsonv.at(L"result_code");
    if (result_code.is_integer()) {
        if (result_code.as_integer() != 0) {
			Sleep(1000);
            return false;
        }
    }
    else if (result_code.is_string()) {
        if (result_code.as_string() != L"0") {
			Sleep(1000);
            return false;
        }
    }
	_login_cookie = L"uamtk=";
	_login_cookie += jsonv.at(L"uamtk").as_string();
	update_cookie(_login_cookie);
	Sleep(600);
	auto tkparam = first_check_login();
	if (tkparam.empty()) {
		Sleep(1000);
		return false;
	}
	Sleep(600);
	if (!second_check_login(tkparam)) { return false; }
	if (!third_check_login()) { return false; }
	//if (!fourth_check_login()) { return false; }
	return true;
}
/*
 #4 用户登录第一次验证+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
std::string Login::first_check_login() {
	std::string param;
	http_client client(URL_LOGIN_FIRST_CHECK);
	http_request req(methods::POST);
	req.headers() = _sesstion.reset_headers(L"https://kyfw.12306.cn/otn/passport?redirect=/otn/login/userLogin");
	req.headers().add(L"Cookie", _login_cookie);
	req.set_body("appid=otn", "application/x-www-form-urlencoded;charset=UTF-8");
	http_response response = client.request(req).get();
	if (response.headers()[L"Content-Type"].find(L"application/json") == std::wstring::npos) {
		std::wcout << L"访问太频繁了,等一会儿吧\n";
		Sleep(1000);
		return param;
	}

	auto userVerify = response.extract_json().get();
	std::wcout << L"第一次验证\n";
	std::wcout << userVerify.at(L"result_message")<<"\n";
	if (userVerify[L"result_code"].as_integer() != 0) {
		std::wcout << L"验证失败(uamtk) code:" << userVerify[L"result_code"] << "\n";
		return param;
	}
	param = "tk=";
	param += utility::conversions::utf16_to_utf8(userVerify[L"newapptk"].as_string());
	return param;
}
/*/
#5 用户登录第二次验证++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
bool Login::second_check_login(const std::string& param) {
	http_client client(URL_LOGIN_SECOND_CHECK);
	http_request req(methods::POST);
	_login_cookie_map.erase(L"uamtk");
	update_cookie(L"");
	req.headers() = _sesstion.reset_headers(L"https://kyfw.12306.cn/otn/passport?redirect=/otn/login/userLogin");
	req.headers().add(L"Cookie", _login_cookie);
	req.set_body(param, "application/x-www-form-urlencoded;charset=UTF-8");
	http_response response = client.request(req).get();
	if (response.headers()[L"Content-Type"].find(L"application/json") == std::wstring::npos) {
		std::wcout << L"访问太频繁了,等一会儿吧\n";
		Sleep(1000);
		return false;
	}

	auto userVerify = response.extract_json().get();
	std::wcout << L"第二次验证\n";
	std::wcout << userVerify.at(L"result_message") << "\n";
	if (userVerify[L"result_code"].as_integer() != 0) {
		std::wcout << L"验证失败(uamtk) code:" << userVerify[L"result_code"] << "\n";
		Sleep(1000);
		return false;
	}
	std::wcout << userVerify[L"username"].as_string() << L"你好，欢迎回来\n";
	return true;
}
bool Login::third_check_login() {

	http_client client(URL_LOGIN_THIRD_CHECK);
	http_request req(methods::POST);
	http_headers headers;
	headers.add(L"Host", L"kyfw.12306.cn");
	headers.add(L"Referer", L"https://kyfw.12306.cn/otn/view/index.html");
	headers.add(L"X-Requested-With", L"XMLHttpRequest");
	headers.add(L"User-Agent", L"Mozilla/5.0(WindowsNT6.1;Win64;x64)AppleWebKit/537.36(KHTML,likeGecko)Chrome/71.0.3578.98Safari/537.36");
	headers.add(L"Cookie", _login_cookie);
	req.headers() = headers;
	Sleep(1000);
	http_response response = client.request(req).get();
	if (response.headers()[L"Content-Type"].find(L"application/json") == std::wstring::npos) {
		std::wcout << L"访问太频繁了,等一会儿吧\n";
		Sleep(1000);
		return false;
	}

	auto userVerify = response.extract_json().get();
	std::wcout << L"第三次验证\n";
	if (userVerify[L"status"].as_bool() != true) {
		std::wcout << L"验证失败(uamtk) code:" << userVerify[L"result_code"] << "\n";
		Sleep(1000);
		return false;
	}
	std::wcout << L"验证成功\n";
	return true;
}
bool Login::fourth_check_login() {
	
	http_client client(URL_LOGIN_FOURTH_CHECK);
	http_request req(methods::POST);
	http_headers headers;
	headers.add(L"Host", L"kyfw.12306.cn");
	headers.add(L"Referer", L"https://kyfw.12306.cn/otn/view/index.html");
	headers.add(L"X-Requested-With", L"XMLHttpRequest");
	headers.add(L"User-Agent", L"Mozilla/5.0(WindowsNT6.1;Win64;x64)AppleWebKit/537.36(KHTML,likeGecko)Chrome/71.0.3578.98Safari/537.36");
	_login_cookie_map.erase(L"Max-Age");
	_login_cookie_map.erase(L"_passport_session");
	_login_cookie_map.erase(L"_passport_ct");
	update_cookie(L"");
	headers.add(L"Cookie", _login_cookie);
	req.headers() = headers;
	Sleep(1000);
	std::wcout << L"第四次验证\n";
	http_response response = client.request(req).get();
	if (response.headers()[L"Content-Type"].find(L"application/json") == std::wstring::npos) {
		auto resstr = response.extract_string().get();
		std::wcout << resstr << L"访问太频繁了,等一会儿吧\n";
		Sleep(1000);
		return false;
	}

	auto userVerify = response.extract_json().get();
	auto jdata_filed = userVerify.as_object()[L"data"];
	if (userVerify[L"status"].as_bool() != true) {
		std::wcout << L"验证失败(uamtk) code:" << userVerify[L"result_code"] << "\n";
		Sleep(1000);
		return false;
	}
	std::wcout << jdata_filed[L"user_name"].as_string() + jdata_filed[L"user_regard"].as_string();
	return true;
}

std::string Login::captcha_hand_choose() {
    const std::vector<std::string> coordinates = {
    "46,40","110,43","179,41","251,42","46,118","114,109","180,115","252,124" };
    std::string param = "answer=";

    system("start ./captcha.jpg");
    std::cout << "select a img: ";
    std::wstring jpgsel;
    std::wcin >> jpgsel;
    std::vector<std::wstring> sels = split(jpgsel, L", ");
    for (auto& c : sels)
    {
        int jpg_index = int(c[0] - L'0');
        param += coordinates[jpg_index - 1] + ", ";
    }
    param.pop_back();
    param += "&rand=sjrand"; // passenger:randp
    param += "&ogin_site=E"; // passenger:randp
    return param;
}
std::string Login::captcha_auto_choose(const std::wstring& base64img) {
	std::wcout << L"准备请求验证码答案\n";
    std::string param;
    std::wstring apiurl = L"http://60.205.200.159/api";
    std::wstring img_url = L"http://check.huochepiao.360.cn/img_vcode";

    http_client client(apiurl);
    web::json::value obj;
    obj[L"base64"] = web::json::value::string(base64img);
    auto response = client.request(methods::POST, L"", obj.serialize(), 
        L"application/json").get().extract_json().get();
    if (response.as_object()[L"success"].as_bool() != true) {
        std::wcout << response.as_object()[L"msg"] << "\n";
        Sleep(1000);
        return param;
    }
    http_client client1(img_url);
    web::json::value obj1;
    obj1[L"img_buf"] = web::json::value::string(base64img);
    obj1[L"check"] = web::json::value::string(response.as_object()[L"check"].as_string());
    obj1[L"logon"] = web::json::value::number(1);
    obj1[L"type"] = web::json::value::string(L"D");
    auto response1 = client1.request(methods::POST, L"", obj1.serialize(),
        L"application/json").get();
    web::json::value coordres = web::json::value::parse(response1.extract_string().get());

    std::wstring axis = coordres.as_object()[L"res"].as_string();
    axis.erase(std::remove(axis.begin(), axis.end(), L'('), axis.end());
    axis.erase(std::remove(axis.begin(), axis.end(), L')'), axis.end());
    param = "answer=";
    param += utility::conversions::utf16_to_utf8(axis);
    param += "&rand=sjrand"; // passenger:randp
    param += "&ogin_site=E"; // passenger:randp
	std::wcout << L"获取到验证码答案\n";
    return param;
}
int test_login_main()
{
#define continue_if(cond)  if (!cond) {continue;}
    std::wcout.imbue(std::locale("chs"));
    Login l;
    bool f = false;
	l.set_auto_choose(!f);
    while(f == false){
        f = l.login_init();
        continue_if(f);
        f = l.download_captcha_image();
        continue_if(f);
        f = l.captcha_check();
        f = l.ready_login();

    }
    return 0;
}