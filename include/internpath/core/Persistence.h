#pragma once

#include <string>
#include "internpath/core/AppState.h"
#include "internpath/domain/Result.h"

namespace internpath
{
    class CsvStorage
    {
    public:
        static ActionResult saveSnapshot(const AppState& state, const std::string& directoryPath);
        static ActionResult loadSnapshot(AppState& state, const std::string& directoryPath);
    };
}
