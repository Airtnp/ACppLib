#include "sn_CommonHeader_test.h"
#include "sn_Alg_test.hpp"
#include "sn_Decimal_test.hpp"
#include "sn_Assist_test.hpp"
#include "sn_Reflection_test.hpp"
#include "sn_String_test.hpp"
#include "sn_Log_test.hpp"

#ifdef SN_TEST_DB

namespace sn_DBConfig {
	using namespace std;
	const string user = "root";
	const string pswd = "";
	const string host = "localhost";
	const string db = "FSA";
	const unsigned int port = 3306;
}


using namespace sn_DBConfig;

shared_ptr<sn_DBPImpl::SQLConnPoolBase> sn_DBPImpl::SQLConnPoolBase::connPool = nullptr;
once_flag sn_DBPImpl::SQLConnPoolBase::createFlag = {};
auto res = sn_DBPImpl::DBPConfig::setConfig(user, pswd, host, db, port, 10);
auto sql_pool = sn_DBPImpl::SQLConnPoolBase::getInstance();
auto sql_pool_gc = sn_DBPImpl::SQLConnPoolBase::gc_connPool;
auto conn = sql_pool->getConnection();


namespace sn_DB_test {

	using namespace std;


	

	string eraseFirstTab(const string& str) {
		auto pos = str.find_first_of("\t");
		if (pos != str.npos)
			return str.substr(pos + 1, str.length());
		return str;
	}



	void sn_DB_test() {

		using namespace sn_DBImpl::DBHelper;

		vector<vector<string>> info_set = {
			{ "123", "234", "2333" },
			{ "233", "admin", "admin" }
		};

		string output = "";
		for (auto storage_info : info_set) {
			output += accumulate(next(storage_info.begin()), storage_info.end(), storage_info.at(0), [](string a, string b) { return a + "\t" + b;  });
			output += "\n";
		}
		wcout << string_to_wstring(output) << endl;

		auto test_str = make_tuple("test", output);

		auto pstr = eraseFirstTab(move(get<1>(test_str)));
		auto pstr2 = eraseFirstTab(move(get<0>(test_str)));

		wstring test = L"日";
		string hex_text = wstring_to_hex(test);

		string username = "日";
		string hex_username = wstring_to_unhex(string_to_wstring(username));

		string detail = "日,123,admin,沃日";
		auto d_list = string_split<wstring>(string_to_wstring(detail), L",", split_option::empty_remain);

		vector<pair<string, string>> conds = { make_pair("hex(username)", wstring_to_hex(test)) };
		vector<pair<string, string>> values = { make_pair("username", string_to_unhex(username)) };
		vector<pair<string, string>> values2 = { make_pair("id", "2") };


		auto res_3 = conn->op_insert("user", values);

		auto res = conn->op_select("user");
		auto res_2 = conn->op_update("user", conds, values2);
		auto res_4 = conn->op_delete("user", conds);
		wstring test_2 = utf8_to_wstring(get<0>(res).front().at(0));
		wcout.imbue(locale("chs"));
		wcout << utf8_to_wstring(get<0>(res).front().at(0)) << endl;
		//cout << get<0>(res_2).size() << endl;
		sql_pool->releaseConnection(conn);
	}

}

#endif



using namespace std;

int main() {
#ifdef SN_TEST_DB
	sn_DB_test();
#endif
#ifdef SN_TEST_PREV
	sn_Alg_test::sn_alg_test();
	sn_Decimal_test::sn_decimal_test();
	sn_Assist_test::sn_assist_test();
	sn_Reflection_test::sn_reflection_test();
	sn_String_test::sn_string_test();
	sn_Log_test::sn_log_test();
#endif
	getchar();
	return 0;
}