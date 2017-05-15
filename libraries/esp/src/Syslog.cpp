#include <Syslog.h>
#include <Arduino.h>
#include <cstdarg>
#include <cstdio>

namespace esp {

Syslog& Syslog::Instance()
{
  static Syslog instance;
  return instance;
}

Syslog::Syslog() :
  m_LogLatestIndex(0),
  m_IterIndex(0)
{
}

void Syslog::Log(const char* format, ...)
{
  LogEntry* entry = &m_Entries[m_LogLatestIndex];
  entry->time = millis();

  va_list args;
  va_start(args, format);
  vsnprintf(entry->str, SYSLOG_MAX_STR_LEN, format, args);
  va_end(args);

  m_LogLatestIndex = (m_LogLatestIndex + 1) % SYSLOG_MAX_ENTRIES;
}

Syslog::Iter& Syslog::Items()
{
  m_IterIndex = 0;
  return *this;
}

bool Syslog::HasNext()
{
  return m_IterIndex < SYSLOG_MAX_ENTRIES;
}

LogEntry* Syslog::Next()
{
  return &m_Entries[(m_LogLatestIndex + m_IterIndex++) % SYSLOG_MAX_ENTRIES];
}

} // namespace esp
