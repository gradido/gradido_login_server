#include "JsonGetUserInfos.h"

#include "../lib/DataTypeConverter.h"
#include "../SingletonManager/SessionManager.h"
#include "../controller/User.h"
#include "../controller/EmailVerificationCode.h"

#include "../ServerConfig.h"

Poco::JSON::Object* JsonGetUserInfos::handle(Poco::Dynamic::Var params)
{
	/*
	'session_id' => $session_id,
	'email' => $email,
	'ask' => ['EmailOptIn.Register']
	*/
	// incoming
	int session_id = 0;
	std::string email;
	Poco::JSON::Array::Ptr askArray;

	auto sm = SessionManager::getInstance();

	// if is json object
	if (params.type() == typeid(Poco::JSON::Object::Ptr)) {
		Poco::JSON::Object::Ptr paramJsonObject = params.extract<Poco::JSON::Object::Ptr>();
		/// Throws a RangeException if the value does not fit
		/// into the result variable.
		/// Throws a NotImplementedException if conversion is
		/// not available for the given type.
		/// Throws InvalidAccessException if Var is empty.
		try {
			paramJsonObject->get("email").convert(email);
			paramJsonObject->get("session_id").convert(session_id);
			askArray = paramJsonObject->getArray("ask");
		}
		catch (Poco::Exception& ex) {
			return stateError("json exception", ex.displayText());
		}
	}
	else {
		return stateError("parameter format unknown");
	}

	if (!session_id) {
		return stateError("session_id invalid");
	}
	if (askArray.isNull()) {
		return stateError("ask is zero or not an array");
	}

	auto session = sm->getSession(session_id);
	if (!session) {
		return customStateError("not found", "session not found");
	}

	auto user = controller::User::create();
	if (1 != user->load(email)) {
		return customStateError("not found", "user not found");
	}
	auto userModel = user->getModel();

	
	Poco::JSON::Object* result = new Poco::JSON::Object;
	result->set("state", "success");
	Poco::JSON::Array  jsonErrorsArray;
	Poco::JSON::Object jsonUser;
	Poco::JSON::Object jsonServer;

	for (auto it = askArray->begin(); it != askArray->end(); it++) {
		auto parameter = *it;
		std::string parameterString;
		try {
			parameter.convert(parameterString);
			if (parameterString == "EmailVerificationCode.Register") {
				try {
					auto emailVerificationCode = controller::EmailVerificationCode::load(
						userModel->getID(), model::table::EMAIL_OPT_IN_REGISTER
					);
					if (!emailVerificationCode) {
						emailVerificationCode = controller::EmailVerificationCode::create(userModel->getID(), model::table::EMAIL_OPT_IN_REGISTER);
						UniLib::controller::TaskPtr insert = new model::table::ModelInsertTask(emailVerificationCode->getModel(), false);
						insert->scheduleTask(insert);
					}
					jsonUser.set("EmailVerificationCode.Register", std::to_string(emailVerificationCode->getModel()->getCode()));
				}
				catch (Poco::Exception& ex) {
					printf("exception: %s\n", ex.displayText().data());
				}
			}
			else if (parameterString == "loginServer.path") {
				jsonServer.set("loginServer.path", ServerConfig::g_serverPath);
			}
			else if (parameterString == "user.pubkeyhex") {
				jsonUser.set("pubkeyhex", userModel->getPublicKeyHex());
			}
			else if (parameterString == "user.first_name") {
				jsonUser.set("first_name", userModel->getFirstName());
			}
			else if (parameterString == "user.last_name") {
				jsonUser.set("last_name", userModel->getLastName());
			}
			else if (parameterString == "user.disabled") {
				jsonUser.set("disabled", userModel->isDisabled());
			}
		}
		catch (Poco::Exception& ex) {
			jsonErrorsArray.add("ask parameter invalid");
		}
	}
	result->set("errors", jsonErrorsArray);
	result->set("userData", jsonUser);
	result->set("server", jsonServer);
	return result;
	
}