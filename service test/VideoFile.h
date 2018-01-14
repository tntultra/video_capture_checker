#ifndef VIDEO_FILE_H
#define VIDEO_FILE_H

#pragma once

#include <string>
#include <iosfwd>
#include <boost/date_time/posix_time/posix_time.hpp>

struct TVideoFile
{
	TVideoFile() = default;
	TVideoFile (std::string newFileName, const boost::posix_time::ptime& t) :
		Name (newFileName), TimeAdded (t)
	{}

	TVideoFile(const TVideoFile&) = default;
	TVideoFile(TVideoFile&&) = default;
	TVideoFile& operator=(const TVideoFile&) = default;
	TVideoFile& operator=(TVideoFile&&) = default;
	~TVideoFile() = default;

	std::string Name;
	boost::posix_time::ptime TimeAdded;
};

std::ostream& operator<< (std::ostream&, TVideoFile);
std::istream& operator>> (std::istream&, TVideoFile&);


#endif