#pragma once

#include <cstdint>
#include <string>

#include "internpath/core/AppState.h"
#include "internpath/domain/Result.h"

namespace internpath
{
    // M3 CONTRACTS. Implement with separate .h/.cpp per service after DSA unit tests.
    // These are signatures, not working implementations.
    class StudentService
    {
    public:
        explicit StudentService(AppState& state) : state_(state) {}

        ActionResult registerStudent(const std::string& name,
                                     const std::string& email);
        ActionResult setSkillLevel(StudentId studentId, SkillId skillId, int level);
        ActionResult undoLastProfileEdit(StudentId studentId);

    private:
        AppState& state_;
    };

    class AdminService
    {
    public:
        explicit AdminService(AppState& state) : state_(state) {}

        ActionResult addSkill(const std::string& name);
        ActionResult addCompany(const std::string& name);
        ActionResult addPrerequisite(SkillId first, SkillId next, int minimumLevel);
        ActionResult publishInternship(Internship&& internship);
        ActionResult closeRound(InternshipId internshipId);

    private:
        AppState& state_;
    };

    class ApplicationService
    {
    public:
        explicit ApplicationService(AppState& state) : state_(state) {}

        ActionResult apply(StudentId studentId, InternshipId internshipId);
        ActionResult withdraw(ApplicationId applicationId);

    private:
        AppState& state_;
    };
}
