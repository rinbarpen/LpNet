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
 $MESSAGE      ��Ϣ
 $LOG_LEVEL    ��־����
 $TOTAL_MS     �ۼƺ�����
 $LOG_NAME     ��־����
 $CHAR:\n      ���з� \n
 $CHAR:\t      �Ʊ�� \t
 $CHAR:[       ����[
 $CHAR:]       ����]
 $DATETIME     ʱ��
 $LINE         �к�
 $FILENAME     �ļ���

 Ĭ�ϸ�ʽ��
  "$DATETIME{%Y-%m-%d %H:%M:%S}"
  "$CHAR:\t$THREAD_NAME$CHAR:[$THREAD_ID:%FIBER_ID$CHAR:]"
  "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
  "$CHAR:\t$FILENAME$CHAR::$LINE"
  "$CHAR:\t$FUNCTION_NAME"
  "$CHAR:\t$TOTAL_MS"
  "$CHAR:\n$MESSAGE$CHAR:\n"
*/
// ��־����
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
	 * @brief ����־����ת���ı����
	 * @param[in] level ��־����
	 */
	static std::string toString(LogLevel::Level level);

	/**
	 * @brief ���ı�ת������־����
	 * @param[in] str ��־�����ı�
	 */
	static LogLevel::Level fromString(const std::string &str);
};

class Logger;

// ��־�¼�
class LogEvent
{
public:
  SHARED_REG(LogEvent);
/**
 * @brief ���캯��
 * @param[in] pLogger     ��־��
 * @param[in] level       ��־����
 * @param[in] filename    �ļ���
 * @param[in] line        �ļ��к�
 * @param[in] ms_elapse   �������������ĺ�ʱ(����)
 * @param[in] timestamp   ��־�¼�(��)
 */
  LogEvent(LogLevel::Level level
           , std::string filename, int32_t line, std::string function_name
           , uint32_t ms_elapse, uint64_t timestamp);

  std::string getFilename() const { return filename_; }
  std::string getFunctionName() const { return function_name_; }
  int32_t getLine() const { return line_; }
  // ���غ�ʱ
  uint32_t getElapse() const { return ms_elapse_; }
  // ��¼��ʱ��
  uint64_t getTimestamp() const { return timestamp_; }
  // ������־����
  std::string getContent() const { return ss_.str(); }
  LogLevel::Level getLevel() const { return level_; }
  std::stringstream &getSS() { return ss_; }

  void format(const char *fmt, ...);
private:
  /// �ļ���
  std::string filename_;
  /// ������
  std::string function_name_;
  /// �к�
  int32_t line_ = 0;
  /// ����������ʼ�����ڵĺ�����
  uint32_t ms_elapse_ = 0;
  /// ʱ���
  uint64_t timestamp_ = 0;
  /// ��־������
  std::stringstream ss_;
  /// ��־�ȼ�
  LogLevel::Level level_;
};

class LogEventWrapper;
// ��־��ʽ��
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
   * @param[in] pattern ��ʽģ��
   * @details
   *  $MESSAGE      ��Ϣ
   *  $LOG_LEVEL    ��־����
   *  $TOTAL_MS     �ۼƺ�����
   *  $LOG_NAME     ��־����
   *  $CHAR:\n      ���з� \n
   *  $CHAR:\t      �Ʊ�� \t
   *  $CHAR:[       ����[
   *  $CHAR:]       ����]
   *  $DATETIME     ʱ��
   *  $LINE         �к�
   *  $FILENAME     �ļ���
   * 
   * Ĭ�ϸ�ʽ��
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
   * @brief ���ظ�ʽ����־�ı�
   * @param[in] pEventWrapper
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
     * @brief ��������
     */
    virtual ~LogFormatterItem() = default;
    /**
     * @brief ��ʽ����־����
     * @param[in, out] os ��־�����
     * @param[in] pEventWrapper ��־�¼���װ��
     */
    virtual void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) = 0;
	};
private:
  /// ��־��ʽģ��
  std::string pattern_;
  /// ��־��ʽ�������ʽ
  std::vector<LogFormatterItem::ptr> items_;
  /// ������Ϣ
  std::string error_;
  bool hasError_{false};
};

// ��־�����
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
  /// ��־����
  LogLevel::Level level_{LogLevel::LDEBUG};
  /// �Ƿ����Լ�����־��ʽ��
  bool hasFormatter_{false};
  /// Mutex
  Mutex::type mutex_;
  /// ��־��ʽ��
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
   * @brief ���´���־�ļ�
   * @return �ɹ�����true
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
   * @brief ����־��������ת��YAML String
   */
  std::string toYamlString();

  Logger::ptr getLogger() { return root_; }
  void setLogger(Logger::ptr pLogger) { root_ = pLogger; }
private:
  /// ��־����
  std::string name_;
  /// ��־����
  LogLevel::Level level_{LogLevel::Level::LDEBUG};
  /// Mutex
  Mutex::type mutex_;
  /// ��־Ŀ�꼯��
  std::list<LogAppender::ptr> appenders_;
  /// ��־��ʽ��
  LogFormatter::ptr pFormatter_;
  /// ����־��
  Logger::ptr root_;
};

class LogEventWrapper {
public:
  SHARED_REG(LogEventWrapper);

  /**
   * @brief ���캯��
   * @param[in] pEvent  ��־�¼�
   * @param[in] pLogger ��־��
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
  ///��־�¼�
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
   * @brief ��ȡ��־��
   * @param[in] name ��־������
   */
  Logger::ptr getLogger(const std::string &name);

  // TODO: For future do
  void init();
  Logger::ptr getRoot() const { return root_; }
  /**
   * @brief �����е���־������ת��YAML String
   */
  std::string toYamlString();
private:
  /// Mutex
  Mutex::type mutex_;
  /// ��־������
  std::unordered_map<std::string, Logger::ptr> loggers_;
  /// ����־��
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
