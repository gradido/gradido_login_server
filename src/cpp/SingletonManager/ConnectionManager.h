#ifndef GRADIDO_LOGIN_SERVER_SINGLETON_MANAGER_CONNECTION_MANAGER_INCLUDE
#define GRADIDO_LOGIN_SERVER_SINGLETON_MANAGER_CONNECTION_MANAGER_INCLUDE

#include "../Crypto/DRHashList.h"
#include <string>

#include "Poco/Util/LayeredConfiguration.h"
#include "Poco/Data/SessionPoolContainer.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Exception.h"

#include "../model/ErrorList.h"

enum ConnectionType {
	CONNECTION_MYSQL_LOGIN_SERVER,
	CONNECTION_MYSQL_PHP_SERVER,
	CONNECTION_MAX
};

class ConnectionManager : public ErrorList
{
public: 
	~ConnectionManager();

	static ConnectionManager* getInstance();

	bool setConnectionsFromConfig(const Poco::Util::LayeredConfiguration& config, ConnectionType type);

	//!  \param connectionString example: host=localhost;port=3306;db=mydb;user=alice;password=s3cr3t;compress=true;auto-reconnect=true
	inline void setConnection(std::string connectionString, ConnectionType type) {
		if (type == CONNECTION_MYSQL_LOGIN_SERVER || CONNECTION_MYSQL_PHP_SERVER) {
			mSessionPoolNames[type] = Poco::Data::Session::uri(Poco::Data::MySQL::Connector::KEY, connectionString);
			mSessionPools.add(Poco::Data::MySQL::Connector::KEY, connectionString);
			//mConnectionData[type] = connectionString;
		}
	}

	inline Poco::Data::Session getConnection(ConnectionType type) {
		switch (type)
		{
		case CONNECTION_MYSQL_LOGIN_SERVER: 
		case CONNECTION_MYSQL_PHP_SERVER:
			return mSessionPools.getPool(mSessionPoolNames[type]).get();
		default:
			addError(new ParamError("[ConnectionManager::getConnection]", "Connection Type unknown", std::to_string(type)));
			break;
		}
		throw Poco::NotFoundException("Connection Type unknown", std::to_string(type));
		//return Poco::Data::Session(nullptr);
	}

protected:
	ConnectionManager();

private:
	std::string mSessionPoolNames[CONNECTION_MAX];
	Poco::Data::SessionPoolContainer mSessionPools;
};

#endif //GRADIDO_LOGIN_SERVER_SINGLETON_MANAGER_CONNECTION_MANAGER_INCLUDE