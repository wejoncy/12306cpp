#pragma once
#include "stdafx.h"
#include "station.h"
#include <unordered_map>
#include <vector>
using namespace pplx;
using namespace utility;
using namespace concurrency::streams;

using namespace web::http;
using namespace web::http::client;
using namespace web::json;

class TicketInfo
{
public:
	Station _station;
	std::vector<std::unordered_map<std::wstring, std::wstring>> ticket_info_map;
	std::vector<std::pair<std::wstring, bool>> ticket_info_vec;
	std::vector<std::pair<std::wstring, bool>> ticket_title;
	/*std::wstring train_id;
	std::wstring from_station;
	std::wstring to_station;
	std::wstring from_time;
	std::wstring to_time;
	std::wstring travel_time;
	std::wstring sp_class;
	std::wstring first_class;
	std::wstring second_class;
	std::wstring hard_class;
	std::wstring none_class;
	std::wstring other_class;
	std::wstring remark;
	std::wstring sp_soft_berth;
	std::wstring first_soft_berth;
	std::wstring soft_berth;
	std::wstring hard_berth;
	std::wstring sleeper_berth;*/
	TicketInfo();
	void add_one_ticket(std::vector<std::wstring>&& tc);
	void set_show(const std::wstring& c,bool flag);
	//friend std::wostream& operator<<(std::wostream&, const TicketInfo& c);
	std::wstring str();
	std::wstring title();
};
class TicketQuery
{
private:
	TicketInfo _query_tick_vec;
public:
	TicketQuery();
	void parse(http_response response);
	void print_info_table();
	void query_ticket();
};

