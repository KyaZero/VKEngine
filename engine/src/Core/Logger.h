#pragma once
#include <Core/Types.h>
#include <vector>
#include <thread>
#include <string>
#include <cassert>
#include <mutex>
#include <queue>

#define VERBOSE_LOG(...) vke::Logger::Get()->Log(::vke::Logger::Level::Verbose, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define INFO_LOG(...) vke::Logger::Get()->Log(::vke::Logger::Level::Info, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define WARNING_LOG(...) vke::Logger::Get()->Log(::vke::Logger::Level::Warning, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define ERROR_LOG(...) vke::Logger::Get()->Log(::vke::Logger::Level::Error, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define FATAL_LOG(...) vke::Logger::Get()->Log(::vke::Logger::Level::Fatal, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); abort();

#ifdef NDEBUG
#define ASSERT_LOG(...) ((void)0)
#else
#define ___ASSERT_LOG(...) vke::Logger::Get()->Log(::vke::Logger::Level::Fatal, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__), abort()
#define ASSERT_LOG(expression, ...) (void)((!!(expression)) || (___ASSERT_LOG("Assertion failed: %s", (#expression " " #__VA_ARGS__)), 0))
#endif

namespace vke
{
	class Logger
	{
	public:
		enum class Level : char
		{
			All,
			Verbose,
			Info,
			Warning,
			Error,
			Fatal,
			Count
		};

		Logger();
		~Logger();

		static void Create(bool multiThreaded = true);
		static void Destroy();
		static Logger* Get();
		static void SetLevel(Level level);
		static void SetPrint(bool shouldPrint);
		static void SetShouldLogToFile(bool shouldLogToFile);

		void Log(Level level, const char* file, u32 line, const char* function, const char* format, ...);
	private:
		static Logger* s_Instance;
		void VerifyLogPath();
		void LogToFile(const std::string& msg);
		static void Update(Logger* instance, std::chrono::duration<double, std::milli> interval);

#ifdef _WIN32
		static auto GetLevelString(Level level);
#else
		static std::string GetLevelString(Level level);
#endif

		struct LogEntry
		{
			std::string filename;
			std::string function;
			std::string msg;
			u32 line;
			Level level;
		};
		static void Print(Logger* instance, const LogEntry& entry);

		std::queue<LogEntry> m_Queue;
		std::mutex m_QueueMutex;
		std::unique_ptr<std::thread> m_Thread;
		std::string m_LogPath;

		char m_Level;
		char* m_Buffer;

		bool m_ShouldPrint;
		bool m_ShouldLogToFile;
		bool m_MultiThreaded;
	};
}