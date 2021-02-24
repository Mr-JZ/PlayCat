#ifndef ENUM_SERIAL_H
#define ENUM_SERIAL_H

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#ifdef _WIN_
#include "dirent.h"
#include <atlbase.h>
#include <tchar.h>
#include <windows.h>

#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

class EnumSerial {
  public:
    EnumSerial();
    std::vector<std::string> enumSerialPorts();

  private:
#ifdef _WIN_
    TCHAR key_valuename[1000];
    unsigned long len_valuename = 1000;
    wchar_t key_valuedata[1000];
    unsigned long len_valuedata = 1000;
    unsigned long key_type;
    HKEY hkey = NULL;
#endif
};

#endif // ENUM_SERIAL_H
