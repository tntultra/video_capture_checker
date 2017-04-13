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
	SYSTEMTIME LastUpdateTime;
	const int HOURS_TILL_EMERGENCY_CALL = 4;

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

VSTR get_all_filenames_within_folder(const std::string& folder, const std::string& extension)
{
	using namespace VIDEO_CHECKER;
	VSTR names;
	std::string format = "%s*" + extension;
	CHAR search_path[200];
	sprintf_s(search_path, format.c_str(), folder);
	std::string sp{ search_path };
	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				names.push_back(fd.cFileName);
				//log_error(names.back());
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

//thats actually portable
VSTR get_all_folder_names_in_folder(const std::string& path)
{
	VSTR folderNames;
	DIR *dir = opendir(path.c_str());
	struct dirent *entry = readdir(dir);
	while (entry != NULL)
	{
		if (entry->d_type == DT_DIR){
			folderNames.push_back(std::string{ entry->d_name });
		}
		entry = readdir(dir);
	}
	closedir(dir);
	return folderNames;
}