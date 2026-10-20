#pragma once

#include <cstdint>

#include "internpath/domain/Models.h"
#include "internpath/ds/HashTable.h"
#include "internpath/ds/DeadlineAVL.h"
#include "internpath/ds/DirectedGraph.h"

namespace internpath
{
    // M3 CONTRACT - The single authoritative owner of domain records.
    // Append-only arrays preserve stable indexes; indexes point at array POSITIONS,
    // never at elements' raw addresses (array growth invalidates pointers).
    class AppState
    {
    public:
        DynamicArray<Student> students;
        DynamicArray<Skill> skills;
        DynamicArray<Company> companies;
        DynamicArray<Internship> internships;
        DynamicArray<Application> applications;

        HashTable<StudentId, std::uint32_t> studentIndexes;
        HashTable<SkillId, std::uint32_t> skillIndexes;
        HashTable<CompanyId, std::uint32_t> companyIndexes;
        HashTable<InternshipId, std::uint32_t> internshipIndexes;
        HashTable<ApplicationId, std::uint32_t> applicationIndexes;

        DeadlineAVL openDeadlines;
        DirectedGraph skillPrerequisites;

        StudentId nextStudentId = 1;
        SkillId nextSkillId = 1;
        CompanyId nextCompanyId = 1;
        InternshipId nextInternshipId = 1;
        ApplicationId nextApplicationId = 1;
        std::uint64_t nextSubmitSequence = 1;
        std::int32_t demoToday = 20261020;

        // TODO M3: Implement add/find, derived-index rebuild and invariant checks.
        // Services must be the only layer mutating state after integration.
    };
}
