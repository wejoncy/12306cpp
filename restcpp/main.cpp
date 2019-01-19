// The code includes the most frequently used includes necessary to work with C++ REST SDK
#include "stdafx.h"


using namespace pplx;
using namespace utility;
using namespace concurrency::streams;

using namespace web::http;
using namespace web::http::client;
using namespace web::json;
#pragma comment(lib, "cpprest141_2_10d.lib")
int test_query_ticket_main();
int test_login_main();
int main(int argc, char* args[])
{
    std::wcout.imbue(std::locale("chs"));
    std::wcout << L"Calling HTTPGetAsync..." << std::endl;
    // In this case, I call wait. However, you won¡¯t usually want to wait for the asynchronous operations
    test_query_ticket_main();
	test_login_main();
    return 0;
}