#pragma once

#include <cstdint>
#include <string>

#include "internpath/domain/Types.h"
#include "internpath/ds/DynamicArray.h"

namespace internpath
{
    struct Skill
    {
        SkillId id = 0;
        std::string name;
        std::string category;
        std::uint32_t denseIndex = 0;
        bool active = true;
    };

    struct Student
    {
        StudentId id = 0;
        std::string fullName;
        std::string email;
        // Index by Skill::denseIndex. 0 = not declared; 1..3 = proficiency.
        DynamicArray<std::uint8_t> proficiency;
        bool active = true;
    };

    struct Company
    {
        CompanyId id = 0;
        std::string name;
        std::string description;
        bool active = true;
    };

    struct Requirement
    {
        SkillId skillId = 0;
        std::uint8_t requiredLevel = 1;
        std::uint8_t weight = 1;
        bool mandatory = false;
    };

    struct Internship
    {
        InternshipId id = 0;
        CompanyId companyId = 0;
        std::string title;
        std::string description;
        std::string location;
        WorkMode mode = WorkMode::Onsite;
        std::int32_t stipend = 0;
        std::uint32_t capacity = 1;
        std::int32_t deadline = 0; // Validated YYYYMMDD
        InternshipStatus status = InternshipStatus::Open;
        DynamicArray<Requirement> requirements;
    };

    struct Application
    {
        ApplicationId id = 0;
        StudentId studentId = 0;
        InternshipId internshipId = 0;
        ApplicationStatus status = ApplicationStatus::Submitted;
        std::uint64_t submitSequence = 0;
        std::int32_t fitNumeratorAtSubmit = 0;
        std::int32_t fitDenominatorAtSubmit = 1;
        std::uint32_t rankAtClose = 0;
    };
}
