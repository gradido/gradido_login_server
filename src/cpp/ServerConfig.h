#ifndef __GRADIDO_LOGIN_SERVER_SERVER_CONFIG__
#define __GRADIDO_LOGIN_SERVER_SERVER_CONFIG__

#include "Crypto/mnemonic.h"
#include "Crypto/Obfus_array.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "Poco/Net/Context.h"
#include "Poco/Types.h"

#include "tasks/CPUSheduler.h"

namespace ServerConfig {

	enum Mnemonic_Types {
		MNEMONIC_GRADIDO_BOOK_GERMAN_RANDOM_ORDER,
		MNEMONIC_BIP0039_SORTED_ORDER,
		MNEMONIC_MAX
	};

	struct EmailAccount {
		std::string sender;
		std::string username;
		std::string password;
		std::string url;
		int port;
	};

	extern Mnemonic g_Mnemonic_WordLists[MNEMONIC_MAX];

	extern ObfusArray* g_ServerCryptoKey;
	extern ObfusArray* g_ServerKeySeed;

	//extern unsigned char g_ServerAdminPublic[];
	extern UniLib::controller::CPUSheduler* g_CPUScheduler;
	extern UniLib::controller::CPUSheduler* g_CryptoCPUScheduler;
	extern Poco::Net::Context::Ptr g_SSL_CLient_Context;
	extern EmailAccount g_EmailAccount;
	extern int g_SessionTimeout;
	extern std::string g_serverPath;
	extern std::string g_php_serverPath;


	bool loadMnemonicWordLists();
	bool initServerCrypto(const Poco::Util::LayeredConfiguration& cfg);
	bool initEMailAccount(const Poco::Util::LayeredConfiguration& cfg);
	bool initSSLClientContext();

	void writeToFile(std::istream& datas, std::string fileName);

	void unload();
};

#endif //__GRADIDO_LOGIN_SERVER_SERVER_CONFIG__