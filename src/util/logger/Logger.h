#pragma once

#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "yaml-cpp/yaml.h"

#include "marcos.h"
#include "Mutex.h"
#include "Singleton.h"
#include "util/Clock.h"

#define LogEventGen(level, timestamp) \
  std::make_shared<LogEvent>(level, __FILE__, __LINE__, __FUNCTION__, 0, timestamp)

#define LogEventWrapperGen(pLogger, level, timestamp) \
  std::make_shared<LogEventWrapper>(LogEventGen(level, timestamp), pLogger)

#define LogEventWrapperGen2(pLogger, level) \
  std::make_shared<LogEventWrapper>(LogEventGen(level, Clock::now<T_system_clock>()), pLogger)

#define LOG_FMT_LEVEL(pLogger, level, fmt, ...) \
  do { \
    if(pLogger->getLevel() <= level) \
      LogEventWrapperGen2(pLogger, level)->getEvent()->format(fmt, ##__VA_ARGS__); \
  } while(0)

#define LOG_STREAM(pLogger, level) \
  LogEventWrapperGen2(pLogger, level)->getSS()

#define LOG_FMT_DEBUG_A(pLogger, fmt, ...) \
  LOG_FMT_LEVEL(pLogger, LogLevel::LDEBUG, fmt, ##__VA_ARGS__)
#define LOG_FMT_INFO_A(pLogger, fmt, ...) \
  LOG_FMT_LEVEL(pLogger, LogLevel::LINFO, fmt, ##__VA_ARGS__)
#define LOG_FMT_WARN_A(pLogger, fmt, ...) \
  LOG_FMT_LEVEL(pLogger, LogLevel::LWARN, fmt, ##__VA_ARGS__)
#define LOG_FMT_ERROR_A(pLogger, fmt, ...) \
  LOG_FMT_LEVEL(pLogger, LogLevel::LERROR, fmt, ##__VA_ARGS__)
#define LOG_FMT_FATAL_A(pLogger, fmt, ...) \
  LOG_FMT_LEVEL(pLogger, LogLevel::LFATAL, fmt, ##__VA_ARGS__)

#define LOG_ROOT()       SingleLogManager::instance()->getRoot()
#define GET_LOGGER(name) SingleLogManager::instance()->getLogger(name)

#define LOG_FMT_DEBUG(fmt, ...) \
  LOG_FMT_LEVEL(LOG_ROOT(), LogLevel::LDEBUG, fmt, ##__VA_ARGS__)
#define LOG_FMT_INFO(fmt, ...) \
  LOG_FMT_LEVEL(LOG_ROOT(), LogLevel::LINFO, fmt, ##__VA_ARGS__)
#define LOG_FMT_WARN(fmt, ...) \
  LOG_FMT_LEVEL(LOG_ROOT(), LogLevel::LWARN, fmt, ##__VA_ARGS__)
#define LOG_FMT_ERROR(fmt, ...) \
  LOG_FMT_LEVEL(LOG_ROOT(), LogLevel::LERROR, fmt, ##__VA_ARGS__)
#define LOG_FMT_FATAL(fmt, ...) \
  LOG_FMT_LEVEL(LOG_ROOT(), LogLevel::LFATAL, fmt, ##__VA_ARGS__)

#define LOG_DEBUG_A(name) \
  LOG_STREAM(GET_LOGGER(name), LogLevel::LDEBUG)
#define LOG_INFO_A(name) \
  LOG_STREAM(GET_LOGGER(name), LogLevel::LINFO)
#define LOG_WARN_A(name) \
  LOG_STREAM(GET_LOGGER(name), LogLevel::LWARN)
#define LOG_ERROR_A(name) \
  LOG_STREAM(GET_LOGGER(name), LogLevel::LERROR)
#define LOG_FATAL_A(name) \
  LOG_STREAM(GET_LOGGER(name), LogLevel::LFATAL)

#define LY_LOG_DEBUG(pLogger) \
  LOG_STREAM(pLogger, LogLevel::LDEBUG)
#define LY_LOG_INFO(pLogger) \
  LOG_STREAM(pLogger, LogLevel::LINFO)
#define LY_LOG_WARN(pLogger) \
  LOG_STREAM(pLogger, LogLevel::LWARN)
#define LY_LOG_ERROR(pLogger) \
  LOG_STREAM(pLogger, LogLevel::LERROR)
#define LY_LOG_FATAL(pLogger) \
  LOG_STREAM(pLogger, LogLevel::LFATAL)

#define LOG_DEBUG() \
  LOG_STREAM(LOG_ROOT(), LogLevel::LDEBUG)
#define LOG_INFO() \
  LOG_STREAM(LOG_ROOT(), LogLevel::LINFO)
#define LOG_WARN() \
  LOG_STREAM(LOG_ROOT(), LogLevel::LWARN)
#define LOG_ERROR() \
  LOG_STREAM(LOG_ROOT(), LogLevel::LERROR)
#define LOG_FATAL() \
  LOG_STREAM(LOG_ROOT(), LogLevel::LFATAL)


constexpr const char *kDefaultFormatPattern = 
  "$DATETIME{%Y-%m-%d %H:%M:%S}"
  "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
  "$CHAR:\t$FILENAME$CHAR::$LINE"
  "$CHAR:\t$FUNCTION_NAME"
  "$CHAR:\t$TOTAL_MS"
  "$CHAR:\n$MESSAGE$CHAR:\n";

/*
 $MESSAGE      消息
 $LOG_LEVEL    日志级别
 $TOTAL_MS     累计毫秒数
 $LOG_NAME     日志名称
 $CHAR:\n      换行符 \n
 $CHAR:\t      制表符 \t
 $CHAR:[       括号[
 $CHAR:]       括号]
 $DATETIME     时间
 $LINE         行号
 $FILENAME     文件名

 默认格式：
  "$DATETIME{%Y-%m-%d %H:%M:%S}"
  "$CHAR:\t$THREAD_NAME$CHAR:[$THREAD_ID:%FIBER_ID$CHAR:]"
  "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
  "$CHAR:\t$FILENAME$CHAR::$LINE"
  "$CHAR:\t$FUNCTION_NAME"
  "$CHAR:\t$TOTAL_MS"
  "$CHAR:\n$MESSAGE$CHAR:\n"
*/
// 日志级别
struct LogLevel
{
	enum Level : int
  {
		LUNKNOWN = 0,
		LDEBUG   = 1,
		LINFO    = 2,
		LWARN    = 3,
		LERROR   = 4,
		LFATAL   = 5
		/* CUSTOM */
	};

	/**
	 * @brief 将日志级别转成文本输出
	 * @param[in] level 日志级别
	 */
	static std::string toString(LogLevel::Level level);

	/**
	 * @brief 将文本转换成日志级别
	 * @param[in] str 日志级别文本
	 */
	static LogLevel::Level fromString(const std::string &str);
};

class Logger;

struct LogColorConfig
{
  enum ColorType : int
  {
    END = 0,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    DEEP_RED,
  };

  const char *colors[6] = {
    "\033[0m",
    "\033[31m",
    "\033[32m",
    "\033[33m",
    "\033[34m",
    "\033[35m",
  };

  int LOG_END = END;
  int LOG_LEVEL_DEBUG = BLUE;
  int LOG_LEVEL_INFO = GREEN;
  int LOG_LEVEL_WARN = YELLOW;
  int LOG_LEVEL_ERROR = RED;
  int LOG_LEVEL_FATAL = DEEP_RED;

  const char *getColor(int type) const
  {
    return colors[type];
  }
};

// 日志事件
class LogEvent
{
public:
  SHARED_REG(LogEvent);
/**
 * @brief 构造函数
 * @param[in] pLogger     日志器
 * @param[in] level       日志级别
 * @param[in] filename    文件名
 * @param[in] line        文件行号
 * @param[in] ms_elapse   程序启动依赖的耗时(毫秒)
 * @param[in] timestamp   日志事件(秒)
 */
  LogEvent(LogLevel::Level level
           , std::string filename, int32_t line, std::string function_name
           , uint32_t ms_elapse, uint64_t timestamp, LogColorConfig config = LogColorConfig());

  std::string getFilename() const { return filename_; }
  std::string getFunctionName() const { return function_name_; }
  int32_t getLine() const { return line_; }
  // 返回耗时
  uint32_t getElapse() const { return ms_elapse_; }
  // 记录的时间
  uint64_t getTimestamp() const { return timestamp_; }
  // 返回日志内容
  std::string getContent() const { return ss_.str(); }
  LogLevel::Level getLevel() const { return level_; }
  void setLogColorOn(bool on) { color_on_ = on; }
  bool isLogColorOn() const { return color_on_; }
  LogColorConfig getColorConfig() const { return color_config_; }
  std::stringstream &getSS() { return ss_; }

  void format(const char *fmt, ...);
private:
  /// 文件名
  std::string filename_;
  /// 函数名
  std::string function_name_;
  /// 行号
  int32_t line_ = 0;
  /// 程序启动开始到现在的毫秒数
  uint32_t ms_elapse_ = 0;
  /// 时间戳
  uint64_t timestamp_ = 0;
  /// 日志内容流
  std::stringstream ss_;
  /// 日志等级
  LogLevel::Level level_;
  /// 颜色配置
  LogColorConfig color_config_;
  bool color_on_{false};
};

class LogEventWrapper;
// 日志格式器
class LogFormatter
{
protected:
  constexpr static char ID_TOKEN = '$';
  constexpr static int FORMAT_ID_LOC = 0;
  constexpr static int FORMAT_FN_ARG_LOC = 1;
  constexpr static int STATUS_CODE_LOC = 2;

  enum {
    PARSE_OK = 0,
    PARSE_ERROR = 1,
  };

  using PatArgsWrapper = std::tuple<std::string, std::string, int>;
public:
  SHARED_REG(LogFormatter);

  /**
   * @brief
   * @param[in] pattern 格式模板
   * @details
   *  $MESSAGE      消息
   *  $LOG_LEVEL    日志级别
   *  $TOTAL_MS     累计毫秒数
   *  $LOG_NAME     日志名称
   *  $CHAR:\n      换行符 \n
   *  $CHAR:\t      制表符 \t
   *  $CHAR:[       括号[
   *  $CHAR:]       括号]
   *  $DATETIME     时间
   *  $LINE         行号
   *  $FILENAME     文件名
   * 
   * 默认格式：
   *    "$DATETIME{%Y-%m-%d %H:%M:%S}"
   *    "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
   *    "$CHAR:\t$FILENAME$CHAR::$LINE"
   *    "$CHAR:\t$FUNCTION_NAME"
   *    "$CHAR:\t$TOTAL_MS"
   *    "$CHAR:\n$MESSAGE$CHAR:\n"
   */
  LogFormatter(std::string pattern = kDefaultFormatPattern) :
    pattern_(pattern)
  {
    init();
  }

  /**
   * @brief 返回格式化日志文本
   * @param[in] pLogEvent
   * @param[in] pLogger
   */
  std::string format(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger);
  std::ostream &format(std::ostream &ofs, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger);

  bool hasError() const { return hasError_; }
  const std::string lastError() const { return error_; }
  const std::string getPattern() const { return pattern_; }
private:
  void init();
  PatArgsWrapper parsePatToken(const std::string& patToken);
public:
	struct LogFormatterItem
	{
    SHARED_REG(LogFormatterItem);
    /**
     * @brief 析构函数
     */
    virtual ~LogFormatterItem() = default;
    /**
     * @brief 格式化日志到流
     * @param[in, out] os 日志输出流
     * @param[in] pLogEvent 日志事件包装器
     * @param[in] pLogger 日志器
     */
    virtual void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) = 0;
	};
private:
  /// 日志格式模板
  std::string pattern_;
  /// 日志格式解析后格式
  std::vector<LogFormatterItem::ptr> items_;
  /// 错误信息
  std::string error_;
  bool hasError_{false};
};

// 日志添加器
class LogAppender
{
public:
  SHARED_REG(LogAppender);

  LogAppender() = default;
  virtual ~LogAppender() = default;

  virtual void log(LogEvent::ptr pEvent, std::shared_ptr<Logger> pLogger) = 0;
  virtual std::string toYamlString() = 0;

  void setFormatter(LogFormatter::ptr pFormatter);
  LogFormatter::ptr getFormatter();
  
  LogLevel::Level getLevel() const { return level_; }
  void setLevel(LogLevel::Level level) { level_ = level; }
protected:
  /// 日志级别
  LogLevel::Level level_{LogLevel::LDEBUG};
  /// 是否有自己的日志格式器
  bool hasFormatter_{false};
  /// Mutex
  Mutex::type mutex_;
  /// 日志格式器
  LogFormatter::ptr pFormatter_;
};
class FileLogAppender : public LogAppender
{
public:
  SHARED_REG(FileLogAppender);

	FileLogAppender(std::string filename);
	virtual ~FileLogAppender() = default;

  virtual void log(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override;
  virtual std::string toYamlString() override;
  
  /**
   * @brief 重新打开日志文件
   * @return 成功返回true
   */
  bool reopen();
private:
  std::string getWholeFilename();
protected:
  // TODO:
  // can be defined in 'LogEnv' by ini or other config file
  static constexpr uint64_t kMaxLines = 100000;
private:
  /* real filename: filename_ + "_" + current_day + "_" + cnt{02d} + ".log" */
  std::string filename_;
  std::ofstream filestream_;
  uint64_t lastAccessTime_{0};
  uint64_t lines_{0}; 
  uint8_t cnt_{0};  // cnt_ incr when lines_ encounters kMaxLines
  int today_;
};
class StdoutLogAppender : public LogAppender
{
public:
  SHARED_REG(StdoutLogAppender);

	StdoutLogAppender() = default;
  virtual ~StdoutLogAppender() = default;

  virtual void log(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override;
  virtual std::string toYamlString() override;
private:

};

class Logger : public std::enable_shared_from_this<Logger>
{
public:
	using ptr = std::shared_ptr<Logger>;

  Logger(std::string name = "root") :
    name_(name), level_(LogLevel::Level::LDEBUG ),
    pFormatter_(new LogFormatter())
  {}
  Logger(std::string name, LogLevel::Level level, std::string pattern) :
    name_(name), level_(level),
    pFormatter_(new LogFormatter(pattern))
  {}
  ~Logger() = default;

  void log(LogEvent::ptr pEvent);

  void addAppender(LogAppender::ptr pAppender);
  void removeAppender(LogAppender::ptr pAppender);
  void clearAppenders();

  LogLevel::Level getLevel() const { return level_; }
  void setLevel(LogLevel::Level level) { level_ = level; }

  const std::string &getName() const { return name_; }
  void setFormatter(LogFormatter::ptr pFormatter);
  void setFormatter(const std::string &pattern);
  LogFormatter::ptr getFormatter();
  /**
   * @brief 将日志器的配置转成YAML String
   */
  std::string toYamlString();

  Logger::ptr getLogger() { return root_; }
  void setLogger(Logger::ptr pLogger) { root_ = pLogger; }
private:
  /// 日志名称
  std::string name_;
  /// 日志级别
  LogLevel::Level level_{LogLevel::Level::LDEBUG};
  /// Mutex
  Mutex::type mutex_;
  /// 日志目标集合
  std::list<LogAppender::ptr> appenders_;
  /// 日志格式器
  LogFormatter::ptr pFormatter_;
  /// 主日志器
  Logger::ptr root_;
};

class LogEventWrapper {
public:
  SHARED_REG(LogEventWrapper);

  /**
   * @brief 构造函数
   * @param[in] pEvent  日志事件
   * @param[in] pLogger 日志器
   */
  LogEventWrapper(LogEvent::ptr pEvent, std::shared_ptr<Logger> pLogger) :
    pEvent_(pEvent), pLogger_(pLogger)
  {}
  ~LogEventWrapper()
  {
    pLogger_->log(pEvent_);
  }

  LogEvent::ptr getEvent() const { return pEvent_; }
  std::shared_ptr<Logger> getLogger() const { return pLogger_; }
  std::stringstream &getSS() { return pEvent_->getSS(); }
private:
  ///日志事件
  LogEvent::ptr pEvent_;
  std::shared_ptr<Logger> pLogger_;
};

class LogManager
{
public:
  // friend class Logger;
  using ptr = std::shared_ptr<LogManager>;

  LogManager();
  ~LogManager() = default;
  /**
   * @brief 获取日志器
   * @param[in] name 日志器名称
   */
  Logger::ptr getLogger(const std::string &name);

  // TODO: For future do
  void init();
  Logger::ptr getRoot() const { return root_; }
  /**
   * @brief 将所有的日志器配置转成YAML String
   */
  std::string toYamlString();
private:
  /// Mutex
  Mutex::type mutex_;
  /// 日志器容器
  std::unordered_map<std::string, Logger::ptr> loggers_;
  /// 主日志器
  Logger::ptr root_;
};

using SingleLogManager = Singleton<LogManager>;

class LogIniter
{
public:
  /**
   * \brief
   * \param pLogger
   * \param conf
   * conf format:
   * - name : <logger name>
   * - level : <log level>
   * - formatter : <>
   * - appenders : <some appenders>
   */
  static void init(Logger::ptr pLogger, const YAML::Node &conf);
  static void init(Logger::ptr pLogger, const std::string &conf_file);

  /* the appender's formatter is the same as the logger */
  static Logger::ptr getLogger(
    /* logger */
    const std::string &log_name, LogLevel::Level log_level,
    /* formatter */
    const std::string &format_pattern = kDefaultFormatPattern,
    /* appender */
    bool write2file = true, const std::string &filename = "x");
private:
};

/* LogIniter::getLogger("sample", "LogLevel::Level::LDEBUG",
 *                      kDefaultFormatPattern, true,
 *                      "sample")
 *
 * sample_${DATE}_${Count}.log
 */
