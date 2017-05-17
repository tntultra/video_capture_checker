#include "stdafx.h"
#include "video_checker.h"

#include "VideoFile.h"
#include "win_mail.h"

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace VIDEO_CHECKER {
	LPSTR VIDEO_CAPTURE_CHECKER_TITLE = "Video capture checker";
	std::string VIDEO_CAPTURE_PATH;
	std::string LOG_FILE_PATH;
	std::string LOG_FILE_NAME;
	const int DEFAULT_NUM_OF_CAMS = 5;
	int NUM_OF_CAMS = DEFAULT_NUM_OF_CAMS;
	boost::posix_time::ptime LastCheckTime{boost::date_time::not_a_date_time };
	boost::posix_time::ptime LastCameraUpdateTime{ boost::date_time::not_a_date_time };
	boost::posix_time::ptime LastEmailSentTime{ boost::date_time::not_a_date_time };
	int MINUTES_TILL_EMERGENCY_CALL = 180;
	std::string INI_FILE_NAME;
	int TIME_BETWEEN_CHECKS = 60000;

	TVideoFileNames VideoFiles;

	void log_error(std::string errorText) {
		using std::ofstream;
		ofstream file;
		static std::string log_error_fn = LOG_FILE_PATH + "\\log_error.txt";
		file.open(log_error_fn, ofstream::app);
		if ((file.rdstate() & ofstream::failbit) != 0) {
			return;
		}
		file << "[ " << boost::posix_time::second_clock::local_time() << " ] " << errorText << std::endl;
		file.close();
	}

	void log_new_file_added(TVideoFile vFile) {
		using std::ofstream;
		ofstream file;
		file.open(LOG_FILE_PATH + "\\" + LOG_FILE_NAME, ofstream::app);
		if ((file.rdstate() & ofstream::failbit) != 0) {
			return;
		}
		file << vFile;
		file.close();
	}

	void send_emergency_email_mapi()
	{

		bool lowOnSpace = check_hardDrive_space();

		using namespace std;
		//open ini file
		//get credentials from ini file
		boost::property_tree::ptree pt;
		boost::property_tree::ini_parser::read_ini(INI_FILE_NAME, pt);
		std::string name{ pt.get<std::string>("Email.LoginName") };
		std::string password{ pt.get<std::string>("Email.LoginPassword") };
		std::string recipName{ pt.get<std::string>("Email.RecipientName") };
		std::string recipAddress{ pt.get<std::string>("Email.RecipientAddress") };
		std::string subject{ pt.get<std::string>("Email.Subject") };
		std::string text{ pt.get<std::string>("Email.Text") };
		if (lowOnSpace) {
			text += "\nHard drive is low on space! Check log_error file for details.";
		}

		auto msg = create_win_mail_msg(recipName, recipAddress, subject, text);
		win_mail_login(name, password);
		win_mail_send(msg);
		win_mail_logoff();
		delete[] msg.lpRecips;
	}

	void send_video_stop_email_curl()
	{
		using namespace std;
		auto lowOnSpace = check_hardDrive_space();
		//open ini file
		//get credentials from ini file
		boost::property_tree::ptree pt;
		boost::property_tree::ini_parser::read_ini(INI_FILE_NAME, pt);
		auto name{ pt.get<std::string>("Email.LoginName") };
		auto password{ pt.get<std::string>("Email.LoginPassword") };
		auto recipName{ pt.get<std::string>("Email.RecipientName") };
		auto recipAddress{ pt.get<std::string>("Email.RecipientAddress") };
		auto ccs{ pt.get<std::string>("Email.CC") };
		auto smtp{ pt.get<std::string>("Email.SMTP") };

		auto parseCcs = [&ccs] () {
			if (!ccs[0]) {
				return VSTR{};
			}
			VSTR parsedCcs;
			boost::split(parsedCcs, ccs, boost::is_any_of(","));
			return parsedCcs;
		};


		auto subject{ pt.get<std::string>("Email.Subject") };
		auto text{ pt.get<std::string>("Email.Text") };
		if (lowOnSpace) {
			text += "\nHard drive is low on space! Check log_error file for details.";
		}

		CURL_email newEmail{ recipAddress , name , "Ivan" , subject , text, parseCcs() };
		newEmail.send(smtp, name, password);
	}

	bool TVideoFileNames::register_new_file (TVideoFile newFile)
	{
		if (newFile.CameraNum > FileNameHash.size ()) {
			return false;
		}
		auto alrdyExistingName = FileNameHash[newFile.CameraNum-1].find (newFile.Name);
		if (alrdyExistingName == FileNameHash[newFile.CameraNum-1].end ()) {
			FileNameHash[newFile.CameraNum-1].insert (newFile.Name);
			return true;
		}
		return false;
	}
}

//-------------  get_all_filenames_within_folder  -----------------

VSTR get_all_filenames_within_folder(std::string folder, std::string extension, bool dirs)
{
	using namespace VIDEO_CHECKER;
	static std::string current_folder{ "." };
	static std::string parent_folder{ ".." };
	VSTR names;
	std::string sp{ folder + "\\*"};
	if (!dirs){
		sp += extension;
	}
	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA(sp.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!dirs && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			} else if (dirs && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				//if not current dir or parent - add
				std::string fn{fd.cFileName};
				if (fn != current_folder && fn != parent_folder) {
					names.push_back(fn);
				}
			}
		} while (::FindNextFileA(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

bool load_filenames_from_existing_logFile()
{
	using std::ifstream;
	ifstream file;
	file.open(VIDEO_CHECKER::LOG_FILE_PATH + "\\" + VIDEO_CHECKER::LOG_FILE_NAME, ifstream::in);
	if ((file.rdstate() & ifstream::failbit) != 0) {
		return false;
	}
	while (!file.eof()) {
		TVideoFile newVFile;
		file >> newVFile;
		VIDEO_CHECKER::VideoFiles.register_new_file(newVFile);
	}
	return false;
}

bool check_hardDrive_space ()
{
	using namespace std;
	using namespace boost::filesystem;

	auto hardDrivePath = VIDEO_CHECKER::VIDEO_CAPTURE_PATH.substr(0, 3);
	path p{ hardDrivePath };
	try
	{
		auto spaceOnHardDriveInfo = space(p);
		boost::property_tree::ptree pt;
		boost::property_tree::ini_parser::read_ini(VIDEO_CHECKER::INI_FILE_NAME, pt);
		auto spaceForAlertMB = stoi(pt.get<std::string>("Main.FreeSpaceAlertMegabytes"));
		auto freeSpaceMB = spaceOnHardDriveInfo.free / (1024 * 1024);
		if (freeSpaceMB < spaceForAlertMB) {
			VIDEO_CHECKER::log_error("Free space is running low on " + hardDrivePath + ". Only " + std::to_string(freeSpaceMB) + "MB is available.");
			return true;
		}
	}
	catch (filesystem_error &e)
	{
		VIDEO_CHECKER::log_error(e.what());
	}
	return false;
}

std::string month_or_day_to_string(int val)
{
	std::string sVal = std::to_string(val);
	return (val > 9) ? sVal : std::string{ '0' + sVal };
}
