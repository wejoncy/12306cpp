#pragma once
#include "stdafx.h"
#include "request.h"
#include <unordered_map>
using namespace pplx;
using namespace utility;
using namespace concurrency::streams;

using namespace web::http;
using namespace web::http::client;
using namespace web::json;
class Login
{
public:
    Login();
    bool login_init();
    bool download_captcha_image();
    bool captcha_check();
    bool load_cookie();
	void set_auto_choose(bool flag = true);
	bool update_cookie(const std::wstring& ck);
    bool ready_login();
	std::string first_check_login();
	bool second_check_login(const std::string& tk);
	bool third_check_login();
	bool fourth_check_login();
    std::string captcha_auto_choose(const std::wstring& base64img);
    std::string captcha_hand_choose();
private:
    const std::wstring URL_LOGIN_INIT = L"https://kyfw.12306.cn/otn/login/init?station_version=1.9053";
    const std::wstring URL_LOGDEVICE = L"https://kyfw.12306.cn/otn/HttpZF/logdevice?algID=WYEdoc45yu&"
		"hashCode=EhTtj7Znzyie6I21jpgekYReLAnA8fyGEB4VlIGbF0g&FMQw=0&q4f3=zh-CN&VPIf=1&custID=133&"
		"VEek=unknown&dzuS=20.0%20r0&yD16=0&EOQP=895f3bf3ddaec0d22b6f7baca85603c4&lEnu=3232235778&"
		"jp76=e8eea307be405778bd87bbc8fa97b889&hAqN=Win32&platform=WEB&ks0Q=2955119c83077df58dd8bb7832898892&"
		"TeRS=728x1366&tOHY=24xx768x1366&Fvje=i1l1o1s1&q5aJ=-8&wNLf=99115dfb07133750ba677d055874de87&0aew="
		"Mozilla/5.0(WindowsNT6.1;Win64;x64)AppleWebKit/537.36(KHTML,likeGecko)"
		"Chrome/71.0.3578.98Safari/537.36)&E3gR=abfdbb80598e02f8aa71b2b330daa098&timestamp=";

    const std::wstring URL_GETPASSCODENEW_BASE64 = L"https://kyfw.12306.cn/passport/captcha/captcha-image64";
    const std::wstring URL_GETPASSCODENEW = L"https://kyfw.12306.cn/passport/captcha/captcha-image";
    const std::wstring URL_CAPTCHA_CHECK = L"https://kyfw.12306.cn/passport/captcha/captcha-check";
    const std::wstring URL_LOGIN_USERPASS = L"https://kyfw.12306.cn/passport/web/login";
	const std::wstring URL_LOGIN_FIRST_CHECK = L"https://kyfw.12306.cn/passport/web/auth/uamtk";
	const std::wstring URL_LOGIN_SECOND_CHECK = L"https://kyfw.12306.cn/otn/uamauthclient";
	const std::wstring URL_LOGIN_THIRD_CHECK = L"https://kyfw.12306.cn/otn/login/conf";
	const std::wstring URL_LOGIN_FOURTH_CHECK = L"https://kyfw.12306.cn/otn/index/initMy12306Api";

    std::wstring _login_cookie;
    std::unordered_map<std::wstring, std::wstring> _login_cookie_map;
    std::wstring _captcha_img_base64;
	bool _auto_is_choosed = true;
	Requests _sesstion;
};