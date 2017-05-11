#include "stdafx.h"
#include "win_mail.h"
#include "video_checker.h"
#include <boost/date_time/posix_time/posix_time.hpp>

static HMODULE hMapi32 = nullptr;
static LHANDLE pSession = 0;

CURL_email::CURL_email (const std::string& to_, const std::string& from_, const std::string& nameFrom_, const std::string& subject_, const std::string& body_, const std::string& cc_) :
	To{ to_ },
	From{ from_ },
	NameFrom{ nameFrom_ },
	Subject{ subject_},
	Body{ body_},
	CC{ cc_}
{
}

std::string CURL_email::current_time () const
{
	using namespace boost::posix_time;
	auto localTime = second_clock::local_time();
	return to_simple_string(localTime);
}

CURLcode CURL_email::send (const std::string& url, const std::string& username, const std::string& password) const
{
	auto ret = CURLE_OK;

	struct curl_slist *recipients = NULL;
	auto *curl = curl_easy_init();

	TStringData textdata{ set_payload_text() };

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, From.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		curl_easy_setopt(curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
		//curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + From + ">").c_str());
		recipients = curl_slist_append(recipients, ("<" + To + ">").c_str());
		recipients = curl_slist_append(recipients, (std::string{ "<" } +"tntteferi@gmail.com" + ">").c_str());

		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &textdata);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		ret = curl_easy_perform(curl);

		if (ret != CURLE_OK) {
			VIDEO_CHECKER::log_error("curl_easy_perform() failed: %s\n" + std::string{ curl_easy_strerror(ret) });
		}

		curl_slist_free_all(recipients);
		curl_easy_cleanup(curl);
	}

	return ret;
}

std::string CURL_email::set_payload_text() const
{
	std::string payloadText = "Date: " + current_time() + "\r\n";
	payloadText += "To: <" + To + ">\r\n";
	payloadText += "From: <" + From + "> (" + NameFrom + ")\r\n";
	payloadText += "Cc: <tntteferi@gmail.com> (Ivan)\r\n";
	payloadText += "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@\r\n";
	//payloadText += "Message-ID: <" + generate_message_id() + "@" + From.substr(From.find('@') + 1) + ">\r\n";
	payloadText += "Subject: " + Subject + "\r\n";
	payloadText += "\r\n";
	payloadText += Body + "\r\n";
	payloadText += "\r\n";
	payloadText += "\r\n"; // "It could be a lot of lines, could be MIME encoded, whatever.\r\n";
	payloadText += "\r\n"; // "Check RFC5322.\r\n";
	return payloadText;
}

size_t CURL_email::payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	auto *text = reinterpret_cast<TStringData *>(userp);

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1) || (text->bytesleft == 0)) {
		return 0;
	}

	if ((nmemb * size) >= text->msg.size()) {
		text->bytesleft = 0;
		strncpy_s(reinterpret_cast<char *>(ptr), nmemb * size, text->msg.c_str(), text->msg.size());
		return text->msg.size();
	}

	return 0;
}

std::string CURL_email::generate_message_id()
{
	const int MESSAGE_ID_LEN = 37;
	time_t t;
	struct tm tm;

	std::string ret;
	ret.resize(15);

	time(&t);
	gmtime_s(&tm, &t);

	strftime(const_cast<char *>(ret.c_str()),
		MESSAGE_ID_LEN,
		"%Y%m%d%H%M%S.",
		&tm);

	ret.reserve(MESSAGE_ID_LEN);

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	while (ret.size() < MESSAGE_ID_LEN) {
		ret += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return ret;
}



template<typename T>
T win_mail_GetProcAddress(HMODULE library, LPCSTR funcName)
{
	return reinterpret_cast<T>(::GetProcAddress(hMapi32, funcName));
}

bool win_mail_login(std::string name, std::string pwd)
{
	hMapi32 = LoadLibraryA("Mapi32.dll");
	if (!hMapi32) {
		VIDEO_CHECKER::log_error("Mapi32.dll not found!");
		return false;
	}

	// MAPILogon
	auto MAPILogon = win_mail_GetProcAddress<LPMAPILOGON>(hMapi32, "MAPILogon");
	if (!MAPILogon) {
		VIDEO_CHECKER::log_error("MAPILogon not found!");
		return false;
	}

	auto res = MAPILogon(0, nullptr, nullptr, MAPI_NEW_SESSION/* | MAPI_LOGON_UI*/, 0, &pSession);
	if (res != SUCCESS_SUCCESS) {
		VIDEO_CHECKER::log_error("MAPILogon failed!");
		return false;
	}
	return true;
}

bool win_mail_logoff()
{
	// MAPILogoff
	auto MAPILogoff = win_mail_GetProcAddress<LPMAPILOGOFF>(hMapi32, "MAPILogoff");
	if (!MAPILogoff) {
		VIDEO_CHECKER::log_error("MAPILogoff not found!");
		return false;
	}

	auto res = MAPILogoff(pSession, 0, 0, 0);
	if (res != SUCCESS_SUCCESS) {
		VIDEO_CHECKER::log_error("MAPILogoff failed!");
		return false;
	}

	FreeLibrary(hMapi32);
	return true;
}


bool win_mail_send(MapiMessage& msg)
{
	auto MAPISendMail = win_mail_GetProcAddress<LPMAPISENDMAIL>(hMapi32, "MAPISendMail");
	if (!MAPISendMail) {
		VIDEO_CHECKER::log_error("MAPISendMail not found!");
		return false;
	}
	auto res = MAPISendMail(pSession, 0, &msg, 0, 0);
	if (res != SUCCESS_SUCCESS) {
		VIDEO_CHECKER::log_error("MAPISendMail failed!");
		return false;
	}
	return true;
}

MapiMessage create_win_mail_msg(std::string recipName, std::string recipAddr, std::string subject, std::string text)
{
	auto recipDescs = new MapiRecipDesc[1]; 
	auto& recip = recipDescs[0];
	recip.ulRecipClass = MAPI_TO;
	recip.lpszName = _strdup (recipName.c_str());
	recip.lpszAddress = _strdup (recipAddr.c_str());
	recip.ulReserved = 0;
	recip.lpEntryID = nullptr;
	recip.ulEIDSize = 0;

	

	MapiMessage msg;
	msg.lpszSubject = _strdup (subject.c_str());
	msg.lpszNoteText = _strdup (text.c_str());
	msg.nRecipCount = 1;
	msg.lpRecips = recipDescs;
	msg.ulReserved = 0;
	msg.lpszMessageType = nullptr;
	msg.lpszDateReceived = nullptr;
	msg.lpszConversationID = nullptr;
	msg.flFlags = 0;
	msg.lpOriginator = nullptr;
	msg.nFileCount = 0;
	msg.lpFiles = nullptr;
	return msg;
}

void azaza()
{
	CURL *curl = curl_easy_init();
	if (curl) printf("curl_easy_init() succeeded!\n");
	else fprintf(stderr, "Error calling curl_easy_init().\n");
}