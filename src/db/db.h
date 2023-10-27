#pragma once

#include <sqlite3.h>
#include <string>

class DB
{
public:

protected:

};

class SqliteDB : public DB
{
public:
  SqliteDB(){}
  ~SqliteDB() {}

  bool open(const char *filename)
  {
    r = sqlite3_open_v2(filename, &pDb_, 0, nullptr);
    if (r < 0) {
      opened_ = false;
      return false;
    }

    opened_ = true;
    return true;
  }

  void close()
  {
    r = sqlite3_close_v2(pDb_);
    if (r >= 0) {
      opened_ = false;
    }
  }

  bool exec(const char *sql)
  {
    r = sqlite3_exec(pDb_, sql, 0, 0, 0);
    if (r < 0) {
      return false;
    }

    return true;
  }

  bool isOpen() const
  {
    return opened_;
  }

  std::string getError() const
  {
    if (r < 0)
      return sqlite3_errmsg(pDb_);
    return "";
  }

protected:
  sqlite3 *pDb_;
  bool opened_{false};
  int r{0};
};