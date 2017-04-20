// service test.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "video_checker.h"
#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

static
void signal_handler(int signum)
{
	using namespace VIDEO_CHECKER;
	switch (signum) {
	case SIGTERM:
		log_error("SIGTERM\n");
		exit(0);
	case SIGSEGV:
		log_error("SIGSEGV!\n");
		exit(0);
	case SIGFPE:
		log_error("SIGFPE\n");
		exit(0);
		// SIGHUP ��� ������ ���
	default:
		break;
	}
}

SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE service_status_handle;

void WINAPI ServiceCtrlHandler(DWORD request)
{
	switch (request) {
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

void perform_check(const SYSTEMTIME& t)
{
	using namespace VIDEO_CHECKER;
	std::string year = std::to_string(t.wYear);
	std::string month = month_or_day_to_string(t.wMonth);
	std::string day = month_or_day_to_string(t.wDay);
	
	//create log file and get folder by current time
	char underscore{ '_' };
	LOG_FILE_NAME = std::string{ "log" } +underscore + year + underscore + day + underscore + month + ".txt";
	std::string folderName = VIDEO_CAPTURE_PATH + "\\" + year + "-" + day + "-" + month;

	//get all cam folders in folder
	//check for new filenames in cam folders

	auto camFolders = get_all_filenames_within_folder(folderName, {}, true);
	NUM_OF_CAMS = max(static_cast<int>(camFolders.size()), NUM_OF_CAMS);

	//VideoFiles.FileNameHash.reserve(numOfCams);

	if (!VideoFiles.FileNameHash.size()){
		VideoFiles.FileNameHash = TVideoFileNames::HASH_TYPE{ 5, {} };
	}
	else if (NUM_OF_CAMS > static_cast<int>(VideoFiles.FileNameHash.size())){
		VideoFiles.FileNameHash.resize(NUM_OF_CAMS);
	}

	for (size_t i = 0; i < camFolders.size(); ++i){
		//auto folderNameForCam = folderName + "\\" + std::to_string(i) + "\\";
		auto folderNameForCam = folderName + "\\" + camFolders[i];
		int camNum;
		try {
			camNum = stoi(camFolders[i]) - 1;
		} catch (const std::invalid_argument&) {
			log_error(std::string{ "�������� ����� ������ �� �������� ������! (\" " } +folderNameForCam + "\")");
			continue;
		}
		//log_error("folder name for cam = " + folderNameForCam);
		auto allFileNames = get_all_filenames_within_folder(folderNameForCam);
		for (auto fn : allFileNames) {
			if (VideoFiles.register_new_name(camNum, fn)){
				log_new_file_added(camNum + 1, fn, t);
				LastCameraUpdateTime = t;
			}
		}
	}
	LastCheckTime = t;
}

int main_video_file_check_func(int argc, char *argv[])
{
	using namespace VIDEO_CHECKER;
	signal(SIGTERM, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGFPE, signal_handler);

	{//load main config settings
		if (argc >= 3) {
			INI_FILE_NAME = argv[2];
		} else {
			INI_FILE_NAME = "E:\\Video\\config.ini";
		}
		try {
			boost::property_tree::ptree pt;
			boost::property_tree::ini_parser::read_ini (INI_FILE_NAME, pt);
			VIDEO_CAPTURE_PATH = pt.get<std::string> ("Main.CameraDayFeedRootFolder");
			LOG_FILE_PATH = pt.get<std::string> ("Main.LogFolder");
			NUM_OF_CAMS = pt.get<int> ("Main.NumberOfCameras");
			TIME_BETWEEN_CHECKS = pt.get<int> ("Main.MinutesBetweenChecks") * 60000;
			HOURS_TILL_EMERGENCY_CALL = pt.get<int> ("Main.HoursBetweenAlertEmails");
		}
		catch (const boost::property_tree::ptree_bad_path& err) {
			log_error(std::string{ "����������� ������ � config.ini!\n" } +err.what());
			return 1;
		}	catch (...) {
			log_error ("�� ���� ������� config.ini!");
			return 1;
		}
	}

	//get current time
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	if (!LastCheckTime.wYear){//initial check
		LastCheckTime = lt;
		LastCameraUpdateTime = lt;
	}
	if ((LastCheckTime.wDay < lt.wDay) || (LastCheckTime.wDay == lt.wDay + 1)){
		perform_check(LastCheckTime);
		//reset camera files vector
		VideoFiles.FileNameHash.clear();
	}
	perform_check(lt);

	//check for emergency
	auto hCamDiff = get_hour_diff(lt, LastCameraUpdateTime);
	auto hEmailSendDiff = get_hour_diff(lt, LastEmailSentTime);
	if (hCamDiff >= HOURS_TILL_EMERGENCY_CALL && hEmailSendDiff >= HOURS_TILL_EMERGENCY_CALL) {
		LastEmailSentTime = lt;
		send_emergency_email();
	}
	//wait for another refresh
	Sleep(TIME_BETWEEN_CHECKS);
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

	if (service_status_handle == (SERVICE_STATUS_HANDLE)0) {
		return -1;
	}

	service_status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(service_status_handle, &service_status);

	setlocale(LC_ALL, "");
	while (service_status.dwCurrentState == SERVICE_RUNNING) {
		main_video_file_check_func(argc, argv);
	}

	return 0;
}

//run cmd as  admin and type
//./sm.exe create VideoChecker binPath= "E:\Video\VideoChecker.exe -s E:\Video\config.ini"

int main(int argc, char *argv[])
{
	if (argc >= 2 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--service") == 0)) {
		SERVICE_TABLE_ENTRYA service_table[2];
		service_table[0].lpServiceName = VIDEO_CHECKER::VIDEO_CAPTURE_CHECKER_TITLE;
		service_table[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)ServiceMain;
		service_table[1].lpServiceName = nullptr;
		service_table[1].lpServiceProc = nullptr;
		StartServiceCtrlDispatcherA(service_table);
	} else {
		return main_video_file_check_func(argc, argv);
	}
}

