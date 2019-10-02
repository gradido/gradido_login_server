#include "PageRequestHandlerFactory.h"

#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTMLForm.h"

#include "ConfigPage.h"
#include "LoginPage.h"
#include "RegisterPage.h"
#include "HandleFileRequest.h"
#include "DashboardPage.h"
#include "CheckEmailPage.h"
#include "PassphrasePage.h"
#include "SaveKeysPage.h"

#include "../SingletonManager/SessionManager.h"

PageRequestHandlerFactory::PageRequestHandlerFactory()
	: mRemoveGETParameters("^/([a-zA-Z0-9_-]*)")
{
	
}

Poco::Net::HTTPRequestHandler* PageRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
	//printf("request uri: %s\n", request.getURI().data());

	std::string uri = request.getURI();
	std::string url_first_part;
	mRemoveGETParameters.extract(uri, url_first_part);

	printf("[PageRequestHandlerFactory] uri: %s, first part: %s\n", uri.data(), url_first_part.data());
	auto referer = request.find("Referer");
	if (referer != request.end()) {
		printf("referer: %s\n", referer->second.data());
	}

	// check if user has valid session
	Poco::Net::NameValueCollection cookies;
	request.getCookies(cookies);

	int session_id = 0;

	try {
		session_id = atoi(cookies.get("user").data());
	} catch (...) {}
	auto sm = SessionManager::getInstance();
	auto s = sm->getSession(session_id);
	
	if (url_first_part == "/checkEmail") {
		//return new CheckEmailPage(s);
		return handleCheckEmail(s, uri, request);
	}
	if (s) {
		if(url_first_part == "/logout") {
			sm->releseSession(s);
			printf("session released\n");
			return new LoginPage;
		}
		auto sessionState = s->getSessionState();
		if(sessionState == SESSION_STATE_EMAIL_VERIFICATION_CODE_CHECKED || 
		   sessionState == SESSION_STATE_PASSPHRASE_GENERATED) {
		//if (url_first_part == "/passphrase") {
			//return handlePassphrase(s, request);
			return new PassphrasePage(s);
		}
		else if(sessionState == SESSION_STATE_PASSPHRASE_SHOWN) {
		//else if (uri == "/saveKeys") {
			return new SaveKeysPage(s);
		}
		if (s && s->getUser()) {
			return new DashboardPage(s);
		}
	} else {

		if (uri == "/config") {
			return new ConfigPage;
		}
		else if (uri == "/login") {
			return new LoginPage;
		}
		else if (uri == "/register") {
			return new RegisterPage;
		}
	}
	return new LoginPage;
	//return new HandleFileRequest;
	//return new PageRequestHandlerFactory;
}

Poco::Net::HTTPRequestHandler* PageRequestHandlerFactory::handleCheckEmail(Session* session, const std::string uri, const Poco::Net::HTTPServerRequest& request)
{
	Poco::Net::HTMLForm form(request);
	unsigned long long verificationCode = 0;

	// if verification code is valid, go to next page, passphrase
	// login via verification code, if no session is active
	// try to get code from form get parameter
	if (!form.empty()) {
		try {
			verificationCode = stoll(form.get("email-verification-code", "0"));
		} catch (...) {}
	}
	// try to get code from uri parameter
	if (!verificationCode) {
		size_t pos = uri.find_last_of("/");
		try {
			verificationCode = stoll(uri.substr(pos + 1));
		} catch (...) {}
	}

	// if no verification code given or error with given code, show form
	if (!verificationCode) {
		return new CheckEmailPage(session);
	}

	// we have a verification code, now let's check that thing
	auto sm = SessionManager::getInstance();

	// no session or active session don't belong to verification code
	if (!session || session->getEmailVerificationCode() != verificationCode) {
		session = sm->findByEmailVerificationCode(verificationCode);
	}
	// no suitable session in memory, try to create one from db data
	if (!session) {
		session = sm->getNewSession();
		if (session->loadFromEmailVerificationCode(verificationCode)) {
			// login not possible in this function
			/*auto cookie_id = session->getHandle();
			auto user_host = request.clientAddress().host();
			session->setClientIp(user_host);
			response.addCookie(Poco::Net::HTTPCookie("user", std::to_string(cookie_id)));
			*/
		}
		else {
			sm->releseSession(session);
			session = nullptr;
		}
	}
	// suitable session found or created
	if (session) {
		// update session, mark as verified 
		if (session->updateEmailVerification(verificationCode)) {
			return new PassphrasePage(session);
		}
		
	}
	
	return new CheckEmailPage(session);
	
}
