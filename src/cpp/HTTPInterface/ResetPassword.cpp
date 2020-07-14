#include "ResetPassword.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/DeflatingStream.h"


#line 6 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 
#include "../SingletonManager/LanguageManager.h"
#include "../SingletonManager/SessionManager.h"
#include "../SingletonManager/EmailManager.h"
#include "../controller/User.h"
#include "../controller/UserBackups.h"
	
enum PageState {
	PAGE_EMAIL_ASK,
	PAGE_ASK,
	PAGE_WAIT_EMAIL,
	PAGE_WAIT_ADMIN,
	PAGE_EMAIL_ALREADY_SEND
	
};
#line 1 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
 
#include "../ServerConfig.h"	


void ResetPassword::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html");
	bool _compressResponse(request.hasToken("Accept-Encoding", "gzip"));
	if (_compressResponse) response.set("Content-Encoding", "gzip");

	Poco::Net::HTMLForm form(request, request.stream());
#line 21 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 

	PageState state = PAGE_EMAIL_ASK;
	auto lm = LanguageManager::getInstance();
	auto sm = SessionManager::getInstance();
	auto adminReceiver = EmailManager::getInstance()->getAdminReceiver();
	
	const char* pageName = "Passwort vergessen";
	auto lang = chooseLanguage(request);
	
	// class="btn btn-outline-secondary flag-btn"
	// class="btn btn-secondary disabled flag-btn" disabled
	std::string eng_btn_classes = "";
	std::string de_btn_classes = "";
	
	auto langCatalog = lm->getFreeCatalog(lang);
	
	std::string emailInputClass = "form-control";
	std::string passphraseRadioClass = "group";
	
	std::string email = "";
	
	if(!form.empty()) {
		auto session = sm->getNewSession();		
		email = form.get("email", "");
		auto passphraseMemorized = form.get("passphrase_memorized", "");
		auto user = controller::User::create();
		auto ask = form.get("ask_passphrase", "");
		
		if(email != "") 
		{
			bool user_exist = false;
			bool sendUserEmail = false;
			
			if(!sm->isValid(email, VALIDATE_EMAIL)) {
				addError(new Error(gettext(session, "E-Mail"), gettext(session, "Das ist keine g&uuml;ltige E-Mail Adresse")), false);
				emailInputClass += " is-invalid"; 
			}
			user_exist = user->load(email) == 1;
			
			if(ask == "true") 
			{
				if(passphraseMemorized == "") {
					addError(new Error(gettext(session, "Passphrase"), gettext(session, "Bitte w&auml;hle eine Option aus.")), false);
					passphraseRadioClass += " group-is-invalid"; 
				} else if(passphraseMemorized == "true") {
					sendUserEmail = true;
				}
				
			} 
			else
			{
				if(user_exist && (!user->tryLoadPassphraseUserBackup() || !user->hasPublicKey())) {
					sendUserEmail = true;
				}
			}
			
			if(!errorCount()) 
			{
				// send reset password email
				int result = 0;
				if(user_exist) {
					result = session->sendResetPasswordEmail(user, sendUserEmail);
				}
			
				if(2 == result) {
					state = PAGE_EMAIL_ALREADY_SEND;
				} else if(sendUserEmail) {
					state = PAGE_WAIT_EMAIL;
				} else {
					state = PAGE_WAIT_ADMIN;
				}
			} 
		} 
		else 
		{
			addError(new Error(gettext(session, "E-Mail"), gettext(session, "E-Mail Adresse nicht angegeben.")), false);
			emailInputClass += " is-invalid"; 
		}
		
	}
	
	
#line 3 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
 
	bool withMaterialIcons = false; 
	std::ostream& _responseStream = response.send();
	Poco::DeflatingOutputStream _gzipStream(_responseStream, Poco::DeflatingStreamBuf::STREAM_GZIP, 1);
	std::ostream& responseStream = _compressResponse ? _gzipStream : _responseStream;
	responseStream << "\n";
	// begin include login_header.cpsp
	// begin include header.cpsp
	responseStream << "\n";
	responseStream << "<!DOCTYPE html>\n";
	responseStream << "<html>\n";
	responseStream << "<head>\n";
	responseStream << "<meta charset=\"UTF-8\">\n";
	responseStream << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">\n";
	responseStream << "<title>Gradido Login Server: ";
#line 11 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
	responseStream << ( pageName );
	responseStream << "</title>\n";
	responseStream << "<link rel=\"stylesheet\" type=\"text/css\" href=\"";
#line 12 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
	responseStream << ( ServerConfig::g_php_serverPath );
	responseStream << "css/loginServer/style.css\">\n";
#line 13 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
 if(withMaterialIcons) { 	responseStream << "\n";
	responseStream << "<link rel=\"stylesheet\" type=\"text/css\" href=\"";
#line 14 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
	responseStream << ( ServerConfig::g_php_serverPath );
	responseStream << "css/materialdesignicons.min.css\">\n";
#line 15 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
 } 	responseStream << "\n";
	responseStream << "</head>\n";
	responseStream << "<body class=\"header-fixed\">\n";
	responseStream << "<div class=\"versionstring dev-info\">\n";
	responseStream << "\t<p class=\"grd_small\">Login Server in Entwicklung</p>\n";
	responseStream << "\t<p class=\"grd_small\">Alpha ";
#line 20 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
	responseStream << ( ServerConfig::g_versionString );
	responseStream << "</p>\n";
	responseStream << "</div>\n";
	responseStream << "<!--<nav class=\"grd-left-bar expanded\" data-topbar role=\"navigation\">\n";
	responseStream << "\t<div class=\"grd-left-bar-section\">\n";
	responseStream << "\t\t<ul class=\"grd-no-style\">\n";
	responseStream << "\t\t  <li><a href=\"";
#line 25 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\header.cpsp"
	responseStream << ( ServerConfig::g_php_serverPath );
	responseStream << "\" class=\"grd-nav-bn\">Startseite</a>\n";
	responseStream << "\t\t  <li><a href=\"./account/logout\" class=\"grd-nav-bn\">Logout</a></li>\n";
	responseStream << "\t\t</ul>\n";
	responseStream << "\t</div>\n";
	responseStream << "</nav>-->";
	// end include header.cpsp
	responseStream << "\n";
	responseStream << "<div class=\"authentication-theme auth-style_1\">\n";
	responseStream << "      <div class=\"row\">\n";
	responseStream << "        <div class=\"col-12 logo-section\">\n";
	responseStream << "          <a href=\"";
#line 5 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\login_header.cpsp"
	responseStream << ( ServerConfig::g_php_serverPath );
	responseStream << "\" class=\"logo\">\n";
	responseStream << "            <picture>\n";
	responseStream << "\t\t\t\t<source srcset=\"";
#line 7 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\login_header.cpsp"
	responseStream << ( ServerConfig::g_php_serverPath );
	responseStream << "img/logo_schrift.webp\" type=\"image/webp\">\n";
	responseStream << "\t\t\t\t<source srcset=\"";
#line 8 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\login_header.cpsp"
	responseStream << ( ServerConfig::g_php_serverPath );
	responseStream << "img/logo_schrift.png\" type=\"image/png\"> \n";
	responseStream << "\t\t\t\t<img src=\"";
#line 9 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\login_header.cpsp"
	responseStream << ( ServerConfig::g_php_serverPath );
	responseStream << "img/logo_schrift.png\" alt=\"logo\" />\n";
	responseStream << "\t\t\t</picture>\n";
	responseStream << "          </a>\n";
	responseStream << "        </div>\n";
	responseStream << "      </div>\n";
	responseStream << "      <div class=\"row\">\n";
	responseStream << "        <div class=\"col-lg-5 col-md-7 col-sm-9 col-11 mx-auto\">\n";
	responseStream << "          <div class=\"grid\">\n";
	responseStream << "            <div class=\"center-ul-container\">\n";
	responseStream << "              ";
#line 18 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\login_header.cpsp"
	responseStream << ( getErrorsHtml() );
	responseStream << "\n";
	responseStream << "            </div>\n";
	responseStream << "            <div class=\"grid-body\">";
	// end include login_header.cpsp
	responseStream << "\n";
	responseStream << "<style type=\"text/css\">\n";
	responseStream << ".group {\n";
	responseStream << "\tpadding-left:10px;\n";
	responseStream << "\tpadding-top:10px;\n";
	responseStream << "\tpadding-bottom:10px;\n";
	responseStream << "}\n";
	responseStream << ".group-is-invalid {\n";
	responseStream << "\tbackground-color: rgba(240,130,95,.2);\n";
	responseStream << "\t border-color:#dc3545;\n";
	responseStream << "\t padding-right:calc(1.5em + .75rem);\n";
	responseStream << "\t background-image:url(\"data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' fill='%23dc3545' viewBox='-2 -2 7 7'%3e%3cpath stroke='%23dc3545' d='M0 0l3 3m0-3L0 3'/%3e%3ccircle r='.5'/%3e%3ccircle cx='3' r='.5'/%3e%3ccircle cy='3' r='.5'/%3e%3ccircle cx='3' cy='3' r='.5'/%3e%3c/svg%3E\");\n";
	responseStream << "\t background-repeat:no-repeat;\n";
	responseStream << "\t background-position:center right calc(.375em + .1875rem);\n";
	responseStream << "\t background-size:calc(.75em + .375rem) calc(.75em + .375rem)\n";
	responseStream << "}\n";
	responseStream << ".group-is-invalid .radio label .input-frame::before {\n";
	responseStream << "\tborder-color:red;\n";
	responseStream << "}\n";
	responseStream << "\n";
	responseStream << "</style>\n";
	responseStream << "    ";
#line 125 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 if(state == PAGE_EMAIL_ASK) { 	responseStream << "\n";
	responseStream << "\t\t<form action=\"";
#line 126 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( ServerConfig::g_serverPath );
	responseStream << "/resetPassword\">\n";
	responseStream << "\t\t\t<div class=\"item-wrapper\">\n";
	responseStream << "\t\t\t  <div class=\"form-group\">\n";
	responseStream << "\t\t\t\t<label for=\"email\">";
#line 129 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Gebe bitte hier deine E-Mail Adresse an:") );
	responseStream << "&nbsp;&nbsp;&nbsp;&nbsp;</label>\n";
	responseStream << "\t\t\t\t<input type=\"text\" class=\"";
#line 130 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( emailInputClass );
	responseStream << "\" name=\"email\" id=\"email\" placeholder=\"E-Mail\" value=\"";
#line 130 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( email );
	responseStream << "\">\n";
	responseStream << "\t\t\t  </div>\n";
	responseStream << "\t\t\t  <button type=\"submit\" class=\"btn btn-sm btn-primary\" >";
#line 132 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Bestätigen") );
	responseStream << "</button>\n";
	responseStream << "\t\t\t</div>\n";
	responseStream << "\t\t</form>\n";
	responseStream << "\t";
#line 135 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 } else if(state == PAGE_ASK) { 	responseStream << "\n";
	responseStream << "\t\t";
	// begin include flags.cpsp
	responseStream << "<form method=\"GET\" action=\"\">\n";
	responseStream << "\t<div class=\"row pull-right-row\">\n";
	responseStream << "\t  <div class=\"equel-grid pull-right\">\n";
	responseStream << "\t\t<div class=\"grid-body-small text-center\">\n";
	responseStream << "\t\t\t<button id=\"flag-england\" name=\"lang\" value=\"en\" title=\"English\" type=\"submit\" ";
#line 5 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\flags.cpsp"
 if(lang != LANG_EN) { 	responseStream << "class=\"btn btn-outline-secondary flag-btn\"";
#line 5 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\flags.cpsp"
 } 
			else { 	responseStream << "class=\"btn btn-secondary disabled flag-btn\" disabled";
#line 6 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\flags.cpsp"
 } 	responseStream << ">\n";
	responseStream << "\t\t\t  <span class=\"flag-england\"></span>\n";
	responseStream << "\t\t\t</button>\n";
	responseStream << "\t\t</div>\n";
	responseStream << "\t  </div>\n";
	responseStream << "\t  <div class=\"equel-grid pull-right\">\n";
	responseStream << "\t\t<div class=\"grid-body-small text-center\">\n";
	responseStream << "\t\t\t<button id=\"flag-germany\" name=\"lang\" value=\"de\" title=\"Deutsch\" type=\"submit\" ";
#line 13 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\flags.cpsp"
 if(lang != LANG_DE) { 	responseStream << "class=\"btn btn-outline-secondary flag-btn\"";
#line 13 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\flags.cpsp"
 } 
			else { 	responseStream << "class=\"btn btn-secondary disabled flag-btn\" disabled";
#line 14 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\flags.cpsp"
 } 	responseStream << ">\n";
	responseStream << "\t\t\t  <span class=\"flag-germany\"></span>\n";
	responseStream << "\t\t\t</button>\n";
	responseStream << "\t\t</div>\n";
	responseStream << "\t  </div>\n";
	responseStream << "\t</div>\n";
	responseStream << "</form>";
	// end include flags.cpsp
	responseStream << "\n";
	responseStream << "\t\t<form action=\"";
#line 137 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( ServerConfig::g_serverPath );
	responseStream << "/resetPassword\">\n";
	responseStream << "\t\t\t<div class=\"item-wrapper\">\n";
	responseStream << "\t\t\t  <div class=\"form-group\">\n";
	responseStream << "\t\t\t\t<label for=\"email\">";
#line 140 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Gebe bitte hier deine E-Mail Adresse an:") );
	responseStream << "&nbsp;&nbsp;&nbsp;&nbsp;</label>\n";
	responseStream << "\t\t\t\t<input type=\"text\" class=\"";
#line 141 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( emailInputClass );
	responseStream << "\" name=\"email\" id=\"email\" placeholder=\"E-Mail\" value=\"";
#line 141 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( email );
	responseStream << "\">\n";
	responseStream << "\t\t\t\t<label>";
#line 142 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Hast du dir deine Passphrase notiert oder gemerkt?") );
	responseStream << "</label>    \n";
	responseStream << "\t\t\t\t<input type=\"hidden\" name=\"ask_passphrase\" value=\"true\">\n";
	responseStream << "\t\t\t\t<div class=\"";
#line 144 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( passphraseRadioClass );
	responseStream << "\">\n";
	responseStream << "\t\t\t\t\t<div class=\"radio\">\n";
	responseStream << "\t\t\t\t\t  <label class=\"radio-label mr-4\">\n";
	responseStream << "\t\t\t\t\t\t<input name=\"passphrase_memorized\" onclick=\"removeGroupInvalidClass()\"  type=\"radio\" value=\"true\">";
#line 147 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Ja") );
	responseStream << " <i class=\"input-frame\"></i>\n";
	responseStream << "\t\t\t\t\t  </label>\n";
	responseStream << "\t\t\t\t\t</div>\n";
	responseStream << "\t\t\t\t\t<div class=\"radio\">\n";
	responseStream << "\t\t\t\t\t  <label class=\"radio-label\">\n";
	responseStream << "\t\t\t\t\t\t<input name=\"passphrase_memorized\" onclick=\"removeGroupInvalidClass()\" type=\"radio\" value=\"false\">";
#line 152 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Nein") );
	responseStream << " <i class=\"input-frame\"></i>\n";
	responseStream << "\t\t\t\t\t  </label>\n";
	responseStream << "\t\t\t\t\t</div>\n";
	responseStream << "\t\t\t\t</div>\n";
	responseStream << "\t\t\t  </div>\n";
	responseStream << "\t\t\t  <button type=\"submit\" class=\"btn btn-sm btn-primary\" name=\"ask\" >";
#line 157 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Absenden") );
	responseStream << "</button>\n";
	responseStream << "\t\t\t</div>\n";
	responseStream << "\t\t</form>\n";
	responseStream << "\t  ";
#line 160 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 } else if(state == PAGE_WAIT_EMAIL) { 	responseStream << "\n";
	responseStream << "\t\t\t";
#line 161 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Dir wird eine E-Mail zugeschickt um dein Passwort zur&uuml;ckzusetzen.") );
	responseStream << "\n";
	responseStream << "\t  ";
#line 162 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 } else if(state == PAGE_WAIT_ADMIN) { 	responseStream << "\n";
	responseStream << "\t\t\t";
#line 163 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Der Admin hat eine E-Mail bekommen und wird sich bei dir melden.") );
	responseStream << "\n";
	responseStream << "\t  ";
#line 164 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 } else if(state == PAGE_EMAIL_ALREADY_SEND) { 	responseStream << "\n";
	responseStream << "\t\t\t<p>";
#line 165 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Du hast bereits eine E-Mail bekommen. Bitte schau auch in dein Spam-Verzeichnis nach. ") );
	responseStream << "</p>\n";
	responseStream << "\t\t\t<p>";
#line 166 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("Du hast wirklich keine E-Mail erhalten und auch schon ein paar Minuten gewartet?") );
	responseStream << "</p>\n";
	responseStream << "\t\t\t<p><b><a href=\"mailto:";
#line 167 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( adminReceiver );
	responseStream << "?subject=Error Reset Password email&amp;body=Hallo Dario,%0D%0A%0D%0Aich habe keine Passwort zurücksetzen E-Mail erhalten,%0D%0Akannst du das prüfen?%0D%0A%0D%0AMit freundlichen Grüßen%0D%0A\">";
#line 167 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
	responseStream << ( langCatalog->gettext("E-Mail an Support schicken"));
	responseStream << "</a></b></p>\n";
	responseStream << "\t  ";
#line 168 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\resetPassword.cpsp"
 } 	responseStream << "\n";
	responseStream << "          </div>  \n";
	responseStream << "          </div>\n";
	responseStream << "        </div>\n";
	responseStream << "      </div>\n";
	responseStream << "      <div class=\"auth_footer\">\n";
	responseStream << "        <p class=\"text-muted text-center\">© Gradido 2020</p>\n";
	responseStream << "      </div>\n";
	responseStream << "    </div>\n";
	responseStream << "\t<script type=\"text/javascript\">\n";
	responseStream << "\t\tfunction removeGroupInvalidClass() {\n";
	responseStream << "\t\t\tvar elements = document.getElementsByClassName(\"group-is-invalid\");\n";
	responseStream << "\t\t\tif(elements.length > 0) {\n";
	responseStream << "\t\t\t\telements[0].classList.remove(\"group-is-invalid\");\n";
	responseStream << "\t\t\t}\n";
	responseStream << "\t\t}\n";
	responseStream << "\t</script>\n";
	// begin include footer.cpsp
	responseStream << "\t<div class=\"grd-time-used dev-info\">\n";
	responseStream << "\t\t\t";
#line 2 "F:\\Gradido\\gradido_login_server\\src\\cpsp\\footer.cpsp"
	responseStream << ( mTimeProfiler.string() );
	responseStream << "\n";
	responseStream << "\t</div>\n";
	responseStream << "</body>\n";
	responseStream << "</html>";
	// end include footer.cpsp
	responseStream << "\n";
	if (_compressResponse) _gzipStream.close();
}
