#pragma once
#include "stdafx.h"
class Station {
public:
	struct StationInfo
	{
		std::wstring code_;
		std::wstring hanzi;
		std::wstring code;
		std::wstring pingyin;
		std::wstring simplepingyin;
		std::wstring no;
	};
	std::unordered_map<std::wstring, StationInfo> g_city_map_info;
	std::unordered_map<std::wstring, std::wstring> g_code_map_city;
private:
	const std::wstring URL_STATION_NAMES = L"https://kyfw.12306.cn/otn/resources/js/framework/station_name.js";
	void get_info_from_12306();
public:
	void init_map();
};
