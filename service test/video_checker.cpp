#include "stdafx.h"
#include "video_checker.h"

#include <iostream>
#include <fstream>
#include <dirent.h>

namespace VIDEO_CHECKER {
	LPCSTR VIDEO_CAPTURE_CHECKER_TITLE = "Video capture checker";
	std::string VIDEO_CAPTURE_PATH;
	const std::string DEFAULT_CAPTURE_PATH = "E:\\Video\\TestFolder";
	std::string LOG_FILE_PATH;
	std::string LOG_FILE_NAME;
	const int DEFAULT_NUM_OF_CAMS = 5;
	int NUM_OF_CAMS;
	SYSTEMTIME LastCheckTime{ 0, 0, 0, 0, 0, 0, 0, 0 };
	SYSTEMTIME LastCameraUpdateTime{ 0, 0, 0, 0, 0, 0, 0, 0 };
	SYSTEMTIME LastEmailSentTime;
	const int HOURS_TILL_EMERGENCY_CALL = 3;

	TVideoFileNames VideoFiles;

	void log_error(const std::string& errorText) {
		using std::ofstream;
		ofstream file;
		static const std::string log_error_fn = "E:\\Video\\log_error.txt";
		file.open(log_error_fn, ofstream::app);
		if ((file.rdstate() & ofstream::failbit) != 0) {
			return;
		}
		file << errorText << std::endl;
		file.close();
	}
	void log_new_file_added(int camNum, const std::string& newFileName, const SYSTEMTIME& t) {
		using std::ofstream;
		ofstream file;
		file.open("E:\\Video\\" + LOG_FILE_NAME, ofstream::app);
		if ((file.rdstate() & ofstream::failbit) != 0) {
			return;
		}
		file << "<video>\n";
		file << "\t<camera>" << camNum << "</camera>\n";
		file << "\t<name>" << newFileName << "</name>\n"; 
		file << "\t<time>" << t.wDay << '.' << t.wMonth << '.' << t.wYear << ' ' << t.wHour << ':' << t.wMinute << "</time>\n";
		file << "</video>\n";
		file.close();
	}
	void send_emergency_email() {}
}

//-------------  get_all_filenames_within_folder  -----------------

VSTR get_all_filenames_within_folder(const std::string& folder, const std::string& extension, bool dirs)
{
	using namespace VIDEO_CHECKER;
	static const std::string current_folder{ "." };
	static const std::string parent_folder{ ".." };
	VSTR names;
	std::string sp{ folder + "\\*"};
	if (dirs){
		sp += extension;
	}
	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA(sp.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!dirs && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				names.push_back(fd.cFileName);
			}
			else if (dirs && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
				//if not current dir or parent - add
				std::string fn{ fd.cFileName };
				if (fn != current_folder && fn != parent_folder){
					names.push_back(fn);
				}
			}
		} while (::FindNextFileA(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}


std::string month_or_day_to_string(int val)
{
	std::string sVal = std::to_string(val);
	return (val > 9) ? sVal : std::string{ '0' + sVal };
}

int get_hour_diff(SYSTEMTIME& prev, SYSTEMTIME& next)
{
	auto dateDiff = next.wDay - prev.wDay;
	auto hourDiff = next.wHour - prev.wHour;
	if (!dateDiff) {
		return hourDiff;
	} else {
		return (23 - prev.wHour + next.wHour + 24*(dateDiff-1));
	}
}
