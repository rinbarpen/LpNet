#include "Logger.h"
#include "Logger.h"

#include "string_utils.h"

#include <ctime>
#include <tuple>
#include <functional>
#include <string_view>
#include <type_traits>

/// LogLevel
std::string LogLevel::toString(LogLevel::Level level)
{
  switch (level)
  {
// #define XX(name) \
//   case LogLevel::name: \
//     return #name
//
//   XX(LDEBUG);
//   XX(LINFO);
//   XX(LWARN);
//   XX(LERROR);
//   XX(LFATAL);
// #undef XX
  case LogLevel::Level::LDEBUG:
    return "DEBUG";
  case LogLevel::Level::LINFO:
    return "INFO";
  case LogLevel::Level::LWARN:
    return "WARN";
  case LogLevel::Level::LERROR:
    return "ERROR";
  case LogLevel::Level::LFATAL:
    return "FATAL";
  default:
    return "NONE";
  }
  
  UNREACHABLE();
  return {};
}
LogLevel::Level LogLevel::fromString(const std::string &str)
{
// #define XX(name) \
//   if (str == #name) { return LogLevel::name; }
//
//   XX(LDEBUG)
//   XX(LINFO)
//   XX(LWARN)
//   XX(LERROR)
//   XX(LFATAL)
// #undef XX

  if (str == "DEBUG") { return LogLevel::Level::LDEBUG; }
  if (str == "INFO")  { return LogLevel::Level::LINFO;  }
  if (str == "WARN")  { return LogLevel::Level::LWARN;  }
  if (str == "ERROR") { return LogLevel::Level::LERROR; }
  if (str == "FATAL") { return LogLevel::Level::LFATAL; }

  return LogLevel::LUNKNOWN;
}

/// LogEvent
LogEvent::LogEvent(LogLevel::Level level, 
  std::string filename, int32_t line, std::string function_name,
  uint32_t ms_elapse, uint64_t timestamp, LogColorConfig config) :
  level_(level), 
  filename_(filename), line_(line), function_name_(function_name),
  ms_elapse_(ms_elapse), timestamp_(timestamp)
{
}
void LogEvent::format(const char *fmt, ...)
{
  va_list args;
  char buf[256]{0};
  
  va_start(args, fmt);
  int len = vsnprintf(&buf[0], sizeof(buf), fmt, args);
  if (len < 0) {
    va_end(args);
    return;
  }

  ss_ << std::string(buf, len);
  va_end(args);
}

/// LogAppender
void LogAppender::setFormatter(LogFormatter::ptr pFormatter) {
  Mutex::lock locker(mutex_);
  pFormatter_ = pFormatter;
  if (pFormatter_) {
    hasFormatter_ = true;
  }
  else {
    hasFormatter_ = false;
  }
}
LogFormatter::ptr LogAppender::getFormatter() {
  Mutex::lock locker(mutex_);
  return pFormatter_;
}

/// FileLogAppender
FileLogAppender::FileLogAppender(std::string filename) :
  filename_(filename)
{
  reopen();
}
void FileLogAppender::log(LogEvent::ptr pEvent, std::shared_ptr<Logger> pLogger) {
  if (pEvent->getLevel() >= level_) {
    uint64_t now = pEvent->getTimestamp();
    if (now >= (lastAccessTime_ + 3)) {
      reopen();
      lastAccessTime_ = now;
    }

    Mutex::lock locker(mutex_);
    if (!pFormatter_->format(filestream_, pEvent, pLogger)) {
      std::cerr << "error in " << "FileLogAppender::log" << " with Formatter format" << std::endl;
      std::cerr << "log file cannot be created" << std::endl;
    } else {
      lines_++;
      if (lines_ >= kMaxLines) {
        cnt_++;
        lines_ = 0;
      }
    }
  }
}
std::string FileLogAppender::toYamlString()
{
  Mutex::lock locker(mutex_);
  YAML::Node node;
  node["type"] = "FileLogAppender";
  node["file"] = filename_;
  if (level_ != LogLevel::LUNKNOWN) {
    node["level"] = LogLevel::toString(level_);
  }
  if (hasFormatter_ && pFormatter_) {
    node["formatter"] = pFormatter_->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}
bool FileLogAppender::reopen()
{
  Mutex::lock locker(mutex_);
  if (filestream_) {
    filestream_.close();
  }

  // filestream_.open(filename_, std::ios::app);
  filestream_.open(getWholeFilename(), std::ios::app);
  return filestream_.is_open();
}

std::string FileLogAppender::getWholeFilename()
{
  std::string wholeFilename;
  auto &&[today, todayStr] = Clock::currentDay("%Y-%m-%d");
  if (today != today_) {
    today_ = today;
    cnt_ = 0;
  }
  wholeFilename.append(filename_);
  wholeFilename.append("_");
  wholeFilename.append(todayStr);
  wholeFilename.append("_");
  if (cnt_ < 10)
    wholeFilename.append("0");
  wholeFilename.append(std::to_string(cnt_));
  wholeFilename.append(".log");

  // std::sprintf(wholeFilename.data(), 
  //   "%s_%s_%02d.log", 
  //   filename_.c_str(), Clock::currentDayStr(t, "%Y-%m-%d").c_str(), cnt_);
  return wholeFilename;
}

/// StdoutLogAppender
void StdoutLogAppender::log(LogEvent::ptr pEvent, std::shared_ptr<Logger> pLogger) {
  if (pEvent->getLevel() >= level_) {
    Mutex::lock locker(mutex_);
    pEvent->setLogColorOn(true);
    pFormatter_->format(std::cout, pEvent, pLogger);
    pEvent->setLogColorOn(false);
  }
}
std::string StdoutLogAppender::toYamlString() {
  Mutex::lock locker(mutex_);
  YAML::Node node;
  node["type"] = "StdoutLogAppender";
  if (level_ != LogLevel::LUNKNOWN) {
    node["level"] = LogLevel::toString(level_);
  }
  if (hasFormatter_ && pFormatter_) {
    node["formatter"] = pFormatter_->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

/// FormatterItems
class MessageFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  MessageFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << pLogEvent->getContent();
  }
};
class LogLevelFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  LogLevelFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    if (pLogEvent->isLogColorOn()) {
      LogColorConfig conf = pLogEvent->getColorConfig();
      switch (pLogEvent->getLevel()) {
      case LogLevel::LDEBUG:
        os << conf.getColor(conf.LOG_LEVEL_DEBUG);
        break;
      case LogLevel::LINFO:
        os << conf.getColor(conf.LOG_LEVEL_INFO);
        break;
      case LogLevel::LWARN:
        os << conf.getColor(conf.LOG_LEVEL_WARN);
        break;
      case LogLevel::LERROR:
        os << conf.getColor(conf.LOG_LEVEL_ERROR);
        break;
      case LogLevel::LFATAL:
        os << conf.getColor(conf.LOG_LEVEL_FATAL);
        break;
      }
      os << LogLevel::toString(pLogEvent->getLevel());
      if (pLogEvent->getLevel() != LogLevel::LUNKNOWN)
        os << conf.getColor(conf.LOG_END);
    } else {
      os << LogLevel::toString(pLogEvent->getLevel());
    }
  }
};
class ElapseFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  ElapseFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << pLogEvent->getElapse();
  }
};
class LogNameFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  LogNameFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << pLogger->getName();
  }
};
class DateTimeFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  DateTimeFormatterItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
    :timefmt_(format) {
    if (timefmt_.empty()) {
      timefmt_ = "%Y-%m-%d %H:%M:%S";
    }
  }

  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    auto timestamp = pLogEvent->getTimestamp();
    auto tp = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(timestamp));
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    struct tm pTm = *std::localtime(&time);
    char buf[64];
    std::strftime(buf, sizeof(buf), timefmt_.c_str(), &pTm);
    os << buf;
  }
private:
  std::string timefmt_;
};
class FilenameFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  FilenameFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << pLogEvent->getFilename();
  }
};
class LineFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  LineFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << pLogEvent->getLine();
  }
};
class StringFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  StringFormatterItem(const std::string &str)
    :str_(str) {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << str_;
  }
private:
  std::string str_;
};
class CharFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  CharFormatterItem(const std::string &str = "") :
    str_(str)
  {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << str_;
  }
private:
  std::string str_;
};
class FunctionNameFormatterItem final : public LogFormatter::LogFormatterItem {
public:
  FunctionNameFormatterItem(const std::string &str)
    :str_(str) {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override {
    os << pLogEvent->getFunctionName();
  }
private:
  std::string str_;
};

/// LogFormatter
std::string LogFormatter::format(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger)
{
  std::stringstream ss;
  for (auto &item : items_) {
    item->format(ss, pLogEvent, pLogger);
  }
  return ss.str();
}
std::ostream &LogFormatter::format(std::ostream &ofs, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger)
{
  std::stringstream ss;
  for (auto &item : items_) {
    item->format(ss, pLogEvent, pLogger);
  }
  ofs << ss.str();
  ofs.flush();
  return ofs;
}
LogFormatter::PatArgsWrapper LogFormatter::parsePatToken(const std::string& patToken)
{
  if (string_api::start_with(patToken, "CHAR:")) {
    if (patToken.length() <= 5) return std::make_tuple("CHAR", "", PARSE_ERROR);
    auto ch = patToken.substr(5);
    return std::make_tuple("CHAR", std::string(ch), PARSE_OK);
  }
  if (string_api::start_with(patToken, "DATETIME")) {
    if (patToken.length() > 8 && patToken[8] == '{') {
      size_t timefmt_len = patToken.rfind('}');
      if (timefmt_len >= 9) {
        timefmt_len -= 9;
        auto timefmt = patToken.substr(9, timefmt_len);
        return std::make_tuple("DATETIME", timefmt, PARSE_OK);
      }
      else {
        // error timefmt
        return std::make_tuple(patToken, "", PARSE_ERROR);
      }
    }
    else {
      // Default DATETIME format
      return std::make_tuple("DATETIME", "%Y-%m-%d %H:%M:%S", PARSE_OK);
    }
  }
  // NO PARAM ARG
  return {std::string(patToken), "", PARSE_OK};
}
void LogFormatter::init()
{
  std::vector<PatArgsWrapper> vec;
  std::string nstr;
  size_t start_pos = 0, len = 0;
  for (size_t i = 0; i < pattern_.size(); ++i) {
    if (pattern_[i] == ID_TOKEN) {
      if (len != 0) {
        nstr = pattern_.substr(start_pos, len);
        vec.push_back(parsePatToken(nstr));
      }

      start_pos = i + 1;
      len = 0;
      continue;
    }

    ++len;
  }

  if (len != 0) {
    nstr = pattern_.substr(start_pos, len);
    vec.push_back(parsePatToken(nstr));
  }
  else {
    // $
    vec.push_back(std::make_tuple("", "", PARSE_ERROR));
  }

  static std::unordered_map<std::string, std::function<LogFormatterItem::ptr(const std::string &str)> > 
    s_format_items = {
#define XX(STR, ID) \
    { STR, [](const std::string& str) -> LogFormatterItem::ptr { return std::make_shared<ID>(str);} }
      XX("MESSAGE"    , MessageFormatterItem),
      XX("LOG_LEVEL"  , LogLevelFormatterItem),
      XX("TOTAL_MS"   , ElapseFormatterItem),
      XX("LOG_NAME"   , LogNameFormatterItem),
      XX("DATETIME"   , DateTimeFormatterItem),
      XX("FILENAME"   , FilenameFormatterItem),
      XX("LINE"       , LineFormatterItem),
      XX("CHAR"       , CharFormatterItem),
      XX("FUNCTION_NAME", FunctionNameFormatterItem),
#undef XX
  };

  hasError_ = false;
  for (const auto &wrapper : vec) {
    const auto& [id, arg, status] = wrapper;
    if (status != PARSE_OK) {
      items_.push_back(std::make_shared<StringFormatterItem>(id));
      continue;
    }

    auto it = s_format_items.find(id);
    if (it == s_format_items.end()) {
      hasError_ = true;
      error_.clear();
      error_.append("<<PATTERN ERROR: UNSUPPORTED FORMAT $");
      error_.append(id);
      error_.append(">>");
      items_.push_back(std::make_shared<StringFormatterItem>(error_));
    }
    else {
      items_.push_back(it->second(arg));
    }

    // std::cerr << "(" << id << ") - (" << arg << ") - (" << status << ")" << std::endl;
  }
  // std::cerr << items_.size() << std::endl;
}

/// Logger
void Logger::addAppender(LogAppender::ptr pAppender)
{
  Mutex::lock locker(mutex_);

  if (!pAppender->getFormatter()) {
    pAppender->setFormatter(pFormatter_);
  }
  appenders_.push_back(pAppender);
}
void Logger::removeAppender(LogAppender::ptr pAppender)
{
  Mutex::lock locker(mutex_);
  auto it = std::find(appenders_.begin(), appenders_.end(), pAppender);
  if (it != appenders_.end()) {
    appenders_.erase(it); 
  }
}
void Logger::clearAppenders()
{
  Mutex::lock locker(mutex_);
  appenders_.clear();
}
void Logger::log(LogEvent::ptr pEvent)
{
  if (pEvent->getLevel() >= level_) {
    auto self = shared_from_this();
    Mutex::lock locker(mutex_);
    if (!appenders_.empty()) {
      for (auto &pAppender : appenders_) {
        pAppender->log(pEvent, self);
      }
    }
    else if (root_)
      root_->log(pEvent);
  }
}
void Logger::setFormatter(LogFormatter::ptr pFormatter)
{
  Mutex::lock locker(mutex_);
  pFormatter_ = pFormatter;
}
void Logger::setFormatter(const std::string &pattern)
{
  Mutex::lock locker(mutex_);
  pFormatter_ = LogFormatter::make_shared(pattern);
}
LogFormatter::ptr Logger::getFormatter()
{
  Mutex::lock locker(mutex_);
  return pFormatter_;
}
std::string Logger::toYamlString()
{
  Mutex::lock locker(mutex_);

  YAML::Node node;
  node["name"] = name_;
  if (level_ != LogLevel::LUNKNOWN) {
    node["level"] = LogLevel::toString(level_);
  }
  if (pFormatter_) {
    node["formatter"] = pFormatter_->getPattern();
  }
  for (auto &pAppender : appenders_) {
    node["appenders"].push_back(YAML::Load(pAppender->toYamlString()));
  }

  std::stringstream ss;
  ss << node;
  return ss.str();
}

/// LogManager
LogManager::LogManager() 
{
  root_.reset(new Logger());
  auto pAppender = std::make_shared<StdoutLogAppender>();
  // pAppender->setFormatter(root_->getFormatter());
  root_->addAppender(pAppender);

  loggers_[root_->getName()] = root_;

  init();
}
void LogManager::init()
{
  
}
Logger::ptr LogManager::getLogger(const std::string &name) {
  Mutex::lock locker(mutex_);
  auto it = loggers_.find(name);
  if (it != loggers_.end()) {
    return it->second;
  }

  auto pLogger = std::make_shared<Logger>(name);
  pLogger->setLogger(root_);
  loggers_[name] = pLogger;
  return pLogger;
}
std::string LogManager::toYamlString() {
  Mutex::lock locker(mutex_);
  YAML::Node node;
  for (auto &[logName, pLogger] : loggers_) {
    node.push_back(YAML::Load(pLogger->toYamlString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

// TODO: Test this
void LogIniter::init(Logger::ptr pLogger, const YAML::Node& conf)
{
  if (conf.IsNull()) return;
  if (pLogger->getName() != conf["name"].as<std::string>()) return;

  if (conf["level"].IsDefined()) {
    pLogger->setLevel(LogLevel::fromString(conf["level"].as<std::string>()));
  }
  else {
    pLogger->setLevel(LogLevel::LDEBUG);
  }

  LogFormatter::ptr pLogFormatter = nullptr;
  if (conf["formatter"].IsDefined()) {
    pLogFormatter = LogFormatter::make_shared(conf["formatter"].as<std::string>());
    pLogger->setFormatter(pLogFormatter);
  }
  else {
    pLogFormatter = LogFormatter::make_shared();
    pLogger->setFormatter(pLogFormatter);
  }

  if (conf["appenders"].IsDefined()) {
    for (auto node : conf["appenders"]) {
      std::string typeStr = node["type"].as<std::string>();
      if (typeStr == "StdoutLogAppender") {
        auto pStdoutLogAppender = StdoutLogAppender::make_shared();
        if (node["level"].IsDefined()) {
          pStdoutLogAppender->setLevel(LogLevel::fromString(conf["level"].as<std::string>()));
        }
        else {
          pStdoutLogAppender->setLevel(LogLevel::LDEBUG);
        }
        pStdoutLogAppender->setFormatter(pLogFormatter);

        pLogger->addAppender(pStdoutLogAppender);
      }
      else if (typeStr == "FileLogAppender") {
        if (!node["file"].IsDefined()) continue;
        auto pFileLogAppender = FileLogAppender::make_shared(node["file"].as<std::string>());
        if (node["level"].IsDefined()) {
          pFileLogAppender->setLevel(LogLevel::fromString(conf["level"].as<std::string>()));
        }
        else {
          pFileLogAppender->setLevel(LogLevel::LDEBUG);
        }
        pFileLogAppender->setFormatter(pLogFormatter);

        pLogger->addAppender(pFileLogAppender);
      }
    }
  }
}
// TODO: Test this
void LogIniter::init(Logger::ptr pLogger, const std::string& conf_file)
{
  std::ifstream ifs(conf_file);
  if (!ifs.is_open()) {
    // no conf file
    return;
  }

  YAML::Node node = YAML::LoadFile(conf_file);
  init(pLogger, node);
}

Logger::ptr LogIniter::getLogger(const std::string& log_name, LogLevel::Level log_level,
  const std::string& format_pattern, bool write2file, const std::string &filename)
{
  auto pLogger = SingleLogManager::instance()->getLogger(log_name);
  pLogger->setLevel(log_level);
  if (format_pattern != kDefaultFormatPattern)
    pLogger->setFormatter(format_pattern);

  if (write2file) {
    pLogger->addAppender(FileLogAppender::make_shared(filename));
  } else {
    pLogger->addAppender(StdoutLogAppender::make_shared());
  }

  return pLogger;
}

