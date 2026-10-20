#pragma once

#include <cstdint>

namespace internpath
{
    // Date is stored as YYYYMMDD, e.g. 20261115.
    // This works as an ordering key ONLY after a real date validation.
    inline bool isValidDate(std::int32_t date)
    {
        int year = date / 10000;
        int month = (date / 100) % 100;
        int day = date % 100;

        if (year < 2000 || year > 2200 || month < 1 || month > 12)
        {
            return false;
        }

        int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool leapYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;

        if (leapYear)
        {
            daysInMonth[2] = 29;
        }

        return day >= 1 && day <= daysInMonth[month];
    }
}
