#include "ElopageWebhook.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/DeflatingStream.h"
#include "Poco/URI.h"
#include "Poco/Data/Binding.h"

using namespace Poco::Data::Keywords;

#include "../SingletonManager/ConnectionManager.h"
#include "../SingletonManager/ErrorManager.h"
#include "../SingletonManager/SessionManager.h"

#include "../ServerConfig.h"

#include "../tasks/PrepareEmailTask.h"
#include "../tasks/SendEmailTask.h"

#include "../model/EmailVerificationCode.h"




void ElopageWebhook::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	// simply write request to file for later lookup
	//ServerConfig::writeToFile(request.stream(), "elopage_webhook_requests.txt");

	


	std::istream& stream = request.stream();
	std::string completeRequest;
	Poco::Net::NameValueCollection elopageRequestData;
	int breakCount = 100;
	while (stream.good() && breakCount > 0) {
		char dummy;
		char keyBuffer[30]; memset(keyBuffer, 0, 30);
		char valueBuffer[75]; memset(valueBuffer, 0, 75);
		/*stream.get(keyBuffer, 30, '=').get(dummy)
			  .get(valueBuffer, 35, '&').get(dummy);*/
		std::string line;
		std::getline(stream, line);
		int mode = 0;
		int cursor = 0;
		for (int i = 0; i < line.size(); i++) {
			char c = line.at(i);
			completeRequest += c;
			if (c == '\n') break;
			if (c == '+') {
				c = ' ';
			}
			if (c == '&') {
				mode = 0;
				cursor = 0;
				std::string urlDecodedValue;
				Poco::URI::decode(valueBuffer, urlDecodedValue);
				elopageRequestData.set(keyBuffer, urlDecodedValue);
				memset(keyBuffer, 0, 30);
				memset(valueBuffer, 0, 75);
				continue;
			}
			switch (mode) {
			case 0: // read key
				if (c == '=') {
					mode = 1;
					cursor = 0;
					continue;
				}
				if (cursor < 29) {
					keyBuffer[cursor++] = c;
				}
				else {
					int zahl = 1;
				}
				break;
			case 1: // read value
				if (cursor < 74) {
					valueBuffer[cursor++] = c;
				}
				else {
					int zahl = 1;
				}
				break;
			}
		}
		//printf("[ElopageWebhook::handleRequest] key: %s, value: %s\n", keyBuffer, valueBuffer);
	///	elopageRequestData.set(keyBuffer, valueBuffer);
		stream.good();
		breakCount--;
	}

	// write stream result also to file
	static Poco::Mutex mutex;

	mutex.lock();

	Poco::FileOutputStream file("elopage_webhook_requests.txt", std::ios::out | std::ios::app);

	if (!file.good()) {
		printf("[ElopageWebhook::handleRequest] error creating file with name: elopage_webhook_requests.txt\n");
		mutex.unlock();
		return;
	}

	Poco::LocalDateTime now;

	std::string dateTimeStr = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::ISO8601_FORMAT);
	file << dateTimeStr << std::endl;
	file << completeRequest << std::endl;
	file << std::endl;
	file.close();
	mutex.unlock();


	UniLib::controller::TaskPtr handleElopageTask(new HandleElopageRequestTask(elopageRequestData));
	handleElopageTask->scheduleTask(handleElopageTask);

	response.setChunkedTransferEncoding(true);
	response.setContentType("application/json");
	bool _compressResponse(request.hasToken("Accept-Encoding", "gzip"));
	if (_compressResponse) response.set("Content-Encoding", "gzip");

	
	std::ostream& _responseStream = response.send();
	Poco::DeflatingOutputStream _gzipStream(_responseStream, Poco::DeflatingStreamBuf::STREAM_GZIP, 1);
	std::ostream& responseStream = _compressResponse ? _gzipStream : _responseStream;
	
	if (_compressResponse) _gzipStream.close();
}


// ****************************************************************************
HandleElopageRequestTask::HandleElopageRequestTask(Poco::Net::NameValueCollection& requestData)
	: CPUTask(ServerConfig::g_CPUScheduler), mRequestData(requestData) 
{
}

bool HandleElopageRequestTask::validateInput()
{
	auto sm = SessionManager::getInstance();
	if (mEmail == "" || !sm->isValid(mEmail, VALIDATE_EMAIL)) {
		addError(new Error(__FUNCTION__, "email is invalid or empty"));
		return false;
	}
	if (mFirstName == "" || !sm->isValid(mFirstName, VALIDATE_NAME)) {
		addError(new Error(__FUNCTION__, "first name is invalid or empty"));
		return false;
	}

	if (mLastName == "" || !sm->isValid(mLastName, VALIDATE_NAME)) {
		addError(new Error(__FUNCTION__, "last name is invalid or empty"));
		return false;
	}

	return true;
}


void HandleElopageRequestTask::writeUserIntoDB()
{
	auto cm = ConnectionManager::getInstance();
	auto session = cm->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
	Poco::Data::Statement insert(session);
	insert << "INSERT INTO users (email, first_name, last_name) VALUES(?, ?, ?);",
		use(mEmail), use(mFirstName), use(mLastName);
	try {
		insert.execute();
	}
	catch (Poco::Exception& ex) {
		addError(new ParamError(__FUNCTION__, "mysql error", ex.displayText().data()));
	}
}

int HandleElopageRequestTask::getUserIdFromDB()
{
	auto cm = ConnectionManager::getInstance();
	auto session = cm->getConnection(CONNECTION_MYSQL_LOGIN_SERVER);
	Poco::Data::Statement select(session);
	int user_id = 0;
	select << "SELECT id from users where email = ?;",
		into(user_id), use(mEmail);
	try {
		select.execute();
	}
	catch (Poco::Exception& ex) {
		addError(new ParamError(__FUNCTION__, "mysql error selecting from db", ex.displayText().data()));
	}

	return user_id;
}


int HandleElopageRequestTask::run()
{
	// get input data
	
	mEmail = mRequestData.get("payer[email]", "");
	mFirstName = mRequestData.get("payer[first_name]", "");
	mLastName = mRequestData.get("payer[last_name]", "");
	std::string order_id = mRequestData.get("order_id", "");

	addError(new ParamError("HandleElopageRequestTask", "order_id", order_id.data()));

	// validate input
	if (!validateInput()) {
		// if input is invalid we can stop now
		sendErrorsAsEmail();
		return -1;
	}

	// if user exist we can stop now
	if (getUserIdFromDB()) {
		sendErrorsAsEmail();
		return -2;
	}

	// if user with this email didn't exist
	// we can create a new user and send a email to him
	
	// prepare email in advance
	// create connection to email server
	UniLib::controller::TaskPtr prepareEmail(new PrepareEmailTask(ServerConfig::g_CPUScheduler));
	prepareEmail->scheduleTask(prepareEmail);

	// write user entry into db
	writeUserIntoDB();
	
	// get user id from db
	int user_id = getUserIdFromDB();
	// we didn't get a user_id, something went wrong
	if (!user_id) {
		addError(new Error("User loadEntryDBId", "user_id is zero"));
		sendErrorsAsEmail();
		return -3;
	}

	Poco::AutoPtr<EmailVerificationCode> emailVerification(new EmailVerificationCode(user_id));

	// create email verification code
	if (!emailVerification->getCode()) {
		// exit if email verification code is empty
		addError(new Error("Email verification", "code is empty, error in random?"));
		sendErrorsAsEmail();
		return -4;
	}

	// write email verification code into db
	UniLib::controller::TaskPtr saveEmailVerificationCode(new ModelInsertTask(emailVerification));
	saveEmailVerificationCode->scheduleTask(saveEmailVerificationCode);

	// send email to user
	auto message = new Poco::Net::MailMessage;

	message->addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, mEmail));
	message->setSubject("Gradido: E-Mail Verification");
	std::stringstream ss;
	ss << "Hallo " << mFirstName << " " << mLastName << "," << std::endl << std::endl;
	ss << "Du oder jemand anderes hat sich soeben mit dieser E-Mail Adresse bei Elopage f�r Gradido angemeldet. " << std::endl;
	ss << "Um dein Gradido Konto anzulegen und deine E-Mail zu best�tigen," << std::endl;
	ss << "klicke bitte auf den Link: https://gradido2.dario-rekowski.de/account/checkEmail/" << emailVerification->getCode() << std::endl;
	ss << "oder kopiere den Code: " << emailVerification->getCode() << " selbst dort hinein." << std::endl << std::endl;
	ss << "Mit freundlichen Gr��e" << std::endl;
	ss << "Dario, Gradido Server Admin" << std::endl;


	message->addContent(new Poco::Net::StringPartSource(ss.str()));

	UniLib::controller::TaskPtr sendEmail(new SendEmailTask(message, ServerConfig::g_CPUScheduler, 1));
	sendEmail->setParentTaskPtrInArray(prepareEmail, 0);
	sendEmail->setParentTaskPtrInArray(saveEmailVerificationCode, 1);
	sendEmail->scheduleTask(sendEmail);

	// if errors occured, send via email
	//if (errorCount() > 1) {
		sendErrorsAsEmail();
	//}
	
	
	return 0;
}