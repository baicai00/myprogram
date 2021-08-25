#ifndef __MYSQLCLIENT_H_
#define __MYSQLCLIENT_H_

#include <string>
#include <map>
#include <stdint.h>
#include <vector>
#include "mysql.h"


class MysqlClient
{
public:
	MysqlClient();
	~MysqlClient();
	bool mysql_connect(const std::string& ip_port, const std::string& user, const std::string& password, const std::string& database);
	MYSQL* mysql_reconnect();
	bool mysql_exec(const std::string& sql);
	uint32_t mysql_last_affected_rows();
	MYSQL* handle()
	{
		return m_mysql_ctx;
	}

	int get_insert_id();

	

private: 
	void init_mysql_ctx();
	std::string m_ip;
	int m_port;
	std::string m_database;
	std::string m_user;
	std::string m_password;

	MYSQL* m_mysql_ctx;
};

class MysqlResult;
class MysqlRow  //MysqlResult::fetch_row返回true才能用
{
public:
	friend class MysqlResult;
	MysqlRow()
	{
		m_result = NULL;
	}
	int64_t get_int64(const std::string& key);
	const std::string& get_string(const std::string& key);
	int get_int(const std::string& key);

	double get_double(const std::string& key);

	void clear()
	{
		m_row.clear();
	}
private:
	std::vector<std::string> m_row;
	MysqlResult* m_result;
};

class MysqlResult
{
public:
	MysqlResult(MYSQL* mysql);
	bool fetch_row(MysqlRow& row);
	int name2idx(const std::string& name)
	{
		auto it = m_field.find(name);
		if (it == m_field.end())
			return -1;
		return it->second;
	}
	~MysqlResult();

	int32_t num_rows();

private:
	MYSQL_RES* m_result;
	std::map<std::string, int> m_field;
};

std::string mysql_string(const std::string& from, MysqlClient& mysql);

#endif
