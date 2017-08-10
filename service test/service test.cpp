// service test.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "video_checker.h"
#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include "win_mail.h"

#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")

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
		// SIGHUP под виндой нет
	default:
		break;
	}
}

SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE service_status_handle;

static const DWORD SERVICE_CONTROL_SEND_TEST_MAIL = 128;

void WINAPI ServiceCtrlHandler(DWORD request)
{
	switch (request) {
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		service_status.dwWin32ExitCode = 0;
		service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(service_status_handle, &service_status);
		return;
	case SERVICE_CONTROL_SEND_TEST_MAIL:
		VIDEO_CHECKER::send_video_stop_email_curl();
		return;

	default:
		break;
	}
	SetServiceStatus(service_status_handle, &service_status);
	return;
}

DWORD WINAPI MP3Proc(_In_ LPVOID lpParameter) //lpParameter can be a pointer to a structure that store data that you cannot access outside of this function. You can prepare this structure before `CreateThread` and give it's address in the `lpParameter`
{
	auto data = *reinterpret_cast<std::string*> (lpParameter); //If you call this structure Data, but you can call it whatever you want.
	std::string sendStringText{ "open " + data + " type mpegvideo alias mp3" };
	while (true)
	{
		auto mciRes = mciSendString(sendStringText.c_str(), NULL, 0, NULL);
		//char buffer[256]{};
		//mciGetErrorString(mciRes, buffer, 256);

		auto res = mciSendString("play mp3 from 0 wait", NULL, 0, NULL);
		//mciGetErrorString(res, buffer, 256);

		//Do here what you want to do when the mp3 playback is over
		SuspendThread(GetCurrentThread()); //or the handle of this thread that you keep in a static variable instead
	}
	return 0;
}


void perform_check(const boost::posix_time::ptime& t)
{
	VIDEO_CHECKER::log_error("Started new check at " + to_simple_string(t));

	using namespace VIDEO_CHECKER;
	auto year = std::to_string(t.date().year());
	auto month = month_or_day_to_string(t.date().month());
	auto day = month_or_day_to_string(t.date().day());
	
	//create log file and get folder by current time
	auto underscore{ '_' };
	LOG_FILE_NAME = std::string{ "log" } +underscore + year + underscore + month + underscore + day + ".txt";
	auto folderName = VIDEO_CAPTURE_PATH + "\\" + year + "-" + month + "-" + day;

	//get all cam folders in folder
	//check for new filenames in cam folders

	auto camFolders = get_all_filenames_within_folder(folderName, {}, true);
	NUM_OF_CAMS = max(static_cast<int>(camFolders.size()), NUM_OF_CAMS);

	//VideoFiles.FileNameHash.reserve(numOfCams);

	if (!VideoFiles.FileNameHash.size()){
		using namespace boost::filesystem;
		try {
			file_status s = status(LOG_FILE_NAME);
			if (exists(s)) {
				load_filenames_from_existing_logFile();
			}
		} catch (filesystem_error &e) {
			log_error(e.what());
		}

		VideoFiles.FileNameHash = TVideoFileNames::HASH_TYPE{ 5, {} };
	}
	else if (NUM_OF_CAMS > static_cast<int>(VideoFiles.FileNameHash.size())){
		VideoFiles.FileNameHash.resize(NUM_OF_CAMS);
	}

	for (size_t i = 0; i < camFolders.size(); ++i){
		//auto folderNameForCam = folderName + "\\" + std::to_string(i) + "\\";
		auto folderNameForCam = folderName + "\\" + camFolders[i] + "\\rec";
		int camNum;
		try {
			camNum = stoi(camFolders[i]);
		} catch (const std::invalid_argument&) {
			log_error(std::string{ "Название папки камеры не является числом! (\" " } +folderNameForCam + "\")");
			continue;
		}
		//log_error("camera folder name:" + folderNameForCam);
		//log_error("camera num:" + std::to_string(camNum));

		auto allFileNames = get_all_filenames_within_folder(folderNameForCam);
		for (auto& fn : allFileNames) {
			TVideoFile vFile{ camNum, fn, t };
			//log_error("file:" + fn);
			if (VideoFiles.register_new_file(vFile)) {
				//log_error("new file!");
				log_new_file_added(vFile);
				LastCameraUpdateTime = t;
			}
		}
	}
	LastCheckTime = t;
	log_error("check performed correctly");
}

int main_video_file_check_func()
{
	using namespace VIDEO_CHECKER;
	signal(SIGTERM, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGFPE, signal_handler);

	{//load main config settings
		if (!INI_FILE_NAME[0]) {
			using namespace boost::filesystem;
			path full_path(current_path());
			INI_FILE_NAME = std::string{ full_path.string() } +"\\config.ini";
			log_error("Ini file: " + INI_FILE_NAME);
		}
		try {
			boost::property_tree::ptree pt;
			boost::property_tree::ini_parser::read_ini (INI_FILE_NAME, pt);
			VIDEO_CAPTURE_PATH = pt.get<std::string> ("Main.CameraDayFeedRootFolder");
			LOG_FILE_PATH = pt.get<std::string> ("Main.LogFolder");
			NUM_OF_CAMS = pt.get<int> ("Main.NumberOfCameras");
			TIME_BETWEEN_CHECKS = pt.get<int> ("Main.MinutesBetweenChecks") * 60000;
			MINUTES_TILL_EMERGENCY_CALL = pt.get<int> ("Main.MinutesBetweenAlertEmails");
			SOUND_FILE_NAME = pt.get<std::string>("Main.SoundFilePath");
		}
		catch (const boost::property_tree::ptree_bad_path& err) {
			log_error(std::string{ "Отсутствует строка в config.ini!\n" } +err.what());
			return 1;
		}	catch (...) {
			log_error ("Не могу открыть config.ini!" + INI_FILE_NAME);
			return 1;
		}
	}

	//sound thread
	if (SOUND_THREAD == NULL) {
		SOUND_THREAD = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MP3Proc,       // thread function name
			&SOUND_FILE_NAME,          // argument to thread function 
			CREATE_SUSPENDED,                      // use default creation flags 
			NULL);
	}


	VIDEO_CHECKER::open_log_file();
	//get current time
	using namespace boost::posix_time;
	using namespace boost::gregorian;
	auto localTime = second_clock::local_time();

	if (LastCheckTime.is_not_a_date_time()){//initial check
		LastCheckTime = localTime;
		LastCameraUpdateTime = localTime;
	}

	auto today = localTime.date(); //Get the date part out of the time
	auto today_start(today); //midnight 

	if (LastCheckTime.date() < today_start){
		perform_check(LastCheckTime);
		//reset camera files vector
		VideoFiles.FileNameHash.clear();
	}
	perform_check(localTime);
	
	log_error("localTime=\"" + to_simple_string(localTime) + "\"");
	log_error("LastCameraUpdateTime=\"" + to_simple_string(LastCameraUpdateTime) + "\"");
	log_error("LastVideoStopEmailSentTime=\"" + to_simple_string(LastVideoStopEmailSentTime) + "\"");
	log_error("LastLowSpaceEmailSentTime=\"" + to_simple_string(LastLowSpaceEmailSentTime) + "\"");
	//check for emergency
	if ((LastCameraUpdateTime + minutes(MINUTES_TILL_EMERGENCY_CALL)) < localTime) {
		if (LastVideoStopEmailSentTime.is_not_a_date_time() || (LastVideoStopEmailSentTime + minutes(MINUTES_TILL_EMERGENCY_CALL)) < localTime) {
			LastVideoStopEmailSentTime = localTime;
			send_video_stop_email_curl();
			ResumeThread(SOUND_THREAD);
		}
	}

	if (check_hardDrive_space()) {
		if (LastLowSpaceEmailSentTime.is_not_a_date_time() || (LastLowSpaceEmailSentTime + minutes(MINUTES_TILL_EMERGENCY_CALL)) < localTime) {
			LastLowSpaceEmailSentTime = localTime;
			send_low_space_email_curl();
		}
	}
	VIDEO_CHECKER::close_log_file();
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
		main_video_file_check_func();
		Sleep(VIDEO_CHECKER::TIME_BETWEEN_CHECKS);
	}

	return 0;
}

//run cmd as  admin and type
//./sc.exe create VideoChecker binPath= "E:\Video\VideoChecker.exe -s E:\Video\config.ini"

int main(int argc, char *argv[])
{
	if (argc >= 3) {
		VIDEO_CHECKER::INI_FILE_NAME = argv[2];
	}
	if (argc >= 2 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--service") == 0)) {
		SERVICE_TABLE_ENTRYA service_table[2];
		service_table[0].lpServiceName = VIDEO_CHECKER::VIDEO_CAPTURE_CHECKER_TITLE;
		service_table[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)ServiceMain;
		service_table[1].lpServiceName = nullptr;
		service_table[1].lpServiceProc = nullptr;
		StartServiceCtrlDispatcherA(service_table);
	} else {
		return main_video_file_check_func();
	}
}

