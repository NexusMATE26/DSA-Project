#pragma once

#include <string>

namespace internpath
{
    enum class ErrorCode
    {
        None,
        NotFound,
        Duplicate,
        InvalidInput,
        NotEligible,
        RoundClosed,
        Conflict,
        StorageError,
        NotImplemented
    };

    struct ActionResult
    {
        bool ok = false;
        ErrorCode error = ErrorCode::NotImplemented;
        std::string message;

        static ActionResult success(const std::string& text = "")
        {
            return {true, ErrorCode::None, text};
        }

        static ActionResult failure(ErrorCode code, const std::string& text)
        {
            return {false, code, text};
        }
    };
}
