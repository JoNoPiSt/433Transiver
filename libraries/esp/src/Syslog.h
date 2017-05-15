#ifndef SYSLOG_H
#define SYSLOG_H

#include <cstdint>

namespace esp {

#ifndef SYSLOG_MAX_ENTRIES
#define SYSLOG_MAX_ENTRIES 35
#endif

#ifndef SYSLOG_MAX_STR_LEN
#define SYSLOG_MAX_STR_LEN 50
#endif

// TODO: extract to separate file
template <typename T>
class Iterator {
public:
  virtual ~Iterator() {}

  virtual bool HasNext() = 0;
  virtual T* Next() = 0;
};

struct LogEntry {
  unsigned long time;
  char str[SYSLOG_MAX_STR_LEN];
};

class Syslog : protected Iterator<LogEntry> {
public:
  typedef Iterator<LogEntry> Iter;

  static Syslog& Instance();

  void Log(const char* format, ...);

  Syslog::Iter& Items();

protected:
  bool HasNext();
  LogEntry* Next();

private:
  Syslog();

  LogEntry m_Entries[SYSLOG_MAX_ENTRIES];
  uint8_t m_LogLatestIndex;
  uint8_t m_IterIndex;
};

}

#endif
