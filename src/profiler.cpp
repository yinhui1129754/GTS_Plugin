#include "profiler.hpp"
#include "Config.hpp"
#include "data/time.hpp"
#include <chrono>

namespace Gts {
	Profiler::Profiler(std::string_view name) : name(std::string(name)) {

	}

	void Profiler::Start()
	{
		if (!this->running) {
			m_beg = Clock::now();
			this->running = true;
		}
	}

	void Profiler::Stop()
	{
		if (this->running) {
			this->elapsed += RunningTime();
			this->running = false;
		}
	}

	void Profiler::Reset()
	{
		this->elapsed = 0.0;
	}

	double Profiler::Elapsed() {
		if (this->IsRunning()) {
			return this->elapsed + this->RunningTime();
		} else {
			return this->elapsed;
		}
	}

	bool Profiler::IsRunning() {
		return this->running;
	}

	double Profiler::RunningTime() {
		if (this->running) {
			return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
		} else {
			return 0;
		}
	}

	std::string Profiler::GetName() {
		return this->name;
	}

	ProfilerHandle::ProfilerHandle(std::string_view name) : name(std::string(name)) {
		Profilers::Start(name);
	}

	ProfilerHandle::~ProfilerHandle() {
		Profilers::Stop(this->name);
	}

	ProfilerHandle Profilers::Profile(std::string_view name) {
		return ProfilerHandle(name);
	}

	void Profilers::Start(std::string_view name) {
		if (Config::GetSingleton().GetDebug().ShouldProfile()) {
			auto& me = Profilers::GetSingleton();
			auto key = std::string(name);
			me.profilers.try_emplace(key, name);
			me.profilers.at(key).Start();
			if (me.AnyRunning()) {
				me.totalTime.Start();
			}
		}
	}

	void Profilers::Stop(std::string_view name) {
		if (Config::GetSingleton().GetDebug().ShouldProfile()) {
			auto& me = Profilers::GetSingleton();
			auto key = std::string(name);
			me.profilers.try_emplace(key, name);
			me.profilers.at(key).Stop();
			if (!me.AnyRunning()) {
				me.totalTime.Stop();
			}
		}
	}

	bool Profilers::AnyRunning() {
		for (auto& [key, profiler]: this->profilers) {
			if (profiler.IsRunning()) {
				return true;
			}
		}
		return false;
	}

	void Profilers::Report() {
		for (auto& [name, profiler]: Profilers::GetSingleton().profilers) {
			if (profiler.IsRunning()) {
				log::warn("The profiler {} is still running", name);
			}
		}
		std::string report = "Reporting Profilers:";
		report += std::format("\n|{:20}|", "Name");
		report += std::format("{:15s}|",                        "Seconds");
		report += std::format("{:15s}|",                        "% OurCode");
		report += std::format("{:15s}|",                        "s per frame");
		report += std::format("{:15s}|",                        "% of frame");
		report += "\n------------------------------------------------------------------------------------------------";

		static std::uint64_t last_report_frame = 0;
		static double last_report_time = 0.0;
		std::uint64_t current_report_frame = Time::WorldTimeElapsed();
		double current_report_time = Time::WorldTimeElapsed();
		double total_time = current_report_time - last_report_time;

		double total = Profilers::GetSingleton().totalTime.Elapsed();
		for (auto& [name, profiler]: Profilers::GetSingleton().profilers) {
			double elapsed = profiler.Elapsed();
			double spf = elapsed / (current_report_frame - last_report_frame);
			double time_percent = elapsed/total_time*100;
			std::string shortenedName = name;
			if (shortenedName.length() > 19) {
				shortenedName = shortenedName.substr(0, 18) + "…";
			}
			report += std::format("\n {:20}:					{:15.3f}|{:14.1f}%|{:15.3f}|{:14.3f}%", shortenedName, elapsed, elapsed*100.0/total, spf, time_percent);
			profiler.Reset();
		}
		log::info("{}", report);

		Profilers::GetSingleton().totalTime.Reset();

		last_report_frame = current_report_frame;
		last_report_time = current_report_time;
	}

	Profilers& Profilers::GetSingleton() {
		static Profilers instance;
		return instance;
	}
}
