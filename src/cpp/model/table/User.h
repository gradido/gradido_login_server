#ifndef GRADIDO_LOGIN_SERVER_MODEL_TABLE_USER_INCLUDE
#define GRADIDO_LOGIN_SERVER_MODEL_TABLE_USER_INCLUDE

#include "ModelBase.h"
#include "../../SingletonManager/MemoryManager.h"

#include "Poco/Tuple.h"
#include "Poco/DateTime.h"
//#include "Poco/Nullable.h"
//#include "Poco/Data/LOB.h"

#include <shared_mutex>

#include "UserRoles.h"

namespace model {
	namespace table {
		enum UserFields
		{
			USER_FIELDS_ID,
			USER_FIELD_EMAIL,
			USER_FIELDS_FIRST_NAME,
			USER_FIELDS_LAST_NAME,
			USER_FIELDS_PASSWORD,
			USER_FIELDS_PUBLIC_KEY,
			USER_FIELDS_PRIVATE_KEY,
			USER_FIELDS_CREATED,
			USER_FIELDS_EMAIL_CHECKED,
			USER_FIELDS_LANGUAGE
		};

		typedef Poco::Tuple<int, std::string, std::string, std::string, Poco::Nullable<Poco::Data::BLOB>, Poco::DateTime, int, int> UserTuple;

		class User : public ModelBase 
		{
		public:
			User();
			User(UserTuple tuple);
			User(const std::string& email, const std::string& first_name, const std::string& last_name, Poco::UInt64 passwordHashed = 0, std::string languageKey = "de");
			~User();

			// generic db operations
			const char* getTableName() const { return "users"; }
			std::string toString();
			std::string toHTMLString();

			// specific db operation
			size_t updatePrivkey();
			size_t updatePublickey();
			size_t updatePrivkeyAndPasswordHash();
			size_t updatePubkeyAndPrivkey();
			
			//! \brief update first_name, last_name, disabled and language
			size_t updateFieldsFromCommunityServer();

			// default getter unlocked
			inline const std::string getEmail() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mEmail; }
			inline const std::string getFirstName() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mFirstName; }
			inline const std::string getLastName() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mLastName; }
			inline std::string getNameWithEmailHtml() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mFirstName + "&nbsp;" + mLastName + "&nbsp;&lt;" + mEmail + "&gt;"; }
			inline std::string getNameWithEmail() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mFirstName + " " + mLastName + "<" + mEmail + ">"; }
			inline const Poco::UInt64 getPasswordHashed() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mPasswordHashed; }
			inline RoleType getRole() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); if (mRole.isNull()) return ROLE_NONE; return static_cast<RoleType>(mRole.value()); }
			inline const unsigned char* getPublicKey() const { if (mPublicKey.isNull()) return nullptr; return mPublicKey.value().content().data(); }
			std::string getPublicKeyHex() const;

			inline bool hasPrivateKeyEncrypted() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return !mPrivateKey.isNull(); }
			inline const std::vector<unsigned char>& getPrivateKeyEncrypted() const { return mPrivateKey.value().content(); }
			inline bool isEmailChecked() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mEmailChecked; }
			inline const std::string getLanguageKey() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mLanguageKey; }
			inline bool isDisabled() const { std::shared_lock<std::shared_mutex> _lock(mSharedMutex); return mDisabled; }

			// default setter unlocked
			inline void setEmail(const std::string& email) { std::unique_lock<std::shared_mutex> _lock(mSharedMutex); mEmail = email; }
			inline void setFirstName(const std::string& first_name) { std::unique_lock<std::shared_mutex> _lock(mSharedMutex); mFirstName = first_name; }
			inline void setLastName(const std::string& last_name) { std::unique_lock<std::shared_mutex> _lock(mSharedMutex); mLastName = last_name; }
			inline void setPasswordHashed(const Poco::UInt64& passwordHashed) { std::unique_lock<std::shared_mutex> _lock(mSharedMutex); mPasswordHashed = passwordHashed; }
			void setPublicKey(const unsigned char* publicKey);
			//! \brief set encrypted private key
			//! \param privateKey copy data, didn't move memory bin
			void setPrivateKey(const MemoryBin* privateKey);
			inline void setEmailChecked(bool emailChecked) { std::unique_lock<std::shared_mutex> _lock(mSharedMutex); mEmailChecked = emailChecked; }
			inline void setLanguageKey(const std::string& languageKey) { std::unique_lock<std::shared_mutex> _lock(mSharedMutex); mLanguageKey = languageKey; }
			inline void setDisabled(bool disabled) { std::unique_lock<std::shared_mutex> _lock(mSharedMutex); mDisabled = disabled; }

			Poco::JSON::Object getJson();			

		protected:

			Poco::Data::Statement _loadFromDB(Poco::Data::Session session, const std::string& fieldName);
			Poco::Data::Statement _loadIdFromDB(Poco::Data::Session session);
			Poco::Data::Statement _loadMultipleFromDB(Poco::Data::Session session, const std::string& fieldName);
			Poco::Data::Statement _loadMultipleFromDB(Poco::Data::Session session, const std::vector<std::string> fieldNames, MysqlConditionType conditionType = MYSQL_CONDITION_AND);
			// insert only with email, first_name, last_name, password if exist and language
			Poco::Data::Statement _insertIntoDB(Poco::Data::Session session);
			
			std::string mEmail;
			std::string mFirstName;
			std::string mLastName;

			Poco::UInt64 mPasswordHashed;

			Poco::Nullable<Poco::Data::BLOB> mPublicKey;
			Poco::Nullable<Poco::Data::BLOB> mPrivateKey;
			// created: Mysql DateTime
			Poco::DateTime mCreated;

			bool mEmailChecked;
			std::string mLanguageKey;
			//! if account should delete but cannot because public keys for transaction needed, until gradido node is running
			//! if set to true, prevent login
			bool mDisabled;

			// from neighbor tables
			Poco::Nullable<int> mRole;

			mutable std::shared_mutex mSharedMutex;
		};
	}
}

#endif //GRADIDO_LOGIN_SERVER_MODEL_TABLE_USER_INCLUDE