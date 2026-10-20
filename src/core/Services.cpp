#include "internpath/core/ServiceContracts.h"

#include <algorithm>

#include "internpath/algo/GraphAlgorithms.h"
#include "internpath/algo/Sorting.h"
#include "internpath/domain/Date.h"
#include "internpath/domain/Matching.h"

namespace internpath
{
    // ==========================================
    // StudentService
    // ==========================================

    ActionResult StudentService::registerStudent(const std::string& name,
                                                 const std::string& email)
    {
        if (name.empty() || email.empty())
        {
            return ActionResult::failure(ErrorCode::InvalidInput, "Name and email cannot be empty");
        }

        StudentId id = state_.nextStudentId++;
        Student student;
        student.id = id;
        student.fullName = name;
        student.email = email;
        student.active = true;

        for (std::size_t i = 0; i < state_.skills.size(); i++)
        {
            student.proficiency.pushBack(0);
        }

        std::uint32_t denseIdx = static_cast<std::uint32_t>(state_.students.size());
        state_.students.pushBack(std::move(student));
        state_.studentIndexes.insertOrAssign(id, denseIdx);
        state_.undoHistories.pushBack(UndoHistory{});

        return ActionResult::success("Student registered successfully", id);
    }

    ActionResult StudentService::setSkillLevel(StudentId studentId, SkillId skillId, int level)
    {
        if (level < 0 || level > 3)
        {
            return ActionResult::failure(ErrorCode::InvalidInput, "Proficiency level must be between 0 and 3");
        }

        Student* student = state_.findStudent(studentId);

        if (student == nullptr)
        {
            return ActionResult::failure(ErrorCode::NotFound, "Student not found");
        }

        Skill* skill = state_.findSkill(skillId);

        if (skill == nullptr)
        {
            return ActionResult::failure(ErrorCode::NotFound, "Skill not found");
        }

        while (student->proficiency.size() <= skill->denseIndex)
        {
            student->proficiency.pushBack(0);
        }

        std::uint8_t oldLevel = student->proficiency.at(skill->denseIndex);
        student->proficiency.at(skill->denseIndex) = static_cast<std::uint8_t>(level);

        UndoHistory* undo = state_.getUndoHistory(studentId);

        if (undo != nullptr)
        {
            undo->recordEdit({studentId, skillId, oldLevel, static_cast<std::uint8_t>(level)});
        }

        return ActionResult::success("Skill level updated");
    }

    ActionResult StudentService::undoLastProfileEdit(StudentId studentId)
    {
        UndoHistory* undo = state_.getUndoHistory(studentId);

        if (undo == nullptr || undo->empty())
        {
            return ActionResult::failure(ErrorCode::NotFound, "No profile edits to undo");
        }

        ProfileEdit edit;
        undo->popUndo(edit);

        Student* student = state_.findStudent(studentId);
        Skill* skill = state_.findSkill(edit.skillId);

        if (student != nullptr && skill != nullptr && student->proficiency.size() > skill->denseIndex)
        {
            student->proficiency.at(skill->denseIndex) = edit.oldLevel;
        }

        return ActionResult::success("Profile edit undone");
    }

    DynamicArray<Recommendation> StudentService::getRecommendations(StudentId studentId)
    {
        DynamicArray<Recommendation> recommendations;
        Student* student = state_.findStudent(studentId);

        if (student == nullptr)
        {
            return recommendations;
        }

        for (std::size_t i = 0; i < state_.internships.size(); i++)
        {
            const Internship& internship = state_.internships.at(i);

            if (internship.status != InternshipStatus::Open)
            {
                continue;
            }

            DynamicArray<SkillComparison> comparisons;

            for (std::size_t r = 0; r < internship.requirements.size(); r++)
            {
                const Requirement& req = internship.requirements.at(r);
                Skill* skill = state_.findSkill(req.skillId);
                std::uint8_t studentLevel = 0;

                if (skill != nullptr && skill->denseIndex < student->proficiency.size())
                {
                    studentLevel = student->proficiency.at(skill->denseIndex);
                }

                comparisons.pushBack({studentLevel, req.requiredLevel, req.weight, req.mandatory});
            }

            MatchResult match = calculateMatch(comparisons.data(), comparisons.size());
            recommendations.pushBack({internship.id, match, internship.deadline});
        }

        auto comesBefore = [](const Recommendation& a, const Recommendation& b)
        {
            // 1. Eligible first
            if (a.match.eligible && !b.match.eligible)
            {
                return true;
            }

            if (!a.match.eligible && b.match.eligible)
            {
                return false;
            }

            // 2. Fit descending (exact integer cross-multiplication)
            int fitCmp = compareFit(a.match, b.match);

            if (fitCmp > 0)
            {
                return true;
            }

            if (fitCmp < 0)
            {
                return false;
            }

            // 3. Deadline ascending
            if (a.deadline != b.deadline)
            {
                return a.deadline < b.deadline;
            }

            // 4. ID ascending
            return a.internshipId < b.internshipId;
        };

        if (recommendations.size() > 1)
        {
            mergeSort(recommendations.data(), recommendations.size(), comesBefore);
        }

        return recommendations;
    }

    DynamicArray<LearningTask> StudentService::getLearningPlan(StudentId studentId,
                                                              InternshipId internshipId)
    {
        DynamicArray<LearningTask> plan;
        Student* student = state_.findStudent(studentId);
        Internship* internship = state_.findInternship(internshipId);

        if (student == nullptr || internship == nullptr)
        {
            return plan;
        }

        std::uint32_t numSkills = static_cast<std::uint32_t>(state_.skills.size());
        DynamicArray<std::uint8_t> demandedLevels;

        for (std::uint32_t i = 0; i < numSkills; i++)
        {
            demandedLevels.pushBack(0);
        }

        for (std::size_t r = 0; r < internship->requirements.size(); r++)
        {
            const Requirement& req = internship->requirements.at(r);
            Skill* skill = state_.findSkill(req.skillId);

            if (skill != nullptr && skill->denseIndex < numSkills)
            {
                if (req.requiredLevel > demandedLevels.at(skill->denseIndex))
                {
                    demandedLevels.at(skill->denseIndex) = req.requiredLevel;
                }
            }
        }

        DynamicArray<std::uint32_t> topoOrder;
        bool validTopo = topologicalOrder(state_.skillPrerequisites, topoOrder);

        if (!validTopo || topoOrder.size() != numSkills)
        {
            topoOrder.clear();
            for (std::uint32_t i = 0; i < numSkills; i++)
            {
                topoOrder.pushBack(i);
            }
        }

        // Reverse propagation of minimum prerequisite requirements
        for (std::size_t i = topoOrder.size(); i > 0; i--)
        {
            std::uint32_t v = topoOrder.at(i - 1);
            std::uint8_t demandedAtV = demandedLevels.at(v);

            if (demandedAtV == 0)
            {
                continue;
            }

            auto* edgeNode = state_.skillPrerequisites.incoming(v).firstNode();

            while (edgeNode != nullptr)
            {
                std::uint32_t u = edgeNode->value.toIndex;
                std::uint8_t edgeMin = edgeNode->value.minimumLevel;

                if (edgeMin > demandedLevels.at(u))
                {
                    demandedLevels.at(u) = edgeMin;
                }

                edgeNode = edgeNode->next;
            }
        }

        // Output learning tasks in topological order
        std::uint32_t currentStage = 1;

        for (std::size_t i = 0; i < topoOrder.size(); i++)
        {
            std::uint32_t skillIdx = topoOrder.at(i);
            std::uint8_t needed = demandedLevels.at(skillIdx);

            if (needed == 0)
            {
                continue;
            }

            std::uint8_t currentLvl = 0;

            if (skillIdx < student->proficiency.size())
            {
                currentLvl = student->proficiency.at(skillIdx);
            }

            if (currentLvl < needed)
            {
                Skill& sk = state_.skills.at(skillIdx);
                plan.pushBack({sk.id, sk.name, needed, currentLvl, currentStage++});
            }
        }

        return plan;
    }

    // ==========================================
    // AdminService
    // ==========================================

    ActionResult AdminService::addSkill(const std::string& name, const std::string& category)
    {
        if (name.empty())
        {
            return ActionResult::failure(ErrorCode::InvalidInput, "Skill name cannot be empty");
        }

        SkillId id = state_.nextSkillId++;
        std::uint32_t denseIdx = static_cast<std::uint32_t>(state_.skills.size());

        Skill skill;
        skill.id = id;
        skill.name = name;
        skill.category = category;
        skill.denseIndex = denseIdx;
        skill.active = true;

        state_.skills.pushBack(std::move(skill));
        state_.skillIndexes.insertOrAssign(id, denseIdx);
        state_.skillPrerequisites.addVertex();

        for (std::size_t i = 0; i < state_.students.size(); i++)
        {
            state_.students.at(i).proficiency.pushBack(0);
        }

        return ActionResult::success("Skill added successfully", id);
    }

    ActionResult AdminService::addCompany(const std::string& name, const std::string& description)
    {
        if (name.empty())
        {
            return ActionResult::failure(ErrorCode::InvalidInput, "Company name cannot be empty");
        }

        CompanyId id = state_.nextCompanyId++;
        std::uint32_t denseIdx = static_cast<std::uint32_t>(state_.companies.size());

        Company company;
        company.id = id;
        company.name = name;
        company.description = description;
        company.active = true;

        state_.companies.pushBack(std::move(company));
        state_.companyIndexes.insertOrAssign(id, denseIdx);

        return ActionResult::success("Company added successfully", id);
    }

    ActionResult AdminService::addPrerequisite(SkillId first, SkillId next, int minimumLevel)
    {
        Skill* s1 = state_.findSkill(first);
        Skill* s2 = state_.findSkill(next);

        if (s1 == nullptr || s2 == nullptr)
        {
            return ActionResult::failure(ErrorCode::NotFound, "Prerequisite skill not found");
        }

        bool ok = state_.skillPrerequisites.addEdge(s1->denseIndex, s2->denseIndex,
                                                    static_cast<std::uint8_t>(minimumLevel));

        if (!ok)
        {
            return ActionResult::failure(ErrorCode::Conflict, "Cycle or duplicate detected in prerequisites");
        }

        return ActionResult::success("Prerequisite relationship added");
    }

    ActionResult AdminService::publishInternship(Internship&& internship)
    {
        if (!isValidDate(internship.deadline))
        {
            return ActionResult::failure(ErrorCode::InvalidInput, "Invalid internship deadline date");
        }

        if (internship.capacity < 1)
        {
            internship.capacity = 1;
        }

        InternshipId id = state_.nextInternshipId++;
        internship.id = id;
        internship.status = InternshipStatus::Open;

        std::uint32_t denseIdx = static_cast<std::uint32_t>(state_.internships.size());
        state_.openDeadlines.insert({internship.deadline, id});
        state_.internships.pushBack(std::move(internship));
        state_.internshipIndexes.insertOrAssign(id, denseIdx);
        state_.waitlists.pushBack(WaitingList{});

        return ActionResult::success("Internship published successfully", id);
    }

    ActionResult AdminService::closeRound(InternshipId internshipId)
    {
        Internship* internship = state_.findInternship(internshipId);

        if (internship == nullptr)
        {
            return ActionResult::failure(ErrorCode::NotFound, "Internship not found");
        }

        if (internship->status != InternshipStatus::Open)
        {
            return ActionResult::failure(ErrorCode::Conflict, "Internship is not open");
        }

        internship->status = InternshipStatus::Closed;
        state_.openDeadlines.erase({internship->deadline, internshipId});

        DynamicArray<std::size_t> appIndices;

        for (std::size_t i = 0; i < state_.applications.size(); i++)
        {
            const Application& app = state_.applications.at(i);

            if (app.internshipId == internshipId && app.status == ApplicationStatus::Submitted)
            {
                appIndices.pushBack(i);
            }
        }

        auto comesBefore = [this](std::size_t idxA, std::size_t idxB)
        {
            const Application& a = state_.applications.at(idxA);
            const Application& b = state_.applications.at(idxB);

            // 1. Fit numerator DESC
            if (a.fitNumeratorAtSubmit != b.fitNumeratorAtSubmit)
            {
                return a.fitNumeratorAtSubmit > b.fitNumeratorAtSubmit;
            }

            // 2. Submit sequence ASC
            if (a.submitSequence != b.submitSequence)
            {
                return a.submitSequence < b.submitSequence;
            }

            // 3. ID ASC
            return a.id < b.id;
        };

        if (appIndices.size() > 1)
        {
            mergeSort(appIndices.data(), appIndices.size(), comesBefore);
        }

        WaitingList* waitlist = state_.getWaitingList(internshipId);
        std::uint32_t rank = 1;

        for (std::size_t i = 0; i < appIndices.size(); i++)
        {
            Application& app = state_.applications.at(appIndices.at(i));
            app.rankAtClose = rank++;

            if (i < internship->capacity)
            {
                app.status = ApplicationStatus::Allocated;
            }
            else
            {
                app.status = ApplicationStatus::Waitlisted;

                if (waitlist != nullptr)
                {
                    waitlist->append(app.id);
                }
            }
        }

        return ActionResult::success("Internship round closed and candidates allocated/waitlisted");
    }

    // ==========================================
    // ApplicationService
    // ==========================================

    ActionResult ApplicationService::apply(StudentId studentId, InternshipId internshipId)
    {
        Student* student = state_.findStudent(studentId);

        if (student == nullptr)
        {
            return ActionResult::failure(ErrorCode::NotFound, "Student not found");
        }

        Internship* internship = state_.findInternship(internshipId);

        if (internship == nullptr)
        {
            return ActionResult::failure(ErrorCode::NotFound, "Internship not found");
        }

        if (internship->status != InternshipStatus::Open)
        {
            return ActionResult::failure(ErrorCode::RoundClosed, "Internship round is closed");
        }

        if (internship->deadline < state_.demoToday)
        {
            return ActionResult::failure(ErrorCode::RoundClosed, "Application deadline has passed");
        }

        // Prevent duplicate active applications
        for (std::size_t i = 0; i < state_.applications.size(); i++)
        {
            const Application& existing = state_.applications.at(i);

            if (existing.studentId == studentId && existing.internshipId == internshipId)
            {
                if (existing.status == ApplicationStatus::Submitted ||
                    existing.status == ApplicationStatus::Allocated ||
                    existing.status == ApplicationStatus::Waitlisted)
                {
                    return ActionResult::failure(ErrorCode::Duplicate, "Active application already exists");
                }
            }
        }

        // Calculate fit and check mandatory eligibility
        DynamicArray<SkillComparison> comparisons;

        for (std::size_t r = 0; r < internship->requirements.size(); r++)
        {
            const Requirement& req = internship->requirements.at(r);
            Skill* skill = state_.findSkill(req.skillId);
            std::uint8_t studentLevel = 0;

            if (skill != nullptr && skill->denseIndex < student->proficiency.size())
            {
                studentLevel = student->proficiency.at(skill->denseIndex);
            }

            comparisons.pushBack({studentLevel, req.requiredLevel, req.weight, req.mandatory});
        }

        MatchResult match = calculateMatch(comparisons.data(), comparisons.size());

        if (!match.valid || !match.eligible)
        {
            return ActionResult::failure(ErrorCode::NotEligible, "Student does not meet mandatory requirements");
        }

        ApplicationId id = state_.nextApplicationId++;
        std::uint64_t seq = state_.nextSubmitSequence++;

        Application app;
        app.id = id;
        app.studentId = studentId;
        app.internshipId = internshipId;
        app.status = ApplicationStatus::Submitted;
        app.submitSequence = seq;
        app.fitNumeratorAtSubmit = match.numerator;
        app.fitDenominatorAtSubmit = match.denominator;
        app.rankAtClose = 0;

        std::uint32_t denseIdx = static_cast<std::uint32_t>(state_.applications.size());
        state_.applications.pushBack(std::move(app));
        state_.applicationIndexes.insertOrAssign(id, denseIdx);

        return ActionResult::success("Application submitted successfully", id);
    }

    ActionResult ApplicationService::withdraw(ApplicationId applicationId)
    {
        Application* app = state_.findApplication(applicationId);

        if (app == nullptr)
        {
            return ActionResult::failure(ErrorCode::NotFound, "Application not found");
        }

        if (app->status == ApplicationStatus::Withdrawn || app->status == ApplicationStatus::Cancelled)
        {
            return ActionResult::failure(ErrorCode::Conflict, "Application is not active");
        }

        if (app->status == ApplicationStatus::Waitlisted)
        {
            app->status = ApplicationStatus::Withdrawn;
            WaitingList* waitlist = state_.getWaitingList(app->internshipId);

            if (waitlist != nullptr)
            {
                waitlist->withdraw(app->id);
            }

            return ActionResult::success("Waitlisted candidate withdrawn");
        }

        if (app->status == ApplicationStatus::Allocated)
        {
            app->status = ApplicationStatus::Withdrawn;
            WaitingList* waitlist = state_.getWaitingList(app->internshipId);

            if (waitlist != nullptr && !waitlist->empty())
            {
                ApplicationId promotedId = 0;

                if (waitlist->promoteHead(promotedId))
                {
                    Application* promoted = state_.findApplication(promotedId);

                    if (promoted != nullptr)
                    {
                        promoted->status = ApplicationStatus::Allocated;
                    }
                }
            }

            return ActionResult::success("Allocated candidate withdrawn and waitlist promoted");
        }

        app->status = ApplicationStatus::Withdrawn;
        return ActionResult::success("Application withdrawn");
    }
}
