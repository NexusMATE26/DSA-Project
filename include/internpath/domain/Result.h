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
        std::uint32_t createdId = 0;

        static ActionResult success(const std::string& text = "", std::uint32_t id = 0)
        {
            return {true, ErrorCode::None, text, id};
        }

        static ActionResult failure(ErrorCode code, const std::string& text)
        {
            return {false, code, text, 0};
        }
    };
}
