#pragma once

#include <cstdint>

#include "internpath/core/UndoHistory.h"
#include "internpath/core/WaitingList.h"
#include "internpath/domain/Models.h"
#include "internpath/ds/DeadlineAVL.h"
#include "internpath/ds/DirectedGraph.h"
#include "internpath/ds/HashTable.h"

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

        DynamicArray<WaitingList> waitlists;
        DynamicArray<UndoHistory> undoHistories;

        DeadlineAVL openDeadlines;
        DirectedGraph skillPrerequisites;

        StudentId nextStudentId = 1;
        SkillId nextSkillId = 1;
        CompanyId nextCompanyId = 1;
        InternshipId nextInternshipId = 1;
        ApplicationId nextApplicationId = 1;
        std::uint64_t nextSubmitSequence = 1;
        std::int32_t demoToday = 20261020;

        Student* findStudent(StudentId id)
        {
            std::uint32_t index = 0;

            if (studentIndexes.tryGet(id, index) && index < students.size())
            {
                return &students.at(index);
            }

            return nullptr;
        }

        const Student* findStudent(StudentId id) const
        {
            std::uint32_t index = 0;

            if (studentIndexes.tryGet(id, index) && index < students.size())
            {
                return &students.at(index);
            }

            return nullptr;
        }

        Skill* findSkill(SkillId id)
        {
            std::uint32_t index = 0;

            if (skillIndexes.tryGet(id, index) && index < skills.size())
            {
                return &skills.at(index);
            }

            return nullptr;
        }

        const Skill* findSkill(SkillId id) const
        {
            std::uint32_t index = 0;

            if (skillIndexes.tryGet(id, index) && index < skills.size())
            {
                return &skills.at(index);
            }

            return nullptr;
        }

        Company* findCompany(CompanyId id)
        {
            std::uint32_t index = 0;

            if (companyIndexes.tryGet(id, index) && index < companies.size())
            {
                return &companies.at(index);
            }

            return nullptr;
        }

        const Company* findCompany(CompanyId id) const
        {
            std::uint32_t index = 0;

            if (companyIndexes.tryGet(id, index) && index < companies.size())
            {
                return &companies.at(index);
            }

            return nullptr;
        }

        Internship* findInternship(InternshipId id)
        {
            std::uint32_t index = 0;

            if (internshipIndexes.tryGet(id, index) && index < internships.size())
            {
                return &internships.at(index);
            }

            return nullptr;
        }

        const Internship* findInternship(InternshipId id) const
        {
            std::uint32_t index = 0;

            if (internshipIndexes.tryGet(id, index) && index < internships.size())
            {
                return &internships.at(index);
            }

            return nullptr;
        }

        Application* findApplication(ApplicationId id)
        {
            std::uint32_t index = 0;

            if (applicationIndexes.tryGet(id, index) && index < applications.size())
            {
                return &applications.at(index);
            }

            return nullptr;
        }

        const Application* findApplication(ApplicationId id) const
        {
            std::uint32_t index = 0;

            if (applicationIndexes.tryGet(id, index) && index < applications.size())
            {
                return &applications.at(index);
            }

            return nullptr;
        }

        WaitingList* getWaitingList(InternshipId id)
        {
            std::uint32_t index = 0;

            if (internshipIndexes.tryGet(id, index))
            {
                while (waitlists.size() <= index)
                {
                    waitlists.pushBack(WaitingList{});
                }

                return &waitlists.at(index);
            }

            return nullptr;
        }

        UndoHistory* getUndoHistory(StudentId id)
        {
            std::uint32_t index = 0;

            if (studentIndexes.tryGet(id, index))
            {
                while (undoHistories.size() <= index)
                {
                    undoHistories.pushBack(UndoHistory{});
                }

                return &undoHistories.at(index);
            }

            return nullptr;
        }

        void rebuildIndexes()
        {
            studentIndexes.clear();
            for (std::size_t i = 0; i < students.size(); i++)
            {
                studentIndexes.insertOrAssign(students.at(i).id, static_cast<std::uint32_t>(i));
            }

            skillIndexes.clear();
            for (std::size_t i = 0; i < skills.size(); i++)
            {
                skills.at(i).denseIndex = static_cast<std::uint32_t>(i);
                skillIndexes.insertOrAssign(skills.at(i).id, static_cast<std::uint32_t>(i));
            }

            companyIndexes.clear();
            for (std::size_t i = 0; i < companies.size(); i++)
            {
                companyIndexes.insertOrAssign(companies.at(i).id, static_cast<std::uint32_t>(i));
            }

            internshipIndexes.clear();
            openDeadlines.clear();
            for (std::size_t i = 0; i < internships.size(); i++)
            {
                internshipIndexes.insertOrAssign(internships.at(i).id, static_cast<std::uint32_t>(i));

                if (internships.at(i).status == InternshipStatus::Open)
                {
                    openDeadlines.insert({internships.at(i).deadline, internships.at(i).id});
                }
            }

            applicationIndexes.clear();
            for (std::size_t i = 0; i < applications.size(); i++)
            {
                applicationIndexes.insertOrAssign(applications.at(i).id, static_cast<std::uint32_t>(i));
            }
        }

        void clear()
        {
            students.clear();
            skills.clear();
            companies.clear();
            internships.clear();
            applications.clear();
            studentIndexes.clear();
            skillIndexes.clear();
            companyIndexes.clear();
            internshipIndexes.clear();
            applicationIndexes.clear();
            waitlists.clear();
            undoHistories.clear();
            openDeadlines.clear();

            nextStudentId = 1;
            nextSkillId = 1;
            nextCompanyId = 1;
            nextInternshipId = 1;
            nextApplicationId = 1;
            nextSubmitSequence = 1;
        }
    };
}
