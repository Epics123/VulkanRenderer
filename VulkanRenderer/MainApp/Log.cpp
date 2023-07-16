#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

//#include <memory>

std::shared_ptr<spdlog::logger> Log::sCoreLogger;

void Log::init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$"); // see https://github.com/gabime/spdlog/wiki/3.-Custom-formatting for more custom formatting

	sCoreLogger = spdlog::stdout_color_mt("ENGINE");
	sCoreLogger->set_level(spdlog::level::trace);
}
