#include "utils/text.hpp"
#include <algorithm>
#include <cctype>
#include <string>


using namespace RE;
using namespace SKSE;

namespace Gts {

	bool starts_with(std::string_view arg, std::string_view prefix) {
		return arg.compare(0, prefix.size(), prefix);
	}

	bool matches(std::string_view str, std::string_view reg) {
		std::regex the_regex(std::string(reg).c_str());
		return std::regex_match(std::string(str), the_regex);
	}

	std::string str_tolower(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(),
		               [](unsigned char c){
			return std::tolower(c);
		}
		               );
		return s;
	}

	std::string str_toupper(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(),
		               [](unsigned char c){
			return std::toupper(c);
		}
		               );
		return s;
	}

	// courtesy of https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
	void replace_first(
		std::string& s,
		std::string const& toReplace,
		std::string const& replaceWith
		) {
		std::size_t pos = s.find(toReplace);
		if (pos == std::string::npos) {
			return;
		}
		s.replace(pos, toReplace.length(), replaceWith);
	}

	std::string remove_whitespace(std::string s) {
		s.erase(remove(s.begin(),s.end(),' '), s.end());
		return s;
	}

}
