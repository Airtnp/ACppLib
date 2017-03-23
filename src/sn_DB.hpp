#ifndef SN_DB_H
#define SN_DB_H

#include "sn_CommonHeader.h"
#include <windows.h>
#include "../3rdParty/mysql/include/mysql.h"
#include "../3rdParty/gsl/string_span"


#pragma comment(lib, "../3rdParty/mysql/lib/libmysql.lib")

//using namespace to contain global var
namespace sn_DBImpl {

	namespace DBDefine {
		using namespace std;
		using DB_STATUS_CODE = unsigned int;
		using DB_LENGTH = my_ulonglong;
		const unsigned int DB_SUCCESS = 1;
		const unsigned int DB_FAILED = 0;
		const unsigned int DB_ONLINE = 1;
		const unsigned int DB_OFFLINE = 0;

		using DB_CONTENT = vector<vector<string>>;
		const vector<vector<string>> DB_EMPTY_CONTENT = {};

		using DB_MESSAGE = string;

		using DB_OUTPUT = tuple<DB_CONTENT, DB_STATUS_CODE, DB_MESSAGE>;
		using DB_LENGTH_OUTPUT = tuple<DB_LENGTH, DB_STATUS_CODE, DB_MESSAGE>;
	}
	namespace DBHelper {
		using namespace std;
		using std::to_string;
		string to_string(string input) {
			return input;
		}

		string to_string(char* input) {
			return static_cast<string>(input);
		}

		template<typename T>
		string& insert_accumulate_helper(string& dest, T src, string spliter_1, string spliter_2) {
			dest += spliter_1; //space
			dest += to_string(src);
			dest += spliter_2;  //.
			return dest;
		}

		template<typename T>
		//TODO: using void_t/enable_if SFINAE to satisfy the meeting of (id, 233, >)
		string& condition_accumulate_helper(string& dest, T src, string spliter_1, string order_sign, string relation_sign) {
			dest += spliter_1;
			dest += to_string(src.first);
			dest += " " + order_sign + " ";  //=,>,<
			dest += to_string(src.second);
			dest += " " + relation_sign; //and, or
			return dest;
		}

		string& remove_comma_helper(string& dest) {
			auto const pos = dest.find_last_of(",");
			dest = dest.substr(0, pos);
			return dest;
		}

		string& remove_and_helper(string& dest) {
			auto const pos = dest.find_last_of("and");
			dest = dest.substr(0, pos - 2);
			return dest;
		}

		string wstring_to_utf8(const wstring& str) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.to_bytes(str);
		}

		wstring gbk_to_wstring(const string& gbk_str) {
			string gbk_locale_name = ".936";
			std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> conv(new std::codecvt_byname<wchar_t, char, mbstate_t>(gbk_locale_name));
			return conv.from_bytes(gbk_str);
		}

		//for outputing chinese rows, we use wcout(.imbue(local("chs"))) << utf8_to_wstring(content)
		wstring utf8_to_wstring(const string& str) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.from_bytes(str);
		}

		wstring string_to_wstring(const string& str) {
			try {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
				wstring wstr = conv.from_bytes(str);
				return wstr;
			}
			catch (std::range_error&)
			{
				wstring wstr = gbk_to_wstring(str);
				return wstr;
			}
		}

		string wstring_to_string(const wstring& wstr) {
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
			string str = conv.to_bytes(wstr);
			return str;
		}

		//for chinese fields, we use hex(field_name) = wstring_to_hex(L'example')
		string wstring_to_hex(const wstring& str) {
			string u8str = wstring_to_utf8(str);
			stringstream ss;
			ss << std::hex;
			for (unsigned char c : u8str)
				ss << setw(2) << setfill('0') << static_cast<int>(c);
			string hexstr = ss.str();
			hexstr = "\'" + hexstr + "\'";
			return hexstr;
		}

		//to chinese name, use wstring_to_unhex('example');
		string wstring_to_unhex(const wstring& str) {
			string unhexstr = "unhex(" + wstring_to_hex(str) + ")";
			return unhexstr;
		}

		string string_to_hex(const string& str) {
			wstring wstr = string_to_wstring(str);
			return wstring_to_hex(wstr);
		}

		string string_to_unhex(const string& str) {
			wstring wstr = string_to_wstring(str);
			return wstring_to_unhex(wstr);
		}

		struct split_option {
			enum empty_t { empty_remain, empty_discard };
		};

		template<typename T>
		vector<T> string_split(const T& str, const T& delimiter = "\t", split_option::empty_t empty_option = split_option::empty_remain) {
			vector<T> v;
			size_t current;
			size_t next = -1;
			do {
				if (empty_option == split_option::empty_discard) {
					next = str.find_first_not_of(delimiter, next + 1);
					if (next == string::npos)
						break;
					next -= 1;
				}
				current = next + 1;
				next = str.find_first_of(delimiter, current);
				v.push_back(str.substr(current, next - current));
			} while (next != string::npos);
			return v;
		}

		vector<string> string_split_force(const string& str, const wstring& delimiter = L"\t", split_option::empty_t empty_option = split_option::empty_remain) {
			auto wd_list = string_split<wstring>(string_to_wstring(str), delimiter, empty_option);
			vector<string> d_list;
			for (auto v : wd_list)
				d_list.push_back(wstring_to_string(v));
			return d_list;
		}

	}
	using namespace std;
	using namespace gsl;
	using namespace DBHelper;
	using namespace DBDefine;
	using namespace std::placeholders;


	class SQLResultBase {

	public:
		SQLResultBase() = delete;
		SQLResultBase(MYSQL_RES* _res) : res(_res) {
			if (res) {
				num_fields = mysql_num_fields(res);
				num_rows = mysql_num_rows(res);
			}
		}
		~SQLResultBase() noexcept {
			if (res) {
				mysql_free_result(res);
			}
		}
		DB_LENGTH length() const {
			return num_rows;
		}

		DB_CONTENT fetch() {
			if (num_rows == 0)
				return DB_EMPTY_CONTENT;
			for (DB_LENGTH i = 0; i < num_rows; ++i) {
				MYSQL_ROW row = mysql_fetch_row(res);
				vector<string> content;
				for (int j = 0; j < num_fields; ++j) {
					if (row[j])
						content.push_back(row[j]);
					else
						content.push_back("NULL");
				}
				contents.push_back(content);
			}
			return contents;
		}

	private:
		MYSQL_RES * res;
		int num_fields = 0;
		DB_LENGTH num_rows = 0;
		DB_CONTENT contents = DB_EMPTY_CONTENT;
	};

	//RAII MySQL connection
	class SQLConnBase {

	public:
		SQLConnBase(): status(DB_OFFLINE) {
			mysql_init(&cont);
		}

		~SQLConnBase() noexcept  {
			mysql_close(&cont);
		}

		void close() {
			this->~SQLConnBase();
		}

		DB_OUTPUT ping() {
			int server_status = mysql_ping(&cont);
			if (server_status) {
				status = DB_OFFLINE;
				return{ DB_EMPTY_CONTENT, DB_FAILED, "Database Offline" };
			}
			status = DB_ONLINE;
			return{ DB_EMPTY_CONTENT, DB_SUCCESS, "Connection Refreshed" };
		}

		DB_OUTPUT connect(const string& _user, const string& _pswd, const string& _host, const string& _db, const unsigned int _port) {
			user = _user.c_str();
			pswd = _pswd.c_str();
			host = _host.c_str();
			db = _db.c_str();
			port = _port;
			MYSQL* connectStatus = mysql_real_connect(&cont, host, user, pswd, db, port, NULL, 0);
			if (!connectStatus) {
				status = DB_OFFLINE;
				return{ DB_EMPTY_CONTENT, DB_FAILED, "Connection Failed" };
			}
			else {
				status = DB_ONLINE;
				int res = mysql_set_character_set(&cont, "utf8");
				if (res)
					return{ DB_EMPTY_CONTENT, DB_FAILED, "Setting Character Set Failed" };
				return{ DB_EMPTY_CONTENT, DB_SUCCESS, "Success" };
			}
		}

		DB_LENGTH_OUTPUT query_length(const string& _query) {
			if (status == DB_OFFLINE)
				return{ 0, DB_FAILED, "Database No Connection" };
			int server_status = mysql_ping(&cont);
			if (server_status)
				return{ 0, DB_FAILED, "Database Offline" };
			int res = mysql_query(&cont, _query.c_str());
			if (res) {
				return{ 0, DB_FAILED, "Query Failed" };
			}
			auto detail = SQLResultBase(mysql_store_result(&cont));
			auto length = detail.length();
			return{ length, DB_SUCCESS, "Success" };
		}

		DB_OUTPUT query(const string& _query) {
			if (status == DB_OFFLINE)
				return{ DB_EMPTY_CONTENT, DB_FAILED, "Database No Connection" };
			int server_status = mysql_ping(&cont);
			if (server_status)
				return{ DB_EMPTY_CONTENT, DB_FAILED, "Database Offline" };
			int res = mysql_query(&cont, _query.c_str());
			if (res) {
				return { DB_EMPTY_CONTENT, DB_FAILED, "Query Failed" };
			}
			auto detail = SQLResultBase(mysql_store_result(&cont));
			auto contents = detail.fetch();
			return{ contents, DB_SUCCESS, "Success" };
		}
		template<typename T>
		//requires(T::first a, T::second b) {to_string(a);to_string(b);}
		DB_OUTPUT op_insert(const string& table, const vector<T>& items) {
			string query_string = "INSERT INTO ";
			query_string += table;
			query_string += " SET ";
			query_string = accumulate(items.begin(), items.end(), query_string, bind(condition_accumulate_helper<remove_reference<decltype(items)>::type::value_type>, _1, _2, " ", "=", ","));
			query_string = remove_comma_helper(query_string);
			/*
			query_string += " VALUES (";
			vector<T> items;
			for (auto cond : conds) {
				items.push_back(cond.second);
			}
			query_string = accumulate(items.begin(), items.end(), query_string, bind(insert_accumulate_helper<decltype(items)::value_type>, _1, _2, " ", ","));
			query_string = remove_comma_helper(query_string);
			query_string += " )";
			*/
			return query(query_string);
		}

		template<typename T>
		//requires(T::first a, T::second b) {to_string(a);to_string(b);}
		DB_OUTPUT op_delete(const string& table, const vector<T>& conds) {
			string query_string = "DELETE FROM ";
			query_string += table;
			query_string += " WHERE ";
			query_string = accumulate(conds.begin(), conds.end(), query_string, bind(condition_accumulate_helper<remove_reference<decltype(conds)>::type::value_type>, _1, _2, " ", "=", "and"));
			query_string = remove_and_helper(query_string);
			return query(query_string);
		}

		template<typename T, typename P>
		//requires(T::first a, T::second b) {to_string(a);to_string(b);}
		//requires(P::first a, P::second b) {to_string(a);to_string(b);}
		DB_OUTPUT op_update(const string& table, const vector<T>& conds, const vector<P>& values) {
			string query_string = "UPDATE ";
			query_string += table;
			query_string += " SET ";
			query_string = accumulate(values.begin(), values.end(), query_string, bind(condition_accumulate_helper<remove_reference<decltype(values)>::type::value_type>, _1, _2, " ", "=", "and"));
			query_string = remove_and_helper(query_string);
			query_string += " WHERE ";
			query_string = accumulate(conds.begin(), conds.end(), query_string, bind(condition_accumulate_helper<remove_reference<decltype(conds)>::type::value_type>, _1, _2, " ", "=", "and"));
			query_string = remove_and_helper(query_string);
			return query(query_string);
		}

		template<typename T>
		//requires(T::first a, T::second b) {to_string(a);to_string(b);}
		DB_OUTPUT op_select(const string& table, const vector<T>& conds) {
			string query_string = "SELECT * FROM ";
			query_string += table;
			query_string += " WHERE ";
			query_string = accumulate(conds.begin(), conds.end(), query_string, bind(condition_accumulate_helper<remove_reference<decltype(conds)>::type::value_type>, _1, _2, " ", "=", "and"));
			query_string = remove_and_helper(query_string);
			return query(query_string);
		}

		DB_OUTPUT op_select(const string& table) {
			string query_string = "SELECT * FROM ";
			query_string += table;
			return query(query_string);
		}

		DB_LENGTH_OUTPUT op_select_length(const string& table) {
			string query_string = "SELECT * FROM ";
			query_string += table;
			return query_length(query_string);
		}

	private:
		SQLConnBase(const SQLConnBase&) = delete;
		SQLConnBase& operator= (const SQLConnBase&) = delete;

		MYSQL cont;
		unsigned int status;
		czstring<> user;
		czstring<> pswd;
		czstring<> host;
		czstring<> db;
		unsigned int port;
	};

}

#endif