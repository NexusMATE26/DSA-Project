#pragma once

#include <cstdint>

namespace internpath
{
    using StudentId = std::uint32_t;
    using SkillId = std::uint32_t;
    using CompanyId = std::uint32_t;
    using InternshipId = std::uint32_t;
    using ApplicationId = std::uint32_t;

    enum class WorkMode
    {
        Onsite,
        Remote,
        Hybrid
    };

    enum class InternshipStatus
    {
        Open,
        Closed,
        Cancelled
    };

    enum class ApplicationStatus
    {
        Submitted,
        Allocated,
        Waitlisted,
        Withdrawn,
        Cancelled
    };
}
