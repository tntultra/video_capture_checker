#include "stdafx.h"
#include "win_mail.h"
#include "video_checker.h"

static HMODULE hMapi32 = nullptr;
static LHANDLE pSession = 0;

template<typename T>
T win_mail_GetProcAddress(HMODULE library, LPCSTR funcName)
{
	return reinterpret_cast<T>(::GetProcAddress(hMapi32, funcName));
}

bool win_mail_login(const std::string& name, const std::string& pwd)
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

MapiMessage create_win_mail_msg(const std::string& recipName, const std::string& recipAddr, const std::string& subject, const std::string& text)
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
	msg.lpszSubject = "Video feed stopped";
	msg.lpszNoteText = "ALARM!";
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