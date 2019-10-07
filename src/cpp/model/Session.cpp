#include "Session.h"
#include "Profiler.h"
#include "../ServerConfig.h"

#include "Poco/RegularExpression.h"
#include "Poco/Net/StringPartSource.h"

#include "../SingletonManager/SessionManager.h"
#include "../SingletonManager/ConnectionManager.h"
#include "../SingletonManager/ErrorManager.h"
#include "../tasks/PrepareEmailTask.h"
#include "../tasks/SendEmailTask.h"


#include "sodium.h"

using namespace Poco::Data::Keywords;

int WriteEmailVerification::run()
{	
	Profiler timeUsed;
	Poco::UInt64 verificationCode = mSession->getEmailVerificationCode();
	//printf("[WriteEmailVerification::run] E-Mail Verification Code: %llu\n", verificationCode);
	auto dbSession = ConnectionManager::getInstance()->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
	//int user_id = mUser->getDBId();
	Poco::Data::Statement insert(dbSession);
	insert << "INSERT INTO email_opt_in (user_id, verification_code) VALUES(?,?);",
		bind(mUser->getDBId()), use(verificationCode);
	if (1 != insert.execute()) {
		mSession->addError(new Error("WriteEmailVerification", "error inserting email verification code"));
		return -1;
	}
	printf("[WriteEmailVerification] timeUsed: %s\n", timeUsed.string().data());
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------

int WritePassphraseIntoDB::run()
{
	Profiler timeUsed;

	// TODO: encrypt passphrase, need server admin crypto box pubkey
	//int crypto_box_seal(unsigned char *c, const unsigned char *m,
		//unsigned long long mlen, const unsigned char *pk);
	size_t mlen = mPassphrase.size();
	size_t crypto_size = crypto_box_SEALBYTES + mlen;

	auto em = ErrorManager::getInstance();

	auto dbSession = ConnectionManager::getInstance()->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
	Poco::Data::Statement insert(dbSession);
	insert << "INSERT INTO user_backups (user_id, passphrase) VALUES(?,?)",
		use(mUserId), use(mPassphrase);
	try {
		if (insert.execute() != 1) {
			em->addError(new ParamError("WritePassphraseIntoDB::run", "inserting passphrase for user failed", std::to_string(mUserId)));
			em->sendErrorsAsEmail();
		}
	}
	catch (Poco::Exception& ex) {
		em->addError(new ParamError("WritePassphraseIntoDB::run", "insert passphrase mysql error", ex.displayText().data()));
		em->sendErrorsAsEmail();
	}

	printf("[WritePassphraseIntoDB] timeUsed: %s\n", timeUsed.string().data());
	return 0;
}


// --------------------------------------------------------------------------------------------------------------

Session::Session(int handle)
	: mHandleId(handle), mSessionUser(nullptr), mEmailVerificationCode(0), mState(SESSION_STATE_EMPTY), mActive(false)
{

}

Session::~Session()
{

	reset();
}


void Session::reset()
{
	if (mSessionUser) {
		delete mSessionUser;
		mSessionUser = nullptr;
	}
	updateTimeout();
	updateState(SESSION_STATE_EMPTY);
	mPassphrase = "";
	mClientLoginIP = Poco::Net::IPAddress();
	mEmailVerificationCode = 0;
}

void Session::updateTimeout()
{
	mLastActivity = Poco::DateTime();
}

bool Session::createUser(const std::string& name, const std::string& email, const std::string& password)
{
	Profiler usedTime;
	auto sm = SessionManager::getInstance();
	if (!sm->isValid(name, VALIDATE_NAME)) {
		addError(new Error("Vorname", "Bitte gebe einen Namen an. Mindestens 3 Zeichen, keine Sonderzeichen oder Zahlen."));
		return false;
	}
	if (!sm->isValid(email, VALIDATE_EMAIL)) {
		addError(new Error("E-Mail", "Bitte gebe eine g&uuml;ltige E-Mail Adresse an."));
		return false;
	}
	if (!sm->isValid(password, VALIDATE_PASSWORD)) {
		addError(new Error("Passwort", "Bitte gebe ein g&uuml;ltiges Password ein mit mindestens 8 Zeichen, Gro&szlig;- und Kleinbuchstaben, mindestens einer Zahl und einem Sonderzeichen ein!"));

		// @$!%*?&+-
		if (password.size() < 8) {
			addError(new Error("Passwort", "Dein Passwort ist zu kurz!"));
		}
		else if (!sm->isValid(password, VALIDATE_HAS_LOWERCASE_LETTER)) {
			addError(new Error("Passwort", "Dein Passwort enth&auml;lt keine Kleinbuchstaben!"));
		}
		else if (!sm->isValid(password, VALIDATE_HAS_UPPERCASE_LETTER)) {
			addError(new Error("Passwort", "Dein Passwort enth&auml;lt keine Gro&szlig;buchstaben!"));
		}
		else if (!sm->isValid(password, VALIDATE_HAS_NUMBER)) {
			addError(new Error("Passwort", "Dein Passwort enth&auml;lt keine Zahlen!"));
		}
		else if (!sm->isValid(password, VALIDATE_HAS_SPECIAL_CHARACTER)) {
			addError(new Error("Passwort", "Dein Passwort enth&auml;lt keine Sonderzeichen (@$!%*?&+-)!"));
		}
		
		return false;
	}
	/*if (passphrase.size() > 0 && !sm->isValid(passphrase, VALIDATE_PASSPHRASE)) {
		addError(new Error("Merkspruch", "Der Merkspruch ist nicht g&uuml;ltig, er besteht aus 24 W&ouml;rtern, mit Komma getrennt."));
		return false;
	}
	if (passphrase.size() == 0) {
		//mPassphrase = User::generateNewPassphrase(&ServerConfig::g_Mnemonic_WordLists[ServerConfig::MNEMONIC_BIP0039_SORTED_ORDER]);
		mPassphrase = User::generateNewPassphrase(&ServerConfig::g_Mnemonic_WordLists[ServerConfig::MNEMONIC_GRADIDO_BOOK_GERMAN_RANDOM_ORDER]);
	}
	else {
		//mPassphrase = passphrase;
	}*/

	// check if user with that email already exist
	auto dbConnection = ConnectionManager::getInstance()->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
	Poco::Data::Statement select(dbConnection);
	select << "SELECT email from users where email = ?;", useRef(email);
	try {
		if (select.execute() > 0) {
			addError(new Error("E-Mail", "F&uuml;r diese E-Mail Adresse gibt es bereits einen Account"));
			return false;
		}
	}
	catch (Poco::Exception& exc) {
		printf("mysql exception: %s\n", exc.displayText().data());
	}

	mSessionUser = new User(email.data(), name.data());
	updateTimeout();

	// Prepare E-Mail
	UniLib::controller::TaskPtr prepareEmail(new PrepareEmailTask(ServerConfig::g_CPUScheduler));
	prepareEmail->scheduleTask(prepareEmail);

	// create user crypto key
	UniLib::controller::TaskPtr cryptoKeyTask(new UserCreateCryptoKey(mSessionUser, password, ServerConfig::g_CPUScheduler));
	cryptoKeyTask->setFinishCommand(new SessionStateUpdateCommand(SESSION_STATE_CRYPTO_KEY_GENERATED, this));
	cryptoKeyTask->scheduleTask(cryptoKeyTask);

	// depends on crypto key, write user record into db
	UniLib::controller::TaskPtr writeUserIntoDB(new UserWriteIntoDB(mSessionUser, ServerConfig::g_CPUScheduler, 1));
	writeUserIntoDB->setParentTaskPtrInArray(cryptoKeyTask, 0);
	writeUserIntoDB->setFinishCommand(new SessionStateUpdateCommand(SESSION_STATE_USER_WRITTEN, this));
	writeUserIntoDB->scheduleTask(writeUserIntoDB);

	createEmailVerificationCode();

	UniLib::controller::TaskPtr writeEmailVerification(new WriteEmailVerification(mSessionUser, this, ServerConfig::g_CPUScheduler, 1));
	writeEmailVerification->setParentTaskPtrInArray(writeUserIntoDB, 0);
	writeEmailVerification->setFinishCommand(new SessionStateUpdateCommand(SESSION_STATE_EMAIL_VERIFICATION_WRITTEN, this));
	writeEmailVerification->scheduleTask(writeEmailVerification);

	// depends on writeUser because need user_id, write email verification into db
	auto message = new Poco::Net::MailMessage;

	message->addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, email));
	message->setSubject("Gradido: E-Mail Verification");
	std::stringstream ss;
	ss << "Hallo " << name << "," << std::endl << std::endl;
	ss << "Du oder jemand anderes hat sich soeben mit dieser E-Mail Adresse bei Gradido registriert. " << std::endl;
	ss << "Wenn du es warst, klicke bitte auf den Link: https://gradido2.dario-rekowski.de/account/checkEmail/" << mEmailVerificationCode << std::endl;
	ss << "oder kopiere den Code: " << mEmailVerificationCode << " selbst dort hinein." << std::endl << std::endl;
	ss << "Mit freundlichen Gr��e" << std::endl;
	ss << "Dario, Gradido Server Admin" << std::endl;
	

	message->addContent(new Poco::Net::StringPartSource(ss.str()));

	UniLib::controller::TaskPtr sendEmail(new SendEmailTask(message, ServerConfig::g_CPUScheduler, 1));
	sendEmail->setParentTaskPtrInArray(prepareEmail, 0);
	sendEmail->setParentTaskPtrInArray(writeEmailVerification, 1);
	sendEmail->setFinishCommand(new SessionStateUpdateCommand(SESSION_STATE_EMAIL_VERIFICATION_SEND, this));
	sendEmail->scheduleTask(sendEmail);


	// write user into db
	// generate and write email verification into db
	// send email
	
	printf("[Session::createUser] time: %s\n", usedTime.string().data());

	return true;
}

bool Session::updateEmailVerification(Poco::UInt64 emailVerificationCode)
{

	Profiler usedTime;
	const static char* funcName = "Session::updateEmailVerification";
	auto em = ErrorManager::getInstance();
	if(mEmailVerificationCode == emailVerificationCode) {
		if (mSessionUser && mSessionUser->getDBId() == 0) {
			//addError(new Error("E-Mail Verification", "Benutzer wurde nicht richtig gespeichert, bitte wende dich an den Server-Admin"));
			em->addError(new Error(funcName, "user exist with 0 as id"));
			em->sendErrorsAsEmail();
			//return false;
		}
		
		// load correct user from db
		auto dbConnection = ConnectionManager::getInstance()->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
		Poco::Data::Statement update(dbConnection);
		update << "UPDATE users SET email_checked=1 where id = (SELECT user_id FROM email_opt_in where verification_code=?)", use(emailVerificationCode);
		auto updated_rows = update.execute();
		if (updated_rows == 1) {
			Poco::Data::Statement delete_row(dbConnection);
			delete_row << "DELETE FROM email_opt_in where verification_code = ?", use(emailVerificationCode);
			if (delete_row.execute() != 1) {
				em->addError(new Error(funcName, "delete from email_opt_in entry didn't work as expected, please check db"));
				em->sendErrorsAsEmail();
			}
			if (mSessionUser) {
				mSessionUser->setEmailChecked();
			}
			updateState(SESSION_STATE_EMAIL_VERIFICATION_CODE_CHECKED);
			printf("[%s] time: %s\n", funcName, usedTime.string().data());
			
			return true;
		}
		else {
			em->addError(new ParamError(funcName, "update user work not like expected, updated row count", updated_rows));
			em->sendErrorsAsEmail();
		}
		if (!updated_rows) {
			addError(new Error("E-Mail Verification", "Der Code stimmt nicht, bitte &uuml;berpr&uuml;fe ihn nochmal oder registriere dich erneut oder wende dich an den Server-Admin"));
			printf("[%s] time: %s\n", funcName, usedTime.string().data());
			return false;
		}
		
	}
	else {
		addError(new Error("E-Mail Verification", "Falscher Code f&uuml;r aktiven Login"));
		printf("[%s] time: %s\n", funcName, usedTime.string().data());
		return false;
	}
	printf("[%s] time: %s\n", funcName, usedTime.string().data());
	return false;
}

bool Session::isPwdValid(const std::string& pwd)
{
	if (mSessionUser) {
		return mSessionUser->validatePwd(pwd);
	}
	return false;
}

bool Session::loadUser(const std::string& email, const std::string& password)
{
	Profiler usedTime;
	if (mSessionUser) delete mSessionUser;
	mSessionUser = new User(email.data());
	if (!mSessionUser->validatePwd(password)) {
		addError(new Error("Login", "E-Mail oder Passwort nicht korrekt, bitte versuche es erneut"));
		return false;
	}
	detectSessionState();

	return true;
}

/*
SESSION_STATE_CRYPTO_KEY_GENERATED,
SESSION_STATE_USER_WRITTEN,
SESSION_STATE_EMAIL_VERIFICATION_WRITTEN,
SESSION_STATE_EMAIL_VERIFICATION_SEND,
SESSION_STATE_EMAIL_VERIFICATION_CODE_CHECKED,
SESSION_STATE_PASSPHRASE_GENERATED,
SESSION_STATE_PASSPHRASE_SHOWN,
SESSION_STATE_PASSPHRASE_WRITTEN,
SESSION_STATE_KEY_PAIR_GENERATED,
SESSION_STATE_KEY_PAIR_WRITTEN,
SESSION_STATE_COUNT
*/
void Session::detectSessionState()
{
	if (!mSessionUser || !mSessionUser->hasCryptoKey()) {
		return;
	}
	if (mSessionUser->getDBId() == 0) {
		updateState(SESSION_STATE_CRYPTO_KEY_GENERATED);
		return;
	}
	if (!mSessionUser->isEmailChecked()) {
		if (mEmailVerificationCode == 0)
			updateState(SESSION_STATE_USER_WRITTEN);
		else
			updateState(SESSION_STATE_EMAIL_VERIFICATION_WRITTEN);
		return;
	}

	if (mSessionUser->getPublicKeyHex() == "") {
		
		auto dbConnection = ConnectionManager::getInstance()->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
		Poco::Data::Statement select(dbConnection);
		Poco::Nullable<Poco::Data::BLOB> passphrase;
		auto user_id = mSessionUser->getDBId();
		select << "SELECT passphrase from user_backups where user_id = ?;", 
			into(passphrase), use(user_id);
		try {
			if (select.execute() == 1 && !passphrase.isNull()) {
				updateState(SESSION_STATE_PASSPHRASE_WRITTEN);
				return;
			}
		}
		catch (Poco::Exception& exc) {
			printf("mysql exception: %s\n", exc.displayText().data());
		}
		if (mPassphrase != "") {
			updateState(SESSION_STATE_PASSPHRASE_GENERATED);
			return;
		}
		updateState(SESSION_STATE_EMAIL_VERIFICATION_CODE_CHECKED);
		return;
	}

	updateState(SESSION_STATE_KEY_PAIR_WRITTEN);

}

Poco::Net::HTTPCookie Session::getLoginCookie()
{
	auto keks = Poco::Net::HTTPCookie("GRADIDO_LOGIN", std::to_string(mHandleId));
	// prevent reading or changing cookie with js
	keks.setHttpOnly();
	// send cookie only via https
	keks.setSecure(true);
	
	return keks;
}

bool Session::loadFromEmailVerificationCode(Poco::UInt64 emailVerificationCode)
{
	Profiler usedTime;
	const static char* funcName = "Session::loadFromEmailVerificationCode";
	auto em = ErrorManager::getInstance();
	auto dbConnection = ConnectionManager::getInstance()->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
	
	/*Poco::Data::Statement select(dbConnection);
	int user_id = 0;
	select << "SELECT user_id FROM email_opt_in WHERE verification_code=?", into(user_id), use(emailVerificationCode);
	try {
		if (select.execute() == 0) {
			addError(new Error("E-Mail Verification", "Der Code konnte nicht in der Datenbank gefunden werden."));
			return false;
		}
	}
	catch (Poco::Exception& ex) {
		em->addError(new ParamError(funcName, "error selecting verification code entry", ex.displayText().data()));
		em->sendErrorsAsEmail();
		return false;
	}*/
	Poco::Data::Statement select(dbConnection);
	std::string email, name;
	select.reset(dbConnection);
	select << "SELECT email, name FROM users where id = (SELECT user_id FROM email_opt_in WHERE verification_code=?)",
		 into(email), into(name), use(emailVerificationCode);
	try {
		size_t rowCount = select.execute();
		if (rowCount != 1) {
			em->addError(new ParamError(funcName, "select user by email verification code work not like expected, selected row count", rowCount));
			em->sendErrorsAsEmail();
		}
		if (rowCount < 0) {
			addError(new Error("E-Mail Verification", "Konnte keinen passenden Account finden."));
			return false;
		}

		mSessionUser = new User(email.data(), name.data());
		mSessionUser->loadEntryDBId(ConnectionManager::getInstance()->getConnection(CONNECTION_MYSQL_LOGIN_SERVER));
		mEmailVerificationCode = emailVerificationCode;
		updateState(SESSION_STATE_EMAIL_VERIFICATION_WRITTEN);
		printf("[Session::loadFromEmailVerificationCode] time: %s\n", usedTime.string().data());
		return true;
	}
	catch (const Poco::Exception& ex) {
		em->addError(new ParamError(funcName, "error selecting user from verification code", ex.displayText().data()));
		em->sendErrorsAsEmail();
	}

	return false;
}

void Session::updateState(SessionStates newState)
{
	lock();
	updateTimeout();
	printf("[%s] newState: %s\n", __FUNCTION__, translateSessionStateToString(newState));
	if (newState > mState) {
		mState = newState;
	}

	unlock();
}

const char* Session::getSessionStateString()
{
	SessionStates state;
	lock();
	state = mState;
	unlock();
	return translateSessionStateToString(state);
}


const char* Session::translateSessionStateToString(SessionStates state)
{
	switch (state) {
	case SESSION_STATE_EMPTY: return "uninitalized";
	case SESSION_STATE_CRYPTO_KEY_GENERATED: return "crpyto key generated";
	case SESSION_STATE_USER_WRITTEN: return "User saved";
	case SESSION_STATE_EMAIL_VERIFICATION_WRITTEN: return "E-Mail verification code saved";
	case SESSION_STATE_EMAIL_VERIFICATION_SEND: return "Verification E-Mail sended";
	case SESSION_STATE_EMAIL_VERIFICATION_CODE_CHECKED: return "Verification Code checked";
	case SESSION_STATE_PASSPHRASE_GENERATED: return "Passphrase generated";
	case SESSION_STATE_PASSPHRASE_SHOWN: return "Passphrase shown"; 
	case SESSION_STATE_PASSPHRASE_WRITTEN: return "Passphrase written";
	case SESSION_STATE_KEY_PAIR_GENERATED: return "Gradido Address created";
	case SESSION_STATE_KEY_PAIR_WRITTEN: return "Gradido Address saved";
	default: return "unknown";
	}

	return "error";
}

void Session::createEmailVerificationCode()
{
	uint32_t* code_p = (uint32_t*)&mEmailVerificationCode;
	for (int i = 0; i < EMAIL_VERIFICATION_CODE_SIZE / 4; i++) {
		code_p[i] = randombytes_random();
	}

}
/*
bool Session::useOrGeneratePassphrase(const std::string& passphase)
{
	if (passphase != "" && User::validatePassphrase(passphase)) {
		// passphrase is valid 
		setPassphrase(passphase);
		updateState(SESSION_STATE_PASSPHRASE_SHOWN);
		return true;
	}
	else {
		mPassphrase = User::generateNewPassphrase(&ServerConfig::g_Mnemonic_WordLists[ServerConfig::MNEMONIC_BIP0039_SORTED_ORDER]);
		updateState(SESSION_STATE_PASSPHRASE_GENERATED);
		return true;
	}
}
*/
bool Session::generatePassphrase()
{
	mPassphrase = User::generateNewPassphrase(&ServerConfig::g_Mnemonic_WordLists[ServerConfig::MNEMONIC_BIP0039_SORTED_ORDER]);
	updateState(SESSION_STATE_PASSPHRASE_GENERATED);
	return true;
}

bool Session::generateKeys(bool savePrivkey, bool savePassphrase)
{
	bool validUser = true;
	if (mSessionUser) {
		if (!mSessionUser->generateKeys(savePrivkey, mPassphrase, this)) {
			validUser = false;
		}
		else {
			if (savePassphrase) {
				printf("[Session::generateKeys] create save passphrase task\n");
				UniLib::controller::TaskPtr savePassphrase(new WritePassphraseIntoDB(mSessionUser->getDBId(), mPassphrase));
				savePassphrase->setFinishCommand(new SessionStateUpdateCommand(SESSION_STATE_PASSPHRASE_WRITTEN, this));
				savePassphrase->scheduleTask(savePassphrase);
			}
		}
	}
	else {
		validUser = false;
	}
	if (!validUser) {
		addError(new Error("Benutzer", "Kein g&uuml;ltiger Benutzer, bitte logge dich erneut ein."));
		return false;
	}
	// delete passphrase after all went well
	mPassphrase.clear();

	return true;
}