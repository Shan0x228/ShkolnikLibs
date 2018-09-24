#include "../src/AtomHTTP.hpp"
#include <iostream>

using namespace std;
using namespace Shkolnik::net;

int main()
{
    try
    {
        AtomHTTP test(std::string("www.ya.ru"), 80);
        test.request("/", AtomHTTP::HTTP::GET);
		test.add_header("Host: ya.ru");
		//test.add_header("User-Agent: insomnia/6.0.2");
		//test.add_header("Cookie: i=gG/E/FSv2aGCt9g0G9fCEQ+b0WC4eyz1izdhKpilsS0egiXhIBHN/4nr6alL0pSPxD1XUk4rZwxYkH4tEY1E2caF1n0=; mda=0; yandex_gid=213; yandexuid=4112269021537453139; yp=1540045141.ygu.1");
		//test.add_header("Accept: */*");

		cout << test.send_request() << '\n';
        cout << test.get_content() << '\n';
    }
    catch (std::string &e)
    {
        cout << e << '\n';
    }

    cin.get();
}