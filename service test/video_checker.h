#ifndef VIDEO_CHECKER_H
#define VIDEO_CHECKER_H

#include <vector>
#include <unordered_set>
#include <windows.h>

typedef std::vector<std::string> VSTR;

namespace VIDEO_CHECKER {
	extern LPSTR VIDEO_CAPTURE_CHECKER_TITLE;
	extern std::string VIDEO_CAPTURE_PATH;
	extern std::string LOG_FILE_PATH;
	extern std::string LOG_FILE_NAME;
	extern int NUM_OF_CAMS;
	extern const int DEFAULT_NUM_OF_CAMS;
	extern SYSTEMTIME LastCheckTime;
	extern SYSTEMTIME LastCameraUpdateTime;
	extern SYSTEMTIME LastEmailSentTime;
	extern int HOURS_TILL_EMERGENCY_CALL;
	extern std::string INI_FILE_NAME;
	extern int TIME_BETWEEN_CHECKS;//milliseconds

	void log_error(std::string errorText);
	void log_new_file_added(int camNum, std::string newFileName, const SYSTEMTIME& timeAdded);
	void send_emergency_email();

	struct TVideoFileNames {
		bool register_new_name(size_t camNum, std::string fileName){
			if (camNum >= FileNameHash.size()){		
				return false;
			}
			auto alrdyExistingName = FileNameHash[camNum].find(fileName);
			if (alrdyExistingName == FileNameHash[camNum].end()){
				FileNameHash[camNum].insert(fileName);
				return true;
			}
			return false;
		}
		using HASH_TYPE = std::vector<std::unordered_set<std::string>>;
		HASH_TYPE FileNameHash;
	};

	extern TVideoFileNames VideoFiles;
}

int get_hour_diff(SYSTEMTIME& prev, SYSTEMTIME& next);
std::string month_or_day_to_string(int val);
VSTR get_all_filenames_within_folder(std::string folder, std::string extension = ".mp4", bool dirs = false);

#endif //VIDEO_CHECKER_H