/*!
*
* \author: einhornimmond
*
* \date: 02.01.20
*
* \brief: store email for 
*/

#ifndef GRADIDO_LOGIN_SERVER_MODEL_EMAIL_INCLUDE
#define GRADIDO_LOGIN_SERVER_MODEL_EMAIL_INCLUDE

#include "Poco/Net/MailMessage.h"

#include "../../controller/EmailVerificationCode.h"
#include "../../controller/User.h"

#include "../../SingletonManager/LanguageManager.h"

#include "../../lib/ErrorList.h"

namespace model {
	using namespace Poco;

	enum EmailType 
	{
		EMAIL_DEFAULT,
		EMAIL_ERROR,
		EMAIL_USER_VERIFICATION_CODE,
		EMAIL_USER_VERIFICATION_CODE_RESEND,
		EMAIL_USER_VERIFICATION_CODE_RESEND_AFTER_LONG_TIME,
		EMAIL_ADMIN_USER_VERIFICATION_CODE,
		EMAIL_ADMIN_USER_VERIFICATION_CODE_RESEND,
		EMAIL_USER_RESET_PASSWORD,
		EMAIL_ADMIN_RESET_PASSWORD_REQUEST_WITHOUT_MEMORIZED_PASSPHRASE,
		EMAIL_NOTIFICATION_TRANSACTION_CREATION,
		EMAIL_NOTIFICATION_TRANSACTION_TRANSFER,
		EMAIL_USER_REGISTER_OLD_ELOPAGE,
		EMAIL_MAX
	};

	class Email: public ErrorList
	{
	public:
		Email(AutoPtr<controller::EmailVerificationCode> emailVerification, AutoPtr<controller::User> user, EmailType type);
		Email(AutoPtr<controller::User> user, EmailType type);
		//! \param errors copy errors into own memory
		Email(const std::string& errorHtml, EmailType type);
		~Email();

		static EmailType convertTypeFromInt(int type);
		inline EmailType getType() { return mType; }
		static const char* emailTypeString(EmailType type);
		inline controller::User* getUser() { if (!mUser.isNull()) return mUser.get(); return nullptr; }

		virtual bool draft(Net::MailMessage* mailMessage, LanguageCatalog* langCatalog);
		inline void addContent(Poco::Net::StringPartSource* str_content) { mAdditionalStringPartSrcs.push(str_content); }


	protected:
		std::string replaceUserNamesAndLink(const char* src, const std::string& first_name, const std::string& last_name, const std::string& link);
		std::string replaceEmail(const char* src, const std::string& email);
		std::string replaceAmount(const char* src, Poco::Int64 gradido_cent);
		std::string replaceDuration(std::string src, Poco::Timespan duration, LanguageCatalog* lang);

		AutoPtr<controller::EmailVerificationCode> mEmailVerificationCode;
		AutoPtr<controller::User> mUser;
		std::string mErrorHtml;
		EmailType mType;

		std::queue<Poco::Net::StringPartSource*> mAdditionalStringPartSrcs;
	};
}

#endif //GRADIDO_LOGIN_SERVER_MODEL_EMAIL_INCLUDE