#ifndef GRADIDO_LOGIN_SERVER_CONTROLLER_EMAIL_VERIFICATION_CODE_INCLUDE
#define GRADIDO_LOGIN_SERVER_CONTROLLER_EMAIL_VERIFICATION_CODE_INCLUDE

#include "../model/table/EmailOptIn.h"

#include "TableControllerBase.h"

namespace controller {
	class EmailVerificationCode : public TableControllerBase
	{
	public:
		
		~EmailVerificationCode();

		static Poco::AutoPtr<EmailVerificationCode> create(int user_id, model::table::EmailOptInType type = model::table::EMAIL_OPT_IN_REGISTER);
		static Poco::AutoPtr<EmailVerificationCode> create(model::table::EmailOptInType type = model::table::EMAIL_OPT_IN_REGISTER);

		static Poco::AutoPtr<EmailVerificationCode> load(const Poco::UInt64& code);
		static std::vector<Poco::AutoPtr<EmailVerificationCode>>   load(int user_id);
		static Poco::AutoPtr<EmailVerificationCode> load(int user_id, model::table::EmailOptInType type); 

		inline bool deleteFromDB() { return mDBModel->deleteFromDB(); }

		inline Poco::AutoPtr<model::table::EmailOptIn> getModel() { return _getModel<model::table::EmailOptIn>(); }

		std::string getLink();
		inline Poco::Timespan getAge() { return Poco::DateTime() - getModel()->getCreated(); }

	protected:
		EmailVerificationCode(model::table::EmailOptIn* dbModel);
		static Poco::UInt64 createEmailVerificationCode();

		//table::EmailOptIn* mDBModel;
	};
}

#endif //GRADIDO_LOGIN_SERVER_CONTROLLER_EMAIL_VERIFICATION_CODE_INCLUDE