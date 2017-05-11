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

class CURL_email
{
public:
	CURL_email(const std::string &to_,
		const std::string &from_,
		const std::string &nameFrom_,
		const std::string &subject_,
		const std::string &body_,
		const VSTR &cc_ = {},
		const VSTR &bcc_ = {}
	);
	std::string current_time() const;
	CURLcode send(const std::string &url, const std::string &username, const std::string &password) const;
private:
	// data
	std::string To, From, NameFrom, Subject, Body;
	VSTR CC, BCC;
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