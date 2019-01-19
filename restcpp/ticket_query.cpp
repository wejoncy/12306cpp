#include "stdafx.h"
#include "ticket_query.h"
TicketInfo::TicketInfo()
{
	typedef std::pair<std::wstring, bool> ticket_show_t;
	ticket_info_vec.push_back(ticket_show_t(L"train_id", true));
	ticket_info_vec.push_back(ticket_show_t(L"from_station", true));
	ticket_info_vec.push_back(ticket_show_t(L"to_station", true));
	ticket_info_vec.push_back(ticket_show_t(L"from_time", true));
	ticket_info_vec.push_back(ticket_show_t(L"to_time", true));
	ticket_info_vec.push_back(ticket_show_t(L"travel_time", true));
	ticket_info_vec.push_back(ticket_show_t(L"sp_class", true));
	ticket_info_vec.push_back(ticket_show_t(L"first_class", true));
	ticket_info_vec.push_back(ticket_show_t(L"second_class", true));
	ticket_info_vec.push_back(ticket_show_t(L"sp_soft_berth", true));
	ticket_info_vec.push_back(ticket_show_t(L"first_soft_berth", true));
	ticket_info_vec.push_back(ticket_show_t(L"sleeper_berth", true));
	ticket_info_vec.push_back(ticket_show_t(L"soft_class", true));
	ticket_info_vec.push_back(ticket_show_t(L"hard_berth", true));
	ticket_info_vec.push_back(ticket_show_t(L"hard_class", true));
	ticket_info_vec.push_back(ticket_show_t(L"none_class", true));
	ticket_info_vec.push_back(ticket_show_t(L"other_class", true));
	ticket_info_vec.push_back(ticket_show_t(L"remark", true));

	ticket_title.push_back(ticket_show_t(L"车次", true));
	ticket_title.push_back(ticket_show_t(L"出发站", true));
	ticket_title.push_back(ticket_show_t(L"到达站", true));
	ticket_title.push_back(ticket_show_t(L"出发时间", true));
	ticket_title.push_back(ticket_show_t(L"达到时间", true));
	ticket_title.push_back(ticket_show_t(L"历时", true));
	ticket_title.push_back(ticket_show_t(L"商务座", true));
	ticket_title.push_back(ticket_show_t(L"一等座", true));
	ticket_title.push_back(ticket_show_t(L"二等座", true));
	ticket_title.push_back(ticket_show_t(L"高级软卧", true));
	ticket_title.push_back(ticket_show_t(L"一等卧", true));
	ticket_title.push_back(ticket_show_t(L"动卧", true));
	ticket_title.push_back(ticket_show_t(L"软座", true));
	ticket_title.push_back(ticket_show_t(L"硬卧", true));
	ticket_title.push_back(ticket_show_t(L"硬座", true));
	ticket_title.push_back(ticket_show_t(L"无座", true));
	ticket_title.push_back(ticket_show_t(L"其他", true));
	ticket_title.push_back(ticket_show_t(L"remark", true));
	_station.init_map();
}
void TicketInfo::add_one_ticket(std::vector<std::wstring>&& tc) {
	std::unordered_map < std::wstring, std::wstring> l_map;
	l_map[L"train_id"] = tc[3];
	l_map[L"from_station"] = _station.g_code_map_city[tc[6]];
	l_map[L"to_station"] = _station.g_code_map_city[tc[7]];
	l_map[L"from_time"] = tc[8];
	l_map[L"to_time"] = tc[9];
	l_map[L"travel_time"] = tc[10];
	l_map[L"sp_class"] = tc[32];
	l_map[L"first_class"] = tc[30];
	l_map[L"second_class"] = tc[31];
	l_map[L"sp_soft_berth"] = tc[25];
	l_map[L"first_soft_berth"] = tc[21];
	l_map[L"hard_berth"] = tc[23];
	l_map[L"sleeper_berth"] = tc[28];
	l_map[L"hard_class"] = tc[29];
	l_map[L"none_class"] = tc[26];
	l_map[L"other_class"] = tc[22];
	l_map[L"remark"] = tc[1];
	ticket_info_map.push_back(l_map); 
}

void TicketInfo::set_show(const std::wstring& c,bool  flag)
{
	for (uint32_t i = 0; i < ticket_info_vec.size(); ++i) {
		if (ticket_title[i].first == c) {
			ticket_title[i].second = false;
			ticket_info_vec[i].second = false;
		}
	}
	
}
std::wstring TicketInfo::title() {
	std::wstring strs;
	for (uint32_t i = 0; i < ticket_title.size(); ++i) {
		if (ticket_title[i].second) {
			strs.append(L"|");
			strs.append(ticket_title[i].first);
		}
	}
	strs.append(L"\n");
	return strs;
}
std::wstring TicketInfo::str()
{
	std::wstring strs;
	for (auto &tim : ticket_info_map) {
		for (int i = 0; i < ticket_info_vec.size(); ++i) {
			if (ticket_info_vec[i].second) {
				strs.append(L" | ");
				strs.append(tim[ticket_info_vec[i].first]);
			}
		}
		strs.append(L"\n");
	}
	return strs;
}

std::vector<std::wstring> split(const std::wstring& srcs, const std::wstring& delim = L"|")
{
	std::vector<std::wstring> elems;
	std::wstring::size_type prev_pos = 0;
	std::wstring::size_type pos = 0;
	while ((pos = srcs.find_first_of(delim, prev_pos)) != std::wstring::npos)
	{
		elems.push_back(srcs.substr(prev_pos, pos - prev_pos));
		prev_pos = pos + 1;
	}
	elems.push_back(srcs.substr(prev_pos));
	return elems;
}
TicketQuery::TicketQuery()
{

}
void TicketQuery::parse(http_response response)
{
	if (response.headers()[L"Content-Type"].find(L"application/json") == std::wstring::npos) {
		return;
	}
	try {

		const web::json::value& jv = response.extract_json().get();
		const web::json::object& jobj = jv.as_object();
		auto result = jobj.at(L"data").as_object().at(L"result").as_array();
		for (auto& line : result)
		{
			_query_tick_vec.add_one_ticket((split(result[0].as_string())));
		}
		print_info_table();
	}
	catch (web::http::http_exception& ex) {
		std::wcout << response.extract_string().get();
		std::wcout << ex.what();
	}
}
void TicketQuery::print_info_table()
{
	/*std::wcout << L"车次 | " << L"出发站->到达站 | " << L"出发时间->达到时间 | " << L"历时 | " << L"商务座 | "
		<< L"一等座 | " << L"二等座 | " << L"高级软卧 | " << L"一等卧 | " << L"动卧 | " << L"软座 | "
		<< L"硬座 | " << L"无座 | " << L"其他 |" << std::endl;*/
	_query_tick_vec.set_show(L"商务座", false);
	_query_tick_vec.set_show(L"高级软卧", false);
	_query_tick_vec.set_show(L"动卧", false);
	_query_tick_vec.set_show(L"软座", false);
	_query_tick_vec.set_show(L"其他", false);
	_query_tick_vec.set_show(L"软座", false);
	_query_tick_vec.set_show(L"无座", false);
	//_query_tick_vec.set_show(L"硬座", false);
	_query_tick_vec.set_show(L"硬卧", false);
	_query_tick_vec.set_show(L"一等座", false);
	_query_tick_vec.set_show(L"一等卧", false);
	std::wcout << _query_tick_vec.title();
	std::wcout << _query_tick_vec.str();
}
void TicketQuery::query_ticket()
{
	http_client client(U("https://kyfw.12306.cn/otn/leftTicket/queryZ"));

	uri_builder builder;
	std::wstring from_station = _query_tick_vec._station.g_city_map_info[L"云梦"].code;
	std::wstring to_station = _query_tick_vec._station.g_city_map_info[L"武汉"].code;
	std::wstring date = U("2019-01-23");
	std::wcout << L"查询" << date << L"从" << from_station << L"到" << to_station << L"的车票\n";
	// Append the query parameters: ?method=flickr.test.echo&name=value
	builder.append_query(U("leftTicketDTO.train_date"), date);
	builder.append_query(U("leftTicketDTO.from_station"), from_station);
	builder.append_query(U("leftTicketDTO.to_station"), to_station);
	builder.append_query(U("purpose_codes"), U("ADULT"));

	auto path_query_fragment = builder.to_string();
	http_request req(methods::GET);
	/*std::wstring cookies =
		L"JSESSIONID=1B8C76DF5E880CCFB405B76E2115CC5F; RAIL_EXPIRATION=1547899209404; "
		"RAIL_DEVICEID=PQMForQsZPTRycxZZS3u8I-IVnfo4Bdbp2YnDRltMFF82Ji85R5_"
		"F7WlvIe9WsXRiYV9d32qevjHC7Due00PXeiemxjQLM6CDgxn0m65IdBC8cQ1ts6nBvHKTJsyOkyKhU3ce-ICQCxMvsi67JPDWkZie-"
		"qbIwhR; route=c5c62a339e7744272a54643b3be5bf64; _jc_save_fromStation=%u5317%u4EAC%2CBJP; "
		"_jc_save_toStation=%u4E0A%u6D77%2CSHH; _jc_save_fromDate=2019-01-23; _jc_save_toDate=2019-01-16; "
		"_jc_save_wfdc_flag=dc; BIGipServerpassport=770179338.50215.0000; BIGipServerotn=183501322.50210.0000";
	http_headers headers;
	headers.add(L"Cookie", cookies);
	req.headers() = headers*/
	req.set_request_uri(path_query_fragment);
	pplx::task<http_response> resp_task = client.request(req);
	http_response response = resp_task.get();

	std::wostringstream stream;
	assert(response.status_code() == 200);
	parse(response);
}
int test_query_ticket_main()
{
	TicketQuery q;
	q.query_ticket();
	return 0;
}

