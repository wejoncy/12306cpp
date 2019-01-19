#include "stdafx.h"
#include "station.h"
#include <memory>
#include <string>
#include <unordered_map>
using namespace pplx;
using namespace utility;
using namespace concurrency::streams;

using namespace web::http;
using namespace web::http::client;
using namespace web::json;
extern std::vector<std::wstring> split(const std::wstring& s, const std::wstring& delim = L"|");


void Station::get_info_from_12306()
{
    http_client client(URL_STATION_NAMES);
    uri_builder builder;
    builder.append_query(U("station_version"), U("1.9053"));
    auto path_query_fragment = builder.to_string();
    http_request req(methods::GET);
    req.set_request_uri(path_query_fragment);    
    http_response response = client.request(req).get();
    uint16_t status_code = response.status_code();
    FILE* fp = fopen("./code.js", "wb");
    assert(fp != NULL);
    std::wstring wtarget = response.extract_string().get();
    fwrite(wtarget.c_str(), 2, wtarget.size(), fp);
    fclose(fp);
}

void Station::init_map()
{
    FILE* fp = fopen("./code.js", "rb");
    if (fp == NULL)
    {
        get_info_from_12306();
        fp = fopen("./code.js", "rb");
        assert(fp != NULL);
    }
    /* 获取文件大小 */
    fseek(fp, 0, SEEK_END);
    int32_t lSize = ftell(fp);
    rewind(fp);

    std::shared_ptr<wchar_t> buf(new wchar_t[lSize + 2]);
    uint32_t result = fread(buf.get(), 1, lSize, fp);
    std::wstring wjs(buf.get(), result);
    // std::wstring wtarget = utility::conversions::utf8_to_utf16(wjs);

    fclose(fp);
    std::vector<std::wstring> code_vec = split(wjs, L"@");
    for (uint32_t i = 1; i < code_vec.size(); ++i)
    {
        std::vector<std::wstring> code_station = split(code_vec[i]);
        StationInfo stationinfo;
        stationinfo.code_ = code_station[0];
        stationinfo.hanzi = code_station[1];
		stationinfo.code = code_station[2];
        stationinfo.pingyin = code_station[3];
        stationinfo.simplepingyin = code_station[4];
        stationinfo.no = code_station[5];
        g_city_map_info[stationinfo.hanzi] = stationinfo;
		g_code_map_city[stationinfo.code] = stationinfo.hanzi;
    }
}

int test_code_main()
{
    std::wcout.imbue(std::locale("chs"));
	Station s;
    s.init_map();
    std::wcout << s.g_city_map_info[L"北京"].code;
    return 0;
}