#pragma once

#include "toml.hpp"

namespace Gts {
	class Debug {
		public:
			Debug() = default;

			[[nodiscard]] inline spdlog::level::level_enum GetLogLevel() const noexcept {
				return _logLevel;
			}

			[[nodiscard]] inline spdlog::level::level_enum GetFlushLevel() const noexcept {
				return _flushLevel;
			}

			[[nodiscard]] inline bool ShouldProfile() const noexcept {
				return _shouldProfile;
			}

			Debug(const toml::value& data);

			spdlog::level::level_enum _logLevel{spdlog::level::level_enum::info};
			spdlog::level::level_enum _flushLevel{spdlog::level::level_enum::trace};
			bool _shouldProfile = false;
	};

	class Frame {
		public:
			Frame() = default;

			[[nodiscard]] inline int GetInitDelay() const noexcept {
				return _initDelay;
			}

			[[nodiscard]] inline int GetStep() const noexcept {
				return _step;
			}

			Frame(const toml::value& data);

		private:
			int _step = 0;
			int _initDelay = 0;
	};


	class Tremor {
		public:
			Tremor() = default;

			[[nodiscard]] inline std::string GetMethod() const noexcept {
				return _method;
			}
			[[nodiscard]] inline float GetHalfScale() const noexcept {
				return _halfScale;
			}
			[[nodiscard]] inline float GetPowerAtMin() const noexcept {
				return _powerAtMin;
			}
			[[nodiscard]] inline float GetPowerAtMax() const noexcept {
				return _powerAtMax;
			}
			[[nodiscard]] inline float GetMaxScale() const noexcept {
				return _maxScale;
			}
			[[nodiscard]] inline float GetMinScale() const noexcept {
				return _minScale;
			}

			Tremor(const toml::value& data);

		private:
			std::string _method;
			float _halfScale;
			float _powerAtMin;
			float _powerAtMax;
			float _maxScale;
			float _minScale;
	};

	class Config {
		public:
			Config() = default;

			[[nodiscard]] inline const Debug& GetDebug() const noexcept {
				return _debug;
			}

			[[nodiscard]] inline const Frame& GetFrame() const noexcept {
				return _frame;
			}


			[[nodiscard]] inline const Tremor& GetTremor() const noexcept {
				return _tremor;
			}

			[[nodiscard]] static const Config& GetSingleton() noexcept;

			Config(const toml::value& data);

		private:
			Debug _debug;
			Frame _frame;
			Tremor _tremor;
	};
}
