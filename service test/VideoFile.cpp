#include "stdafx.h"
#include "VideoFile.h"
#include "video_checker.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/optional.hpp>

std::ostream & operator<<(std::ostream &os, TVideoFile vFile)
{
	os << "<video>\n";
	os << "\t<name>" << vFile.Name << "</name>\n";
	os << "\t<time>" << vFile.TimeAdded << "</time>\n";
	os << "</video>\n";
	return os;
}

std::istream& operator>> (std::istream& is, TVideoFile& vFile)
{
	/*using namespace boost::spirit;
	using std::string;
	
	std::istream_iterator<char> eos;              // end-of-istream iterator
	std::istream_iterator<char> iit(is);

	using TagValueNameType = std::vector<char>;

	auto openingVideoTag = lit("<video>");
	auto closingVideoTag = lit("</video>");
	auto openingCameraTag = lit("<camera>");
	auto closingCameraTag = lit("</camera>");
	auto openingCameraNameTag = lit("<name>");
	auto closingCameraNameTag = lit("</name>");
	auto openingTimeTag = lit("<time>");
	auto closingTimeTag = lit("</time>");

	auto vf = openingVideoTag >>
		openingCameraTag >> +qi::char_ >> closingCameraTag >>
		openingCameraNameTag >> +qi::char_ >> closingCameraNameTag >>
		openingTimeTag >> +qi::char_ >> closingTimeTag >>
		closingVideoTag;

	std::vector<TagValueNameType> allTagValues;
	if (qi::phrase_parse (iit, eos, vf, allTagValues))	{
		for (auto& value : allTagValues) {
			
		}
	}*/

	/*if (temp != "<video>") {
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
