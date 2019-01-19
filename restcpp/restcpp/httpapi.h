#pragma once
#include "stdafx.h"
#include <unordered_map>

std::unordered_map<std::wstring, std::unordered_map<std::wstring, 
	std::vector<std::pair<std::wstring, std::wstring>>> > g_http_api = {
	{L"login"
		,{{L"login_init",{L"station_version",L"1.9053"}}}
	},
};
