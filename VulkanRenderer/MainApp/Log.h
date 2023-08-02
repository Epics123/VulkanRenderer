#pragma once

#include "spdlog/spdlog.h"

class Log
{
public:
	static void init();

	inline static class std::shared_ptr<spdlog::logger>& getCoreLogger() { return sCoreLogger; }

private:
	static class std::shared_ptr<spdlog::logger> sCoreLogger;
};

// core log macros
#define CORE_TRACE(...) ::Log::getCoreLogger()->trace(__VA_ARGS__);
#define CORE_INFO(...)  ::Log::getCoreLogger()->info(__VA_ARGS__);
#define CORE_WARN(...)  ::Log::getCoreLogger()->warn(__VA_ARGS__);
#define CORE_ERROR(...) ::Log::getCoreLogger()->error(__VA_ARGS__);
#define CORE_CRITICAL(...) ::Log::getCoreLogger()->critical(__VA_ARGS__);