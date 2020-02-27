#include "Gradido_LoginServer.h"
#include <sodium.h>

#include "proto/gradido/TransactionBody.pb.h"

#include "model/User.h"
#include "model/Session.h"
#include "lib/Profiler.h"
#include "ServerConfig.h"
#include "ImportantTests.h"

#include "model/table/User.h"
#include "model/table/EmailOptIn.h"

#ifndef _TEST_BUILD


int main(int argc, char** argv)
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if (sodium_init() < 0) {
		/* panic! the library couldn't be initialized, it is not safe to use */
		printf("error initializing sodium, early exit\n");
		return -1;
	}



	ServerConfig::g_versionString = "0.20.KW08.04";
	printf("User size: %d Bytes, Session size: %d Bytes\n", sizeof(User), sizeof(Session));
	printf("model sizes: User: %d Bytes, EmailOptIn: %d Bytes\n", sizeof(model::table::User), sizeof(model::table::EmailOptIn));
	if (!ImportantTests::passphraseGenerationAndTransformation()) {
		printf("test passphrase generation and transformation failed\n");
		return -2;
	}
	
	Gradido_LoginServer app;
	return app.run(argc, argv);
}
#endif