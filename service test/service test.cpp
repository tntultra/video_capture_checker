// service test.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "video_checker.h"

static
void signal_handler(int signum)
{
	using namespace VIDEO_CHECKER;
	switch (signum)
	{
	case SIGTERM:
		log_error("Шит хэппенс\n");
		exit(0);
	case SIGSEGV:
		log_error("Пиздец!\n");
		exit(0);
	case SIGFPE:
		log_error("На ноль поделил, лалка\n");
		exit(0);
		// SIGHUP под виндой нет
	default:
		break;
	}
}

SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE service_status_handle;

void WINAPI ServiceCtrlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		service_status.dwWin32ExitCode = 0;
		service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(service_status_handle, &service_status);
		return;
	default:
		break;
	}
	SetServiceStatus(service_status_handle, &service_status);
	return;
}

int main_video_file_check_func(int argc, char *argv[])
{
	using namespace VIDEO_CHECKER;
	signal(SIGTERM, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGFPE, signal_handler);

	//get current time
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	std::string year = std::to_string(lt.wYear);
	std::string month = month_or_day_to_string(lt.wMonth);
	std::string day = month_or_day_to_string(lt.wDay);

	//get folder by current time
	if (argc >= 3) {
		VIDEO_CAPTURE_PATH = argv[2];
	}
	else {
		VIDEO_CAPTURE_PATH = DEFAULT_CAPTURE_PATH;
	}

	//get user's appdata folder to create log files
	LOG_FILE_PATH = VIDEO_CAPTURE_PATH;
	//log_error("log file path = " + LOG_FILE_PATH);
	char underscore{ '_' };
	LOG_FILE_NAME = std::string{ "log" } +underscore + year + underscore + day + underscore + month + ".txt";

	std::string folderName = VIDEO_CAPTURE_PATH + "\\" + year + "-" + day + "-" + month;
	//log_error("log file name = " + LOG_FILE_NAME);

	//get all filenames in folder
	//check for new filenames
	auto numOfCams = (argc >= 4) ? std::stoi(argv[3]) : DEFAULT_NUM_OF_CAMS;
	//VideoFiles.FileNameHash.reserve(numOfCams);

	if (!VideoFiles.FileNameHash.size()){
		log_error("initial vector reserve");
		VideoFiles.FileNameHash = TVideoFileNames::HASH_TYPE{ 5, {} };
	}
	else if (numOfCams > (int)VideoFiles.FileNameHash.size()){
		log_error("vector resize");
		VideoFiles.FileNameHash.resize(numOfCams);
	}	

	for (auto i = 1; i <= numOfCams; ++i){
		auto folderNameForCam = folderName + "\\" + std::to_string(i) + "\\";
		//log_error("folder name for cam = " + folderNameForCam);
		auto allFileNames = get_all_filenames_within_folder(folderNameForCam);
		for (auto fn : allFileNames) {
			//
			if (VideoFiles.register_new_name(i-1, fn)){
				log_error("registering new name: " + fn);
				log_new_file_added(i, fn, lt);
				LastUpdateTime = lt;
			}
		}
	}
	//check for emergency
	auto dateDiff = lt.wDay - LastUpdateTime.wDay;
	auto hourDiff = lt.wHour - LastUpdateTime.wHour;
	if ((!dateDiff && hourDiff > HOURS_TILL_EMERGENCY_CALL) ||
		((dateDiff = 1) && (23 - LastUpdateTime.wHour + lt.wHour > HOURS_TILL_EMERGENCY_CALL)) ||
		dateDiff > 1) {
		send_emergency_email();
	}

	return 0;
}

int WINAPI ServiceMain(int argc, char *argv[])
{
	service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwCurrentState = SERVICE_START_PENDING;
	service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	service_status.dwWin32ExitCode = 0;
	service_status.dwServiceSpecificExitCode = 0;
	service_status.dwCheckPoint = 0;
	service_status.dwWaitHint = 0;

	service_status_handle = RegisterServiceCtrlHandlerA(VIDEO_CHECKER::VIDEO_CAPTURE_CHECKER_TITLE, (LPHANDLER_FUNCTION)ServiceCtrlHandler);

	if (service_status_handle == (SERVICE_STATUS_HANDLE)0)
	{
		return -1;
	}

	service_status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(service_status_handle, &service_status);

	setlocale(LC_ALL, "");
	while (service_status.dwCurrentState == SERVICE_RUNNING)
	{
		main_video_file_check_func(argc, argv);
		//wait for another refresh
		Sleep(30000);
	}

	return 0;
}

//run as 
//C:\>sm.exe create CamChecker start=auto DisplayName=CamChecker binPath="D:\Video\CamChecker.exe"

int main(int argc, char *argv[])
{
	if (argc >= 2 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--service") == 0))
	{
		SERVICE_TABLE_ENTRYA service_table[2];
		service_table[0].lpServiceName = (LPSTR)VIDEO_CHECKER::VIDEO_CAPTURE_CHECKER_TITLE;
		service_table[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)ServiceMain;
		service_table[1].lpServiceName = NULL;
		service_table[1].lpServiceProc = NULL;
		StartServiceCtrlDispatcherA(service_table);
	}
	else
	{
		return main_video_file_check_func(argc, argv);
	}
}

