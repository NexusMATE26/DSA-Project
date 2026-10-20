#pragma once

#include <cstdint>
#include <string>

#include "internpath/core/AppState.h"
#include "internpath/domain/Matching.h"
#include "internpath/domain/Result.h"

namespace internpath
{
    struct Recommendation
    {
        InternshipId internshipId = 0;
        MatchResult match;
        std::int32_t deadline = 0;
    };

    struct LearningTask
    {
        SkillId skillId = 0;
        std::string skillName;
        std::uint8_t targetLevel = 1;
        std::uint8_t currentLevel = 0;
        std::uint32_t stage = 0;
    };

    class StudentService
    {
    public:
        explicit StudentService(AppState& state) : state_(state) {}

        ActionResult registerStudent(const std::string& name,
                                     const std::string& email);
        ActionResult setSkillLevel(StudentId studentId, SkillId skillId, int level);
        ActionResult undoLastProfileEdit(StudentId studentId);

        DynamicArray<Recommendation> getRecommendations(StudentId studentId);
        DynamicArray<LearningTask> getLearningPlan(StudentId studentId, InternshipId internshipId);

    private:
        AppState& state_;
    };

    class AdminService
    {
    public:
        explicit AdminService(AppState& state) : state_(state) {}

        ActionResult addSkill(const std::string& name, const std::string& category = "General");
        ActionResult addCompany(const std::string& name, const std::string& description = "");
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
