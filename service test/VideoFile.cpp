#include "stdafx.h"
#include "VideoFile.h"
#include "video_checker.h"

#include <boost/spirit/home/qi.hpp>

std::ostream & operator<<(std::ostream &os, TVideoFile vFile)
{
	os << "<video>\n";
	os << "\t<camera>" << (vFile.CameraNum) << "</camera>\n";
	os << "\t<name>" << vFile.Name << "</name>\n";
	os << "\t<time>" << vFile.TimeAdded << "</time>\n";
	os << "</video>\n";
	return os;
}

std::istream& operator>> (std::istream& is, TVideoFile& vFile)
{
	/*using namespace boost::spirit::qi;
	using boost::spirit::ascii::space;
	using boost::spirit::qi::_1;
	using boost::phoenix::ref;
	std::string temp, open, value, close;
	getline(is,temp);
	if (temp != "<video>") {
		VIDEO_CHECKER::log_error("Cannot read video file - bad opening tag (" + temp + ")");
		return is;
	}
	for (int i = 0; i < 3; ++i) {
		getline(is, temp);
		bool result = phrase_parse(begin(temp), end(temp),
			*space >> char_('<') >> string[open] >> char_('>') >> string[value] >> char_('<') >> string[close] >> char_('/>'), space);
		if (!result) {
			VIDEO_CHECKER::log_error("Cannot read video file - cannot parse (" + temp + ")");
			return is;
		}
	}

	if (temp != "</video>") {
		VIDEO_CHECKER::log_error("Cannot read video file - bad closing tag (" + temp + ")");
		return is;
	}*/
	return is;
}
