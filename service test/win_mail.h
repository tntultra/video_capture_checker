#ifndef WIN_MAIL_H
#define WIN_MAIL_H

#include "video_checker.h"

#include <MAPI.h>
#include <curl/curl.h>

struct TStringData {
	std::string msg;
	size_t bytesleft;

	explicit TStringData(std::string &&m) : 
		msg{ m }, 
		bytesleft{ msg.size() }
	{}

	explicit TStringData(std::string &m) = delete;
};

struct EMAIL_Data
{
	explicit EMAIL_Data (const std::string& to, const std::string& from, const std::string& nameFrom, const std::string& password, const std::string& url, 
		const VSTR& cc = {}, const VSTR& bcc = {}) :
		To (to), From (from), NameFrom(nameFrom), Password(password), URL(url), CC (cc), BCC (bcc)
	{}


	std::string To;
	std::string From, NameFrom, Password;
	std::string URL;
	VSTR CC, BCC;
};

struct EMAIL_Text
{
	std::string Subject;
	std::string Body;
};

class CURL_email
{
public:
	explicit CURL_email(EMAIL_Data data, EMAIL_Text text = {}) :
		Data (data), Text (text) {}

	void set_text(EMAIL_Text text){
		Text = text;
	}

	static CURL_email get_email_data_from_ini();//factory

	std::string current_time() const;
	CURLcode send() const;
private:
	// data
	EMAIL_Data Data;
	EMAIL_Text Text;
	// functions
	std::string set_payload_text() const;
	static std::string generate_message_id();
	std::string generate_ccs() const;
	// static functions
	static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp);
};



bool win_mail_login(std::string name, std::string pwd);
bool win_mail_logoff();
bool win_mail_send(MapiMessage& msg);
MapiMessage create_win_mail_msg(std::string recipName, std::string recipAddr, std::string subject, std::string text);

#endif //WIN_MAIL_H