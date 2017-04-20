#ifndef WIN_MAIL_H
#define WIN_MAIL_H

#include <MAPI.h>
#include <string>

bool win_mail_login(std::string name, std::string pwd);
bool win_mail_logoff();
bool win_mail_send(MapiMessage& msg);
MapiMessage create_win_mail_msg(std::string recipName, std::string recipAddr, std::string subject, std::string text);

#endif //WIN_MAIL_H