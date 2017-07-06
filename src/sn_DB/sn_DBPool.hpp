#ifndef SN_DB_POOL_H
#define SN_DB_POOL_H
#include "sn_CommonHeader.h"
#include "sn_DB.hpp"


namespace sn_DBPImpl {
	using namespace std;
	using namespace gsl;

	namespace DBPConfig {
		using namespace std;
		string user = "root";
		string pswd = "";
		string host = "localhost";
		string db = "example";
		unsigned int port = 3306;
		unsigned int poolSize = 10;
		int setConfig(const string& _user = user, const string& _pswd = pswd, const string& _host = host, const string& _db = db, unsigned int _port = port, unsigned int _poolSize = poolSize) {
			user = _user;
			pswd = _pswd;
			host = _host;
			db = _db;
			port = _port;
			poolSize = _poolSize;
			return 0;
		}
	}

	class SQLConnPoolBase {
	public:
		//Singleton
		static shared_ptr<SQLConnPoolBase> getInstance() {
			call_once(createFlag, [&] {connPool.reset(new SQLConnPoolBase); });
			return connPool;
		}
		shared_ptr<sn_DBImpl::SQLConnBase> getConnection() {
			shared_ptr<sn_DBImpl::SQLConnBase> conn;
			lock_guard<mutex> guard(conn_mutex);
			if (connList.size() > 0) {
				conn = connList.front();
				connList.pop_front();
				occupyList.push_back(conn);
				if (get<1>(conn->ping()) == sn_DBImpl::DB_FAILED) {
					conn->close();
					conn = createConnection();
					occupyList.push_back(conn);
				}
				if (!conn) {
					cerr << "Create Connection Failed" << endl;
					--currentSize;
					occupyList.pop_back();
				}
				return conn;
			}
			else {
				if (currentSize < poolSize) {
					conn = createConnection();
					if (conn) {
						++currentSize;
						occupyList.push_back(conn);
						return conn;
					}
					else {
						cerr << "Create Connection Failed" << endl;
						return nullptr;
					}
				}
				else {
					cerr << "Connection Pool Full" << endl;
					return nullptr;
				}
			}

		}

		void releaseConnection(shared_ptr<sn_DBImpl::SQLConnBase> conn) {
			if (conn) {
				lock_guard<mutex> guard(conn_mutex);
				occupyList.remove(conn);
				connList.push_back(conn);
			}
		}

		class GC_SQLConnPoolBase {
		public:
			~GC_SQLConnPoolBase() {
				if (connPool) {
					lock_guard<mutex> guard(connPool->conn_mutex);
					for (auto& conn : connPool->connList) {
						if (conn)
							conn->close();
					}
					for (auto& conn : connPool->occupyList) {
						if (conn)
							conn->close();
					}
					connPool->currentSize = 0;
					connPool->connList.clear();
					connPool->occupyList.clear();
				}
			}
		};
		static GC_SQLConnPoolBase gc_connPool;
	private:
		SQLConnPoolBase() :
			user(DBPConfig::user), pswd(DBPConfig::pswd), port(DBPConfig::port),
			host(DBPConfig::host), db(DBPConfig::db), poolSize(DBPConfig::poolSize) {
			initConnection(poolSize / 2);
		}
		SQLConnPoolBase(const SQLConnPoolBase&) = default;
		SQLConnPoolBase& operator=(const SQLConnPoolBase&) = default;
		
		void initConnection(int initSize) {
			shared_ptr<sn_DBImpl::SQLConnBase> conn;
			lock_guard<mutex> guard(conn_mutex);
			for (int i = 0; i < initSize; ++i) {
				conn = createConnection();
				if (conn) {
					connList.push_back(move(conn));
					++currentSize;
				}
			}
		}

		shared_ptr<sn_DBImpl::SQLConnBase> createConnection() {
			auto conn = make_shared<sn_DBImpl::SQLConnBase>();
			auto res = conn->connect(user, pswd, host, db, port);
			if (get<1>(res) == sn_DBImpl::DB_FAILED) {
				cerr << get<2>(res) << endl;
				return nullptr;
			}
			return conn;
		}

		const string user;
		const string pswd;
		const string host;
		const string db;
		const size_t poolSize;
		unsigned int port;
		size_t currentSize;
		
		static shared_ptr<SQLConnPoolBase> connPool;
		static once_flag createFlag;
		list<shared_ptr<sn_DBImpl::SQLConnBase>> connList;
		list<shared_ptr<sn_DBImpl::SQLConnBase>> occupyList;
		mutex conn_mutex;
		
	};


}


#endif