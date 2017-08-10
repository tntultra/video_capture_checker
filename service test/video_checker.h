#ifndef VIDEO_CHECKER_H
#define VIDEO_CHECKER_H

#include "VideoFile.h"

#include <vector>
#include <unordered_set>
#include <windows.h>
#include <boost/date_time/posix_time/posix_time.hpp>

typedef std::vector<std::string> VSTR;

namespace VIDEO_CHECKER {
	extern LPSTR VIDEO_CAPTURE_CHECKER_TITLE;
	extern std::string VIDEO_CAPTURE_PATH;
	extern std::string LOG_FILE_PATH;
	extern std::string LOG_FILE_NAME;
	extern int NUM_OF_CAMS;
	extern const int DEFAULT_NUM_OF_CAMS;
	extern boost::posix_time::ptime LastCheckTime;
	extern boost::posix_time::ptime LastCameraUpdateTime;
	extern boost::posix_time::ptime LastVideoStopEmailSentTime;
	extern boost::posix_time::ptime LastLowSpaceEmailSentTime;
	extern int MINUTES_TILL_EMERGENCY_CALL;
	extern std::string INI_FILE_NAME;
	extern int TIME_BETWEEN_CHECKS;//milliseconds
	
	extern HANDLE SOUND_THREAD;
	extern std::string SOUND_FILE_NAME;

	extern std::ofstream LOG_FILE;

	bool open_log_file();
	void close_log_file();
	void log_error(std::string errorText);
	void log_new_file_added(TVideoFile vFile);
	void send_emergency_email_mapi();
	void send_low_space_email_curl();
	void send_video_stop_email_curl();

	struct TVideoFileNames {
		bool register_new_file (TVideoFile newFile);

		using HASH_TYPE = std::vector<std::unordered_set<std::string>>;
		HASH_TYPE FileNameHash;
	};

	extern TVideoFileNames VideoFiles;
}

bool check_hardDrive_space();
std::string month_or_day_to_string(int val);
VSTR get_all_filenames_within_folder(std::string folder, std::string extension = ".*", bool dirs = false);
bool load_filenames_from_existing_logFile();

#endif //VIDEO_CHECKER_H