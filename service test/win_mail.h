#ifndef WIN_MAIL_H
#define WIN_MAIL_H

#include <MAPI.h>
#include <string>

bool win_mail_login(const std::string& name, const std::string& pwd);
bool win_mail_logoff();
bool win_mail_send(MapiMessage& msg);
MapiMessage create_win_mail_msg(const std::string& recipName, const std::string& recipAddr, const std::string& subject, const std::string& text);

#endif //WIN_MAIL_H