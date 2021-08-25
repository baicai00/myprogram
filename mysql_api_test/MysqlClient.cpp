#include "MysqlClient.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
using namespace std;

static string null_string;

MysqlResult::MysqlResult(MYSQL* mysql)
{
	if (mysql == NULL)
	{
		m_result = NULL;
		return;
	}
	m_result = mysql_store_result(mysql);
	MYSQL_FIELD* field = NULL;
	if (m_result == NULL)
		return;
	int i = 0;
	while ((field = mysql_fetch_field(m_result)))
	{
		m_field[field->name] = i++;
	}
}

MysqlResult::~MysqlResult()
{
	if (m_result)
		mysql_free_result(m_result);
}

int32_t MysqlResult::num_rows()
{
	if (m_result) {
		return mysql_num_rows(m_result);
	}
	else {
		return 0;
	}
}

bool MysqlResult::fetch_row(MysqlRow& row)
{
	row.clear();
	row.m_result = this;
	if (m_result == NULL)
		return false;
	MYSQL_ROW r = mysql_fetch_row(m_result);
	if (r == NULL)
	{
		return false;
	}
	unsigned long* l = mysql_fetch_lengths(m_result);
	for (int i = 0; i < (int)m_field.size(); ++i)
	{
		if (r[i] == NULL)
			row.m_row.push_back("");
		else
		{
			if (l == NULL)
				row.m_row.push_back(r[i]);
			else
				row.m_row.push_back(string(r[i], l[i]));   //binnary
		}
	}
	if (row.m_row.empty())
		return false;
	return true;
}

int64_t MysqlRow::get_int64(const std::string& key)
{
	return atoll(get_string(key).c_str());
}

int MysqlRow::get_int(const std::string& key)
{
	return atoi(get_string(key).c_str());
}

double MysqlRow::get_double(const std::string& key)
{
	return stod(get_string(key));
}

const std::string& MysqlRow::get_string(const std::string& key)
{
	int idx = m_result->name2idx(key);
	if (idx < 0 || idx >= (int)m_row.size())
	{
		return null_string;
	}
	return m_row[idx];
}

MysqlClient::MysqlClient()
{
	m_mysql_ctx = NULL;
}

MysqlClient::~MysqlClient()
{
	if (m_mysql_ctx)
		mysql_close(m_mysql_ctx);
}

void MysqlClient::init_mysql_ctx()
{
	mysql_close(m_mysql_ctx);	

	m_mysql_ctx = mysql_init(NULL);
	if (m_mysql_ctx == NULL)
	{
		//LOG(ERROR) << "mysql init failed";
		return;
	}

	//unsigned int timeout = 30;
	unsigned int timeout = 20;
	mysql_options(m_mysql_ctx, MYSQL_OPT_READ_TIMEOUT, &timeout);
	mysql_options(m_mysql_ctx, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

	m_mysql_ctx = mysql_real_connect(m_mysql_ctx, m_ip.c_str(), m_user.c_str(), m_password.c_str(), m_database.c_str(), m_port, NULL, CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS);

	if (m_mysql_ctx == NULL)
	{
		return;
	}

	//mysql_set_character_set(m_mysql_ctx, "utf8mb4");
	mysql_set_character_set(m_mysql_ctx, "utf8");
}

bool MysqlClient::mysql_connect(const std::string& ip_port, const std::string& user, const std::string& password, const std::string& database)
{
	size_t i = ip_port.find(':');
	if (i == string::npos)
	{
		//log_error("mysql connect str not valid");
		return false;
	}
	m_ip = ip_port.substr(0, i);
	m_port = atoi(ip_port.substr(i + 1).c_str());
	m_database = database;
	m_user = user;
	m_password = password;

	init_mysql_ctx();

	if (m_mysql_ctx == NULL)
	{
		//log_error("mysql connect failed %s", ip_port.c_str());
		return false;
	}
	//log_debug("connect mysql ok. host:%s", ip_port.c_str());
	return true;
}


MYSQL* MysqlClient::mysql_reconnect()
{
	if (m_mysql_ctx == NULL || mysql_ping(m_mysql_ctx) != 0)
	{
		init_mysql_ctx();

		if (m_mysql_ctx != NULL)
		{
			//LOG(WARNING) << "mysql reconnect OK";
		}
		else
		{
			//LOG(ERROR) << "mysql reconnect error";
		}
	}
	return m_mysql_ctx;
}

bool MysqlClient::mysql_exec(const std::string& sql)
{
	if (mysql_reconnect() == NULL)
	{
		//log_error("mysqlexec error, sql:%s", sql.c_str());
		return false;
	}
		
	if (mysql_real_query(m_mysql_ctx, sql.c_str(), sql.size()) != 0)
	{
		//LOG(ERROR) << "mysql error:" << mysql_error(m_mysql_ctx) << " sql :" << sql;
		//log_error("mysql error: exec: %s, error: %s" , sql.c_str(), mysql_error(m_mysql_ctx));
		return false;
	}
	return true;
}

uint32_t MysqlClient::mysql_last_affected_rows()
{
	if (mysql_reconnect() == NULL)
	{
		//LOG(ERROR) << __func__ << "() mysql reconnect error";
		return 0;
	}

	return mysql_affected_rows(m_mysql_ctx);
}

int MysqlClient::get_insert_id()
{
	mysql_exec("select  @@IDENTITY as id;");

	MysqlResult result(handle());
	MysqlRow row;
	if (!result.fetch_row(row))
	{
		return -1;
	}
	return row.get_int("id");
}


std::string mysql_string(const std::string& from,  MysqlClient& mysql)
{
	vector<char> temp;
	temp.resize(2 * from.size() + 1);
	if (mysql.handle() == NULL)
		return string();
	mysql_real_escape_string(mysql.handle(), &temp[0], from.c_str(), from.size());
	return &temp[0];
}


