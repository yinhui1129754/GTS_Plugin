#pragma once

using namespace RE;
using namespace SKSE;

namespace Gts {

	template< typename ... Args >
	void Notify(std::string_view rt_fmt_str, Args&&... args) {
		try {
			DebugNotification(std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			log::info("Could not format notification, check valid format string: {}", e.what());
		}
	}

	template< typename ... Args >
	void PrintMessageBox(std::string_view rt_fmt_str, Args&&... args) {
		try {
			PrintMessageBox(std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			log::info("Could not format notification, check valid format string: {}", e.what());
		}
	}

	template< typename ... Args >
	void Cprint(std::string_view rt_fmt_str, Args&&... args) {
		try {
			ConsoleLog::GetSingleton()->Print("%s", std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			log::info("Could not format console log, check valid format string: {}", e.what());
		}
	}

	bool starts_with(std::string_view arg, std::string_view prefix);

	bool matches(std::string_view str, std::string_view reg);

	std::string str_tolower(std::string s);
	std::string str_toupper(std::string s);

	// courtesy of https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
	void replace_first(
		std::string& s,
		std::string const& toReplace,
		std::string const& replaceWith
		);

	std::string remove_whitespace(std::string s);
}
