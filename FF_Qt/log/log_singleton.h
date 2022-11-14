#ifndef __LOG_TEMPLATE_H__
#define __LOG_TEMPLATE_H__
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <type_traits>
#include "QString"
#include "QDateTime"

template< typename DerivedT, typename LogLevelT >
class LogSingletonWithLevel : public LogSingletonInterface
{
public:
	static DerivedT* GetInstance()
	{
		if (instance_ == nullptr)
		{
			std::call_once(once_flag_, [] { instance_ = new DerivedT; });
		}

		return instance_;
	}

	virtual ~LogSingletonWithLevel() = default;

	inline void			SetLogFilePath(const std::wstring& file_path) { log_file_impl_->SetLogFilePath(file_path); }
	inline std::wstring	GetLogFilePath() const { return log_file_impl_->GetLogFilePath(); }
	inline void			SetLogLevel(LogLevelT lv) { log_file_impl_->SetLogLevel(lv); }
	inline void			RemoveHalfIfLongerThan(long max) { log_file_impl_->RemoveHalfIfLongerThan(max); }
	inline void			SetServerLogPrintCallback(ServerLogPrintCb cb) { log_file_impl_->SetServerLogPrintCallback(cb); }

	virtual void WriteLog(LogLevelT lv, const std::wstring& log, LogServerLevel server_level)
	{
#ifdef _DEBUG
		if (!log_debug_filter_ || log_debug_filter_(lv, log))
		{
			this->OutputOnDebugWindow(lv, log);
		}
#endif

		if (this->CanLevelPass(lv))
		{
			log_file_impl_->WriteLog(stringtool::UTF16ToUTF8(log));
		}

		log_file_impl_->ServerLogPrintCall((int)server_level, log);
	}

	virtual void GetLogContent(std::string& content, std::wstring file_path = L"") override { return log_file_impl_->GetLogContent(content, file_path); }
	template<typename FilterT> void SetLogDebugFilter(FilterT&& filter) { log_debug_filter_ = std::forward<FilterT>(filter); }

protected:
	LogSingletonWithLevel() : log_file_impl_(new LogFileWithLevel)
	{
		static_assert(std::is_integral<LogLevelT>::value || std::is_enum<LogLevelT>::value, "LogLevelT must be integral or enum");
	}

	LogSingletonWithLevel(const LogSingletonWithLevel&) = delete;
	LogSingletonWithLevel& operator=(const LogSingletonWithLevel&) = delete;

	virtual bool CanLevelPass(LogLevelT lv) { return (lv >= log_file_impl_->GetLogLevel<LogLevelT>()); }
	virtual void OutputOnDebugWindow(LogLevelT, const std::wstring& log) { fwprintf_s(stdout, L"%s", log.c_str()); }

protected:
	std::unique_ptr<LogFileWithLevel>						log_file_impl_;

	std::function< bool(LogLevelT, const std::wstring&) >	log_debug_filter_;

private:
	static std::once_flag	once_flag_;
	static DerivedT* instance_;
};

template< typename DerivedT, typename LogLevelT >
std::once_flag LogSingletonWithLevel<DerivedT, LogLevelT>::once_flag_;

template< typename DerivedT, typename LogLevelT >
DerivedT* LogSingletonWithLevel<DerivedT, LogLevelT>::instance_ = nullptr;

/**
* HelperT must have 2 static functions:
* 1: std::wstring StringOfLevel(LogLevelT)
* 2：void WriteLog(LogLevelT, const std::wstring&)
*/
template< typename HelperT, typename LogLevelT >
class LogItemWithLevel
{
public:
	LogItemWithLevel(const char* file, long line, LogLevelT lv)
		: index_(0)
		, level_(lv)
	{
		static_assert(std::is_integral<LogLevelT>::value || std::is_enum<LogLevelT>::value, "LogLevelT must be integral or enum");

		auto str = stringtool::UTF8ToUTF16(file);
		std::wstring file_name;
		filepathtool::FilePathApartFileName(str, file_name);
		auto date_time = QDateTime::currentDateTime();
		time_ = date_time.toString("[yyyy-MM-dd hh:mm:ss ").toStdWString();
		file_line_ = QString(QString::fromStdWString(file_name.empty() ? str.c_str() : file_name.c_str()) + QString(":") + QString::number(line) + QString(" ")).toStdWString();
	}

	~LogItemWithLevel()
	{
		std::wstring lv = HelperT::StringOfLevel(level_);

		lv.append(L"] ");

		if (string_.empty())
		{
			string_ = fmt_;
		}
		else if (!fmt_.empty())
		{
			string_.append(fmt_);
		}

		std::wstring log = time_ + file_line_ + lv + string_ + L"\n";

		HelperT::WriteLog(level_, std::move(log));
	}

	LogItemWithLevel& VLog(LogLevelT lv, const std::wstring& fmt)
	{
		level_ = lv;
		fmt_ = fmt;

		return *this;
	}

	LogItemWithLevel& VLog(LogLevelT lv, const std::string& fmt)
	{
		return VLog(lv, stringtool::UTF8ToUTF16(fmt));
	}

	LogItemWithLevel& operator<<(const std::wstring& str)
	{
		return this->append(str);
	}

	LogItemWithLevel& operator<<(const std::string& str)
	{
		return this->append(stringtool::UTF8ToUTF16(str));
	}

	LogItemWithLevel& operator<<(long long lld)
	{
		return this->append(stringtool::NumberToWString(lld));
	}

	LogItemWithLevel& operator<<(uint64_t llu)
	{
		return this->append(stringtool::NumberToWString(llu));
	}

	LogItemWithLevel& operator<<(uint32_t u)
	{
		return this->append(stringtool::NumberToWString(u));
	}

	LogItemWithLevel& operator<<(unsigned long ul)
	{
		return this->append(stringtool::NumberToWString(ul));
	}

	LogItemWithLevel& operator<<(double lf)
	{
		return this->append(stringtool::NumberToWString(lf));
	}

	LogItemWithLevel& operator<<(int d)
	{
		return this->append(stringtool::NumberToWString(d));
	}

	LogItemWithLevel& operator<<(void* ptr)
	{
		std::wstringstream wss;
		wss << ptr;

		return this->append(wss.str());
	}

	LogItemWithLevel& operator<<(const wchar_t* str)
	{
		return this->append(std::wstring(str));
	}

	LogItemWithLevel& operator<<(const char* str)
	{
		return this->append(stringtool::UTF8ToUTF16(str, strlen(str)));
	}

protected:
	inline LogItemWithLevel& append(const std::wstring& str)
	{
		int len = 0;
		size_t pos = NextArg(len);
		if (pos == fmt_.npos)
		{
			fmt_.append(str);
		}
		else
		{
			string_.append(fmt_.substr(0, pos));
			string_.append(str);

			fmt_.erase(0, pos + len);

			index_ += 1;
		}

		return *this;
	}

	inline size_t NextArg(int& len) // 返回"{x}"的索引，len代表"{x}"的长度
	{
		auto pos = fmt_.npos;

		if (!fmt_.empty())
		{
			QString str;
			str.sprintf("{%d}", index_);
			pos = fmt_.find(str.toStdWString());

			len = str.size();
		}

		return pos;
	}

protected:
	std::wstring	fmt_;
	std::wstring	string_;
	int				index_;

	std::wstring	time_;
	std::wstring	file_line_;
	LogLevelT		level_;
};

#endif