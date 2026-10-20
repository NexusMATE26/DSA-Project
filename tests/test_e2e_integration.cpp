#include <iostream>

#include "TestChecks.h"
#include "internpath/core/AppState.h"
#include "internpath/core/Persistence.h"
#include "internpath/core/ServiceContracts.h"

using namespace internpath;

int main()
{
    AppState state;
    StudentService studentService(state);
    AdminService adminService(state);
    ApplicationService appService(state);

    // ==========================================
    // 1. REGISTER
    // ==========================================
    auto s1 = studentService.registerStudent("Alice Smith", "alice@example.com");
    auto s2 = studentService.registerStudent("Bob Jones", "bob@example.com");
    auto s3 = studentService.registerStudent("Charlie Brown", "charlie@example.com");

    CHECK(s1.ok && s1.createdId == 1);
    CHECK(s2.ok && s2.createdId == 2);
    CHECK(s3.ok && s3.createdId == 3);
    CHECK(state.students.size() == 3);

    auto sk1 = adminService.addSkill("C++", "Programming");
    auto sk2 = adminService.addSkill("Data Structures", "Programming");
    auto sk3 = adminService.addSkill("Git", "Tools");

    CHECK(sk1.ok && sk1.createdId == 1);
    CHECK(sk2.ok && sk2.createdId == 2);
    CHECK(sk3.ok && sk3.createdId == 3);
    CHECK(state.skills.size() == 3);

    auto c1 = adminService.addCompany("Innovax Labs", "Embedded systems and edge AI");
    CHECK(c1.ok && c1.createdId == 1);
    CHECK(state.companies.size() == 1);

    // Prerequisite: C++ (1) -> Data Structures (2), minLevel 2
    auto prereq = adminService.addPrerequisite(1, 2, 2);
    CHECK(prereq.ok);

    // Publish Internship: capacity = 1, deadline = 20261115
    Internship intern;
    intern.companyId = 1;
    intern.title = "Systems Intern";
    intern.location = "Colombo";
    intern.mode = WorkMode::Hybrid;
    intern.stipend = 30000;
    intern.capacity = 1;
    intern.deadline = 20261115;
    intern.requirements.pushBack({1, 2, 5, true});  // C++: lvl 2, weight 5, mandatory
    intern.requirements.pushBack({2, 2, 4, true});  // DS: lvl 2, weight 4, mandatory
    intern.requirements.pushBack({3, 1, 2, false}); // Git: lvl 1, weight 2, optional

    auto pub = adminService.publishInternship(std::move(intern));
    CHECK(pub.ok && pub.createdId == 1);
    InternshipId internId = pub.createdId;
    CHECK(state.internships.size() == 1);

    // ==========================================
    // 2. SKILLS & PROFILE UNDO
    // ==========================================
    // Alice sets skills
    CHECK(studentService.setSkillLevel(1, 1, 2).ok); // C++ = 2
    CHECK(studentService.setSkillLevel(1, 2, 2).ok); // DS = 2
    CHECK(studentService.setSkillLevel(1, 3, 1).ok); // Git = 1

    // Test Undo for Alice
    CHECK(studentService.undoLastProfileEdit(1).ok); // Git reverts to 0
    CHECK(state.students.at(0).proficiency.at(2) == 0);
    CHECK(studentService.setSkillLevel(1, 3, 1).ok); // Git set back to 1
    CHECK(state.students.at(0).proficiency.at(2) == 1);

    // Bob sets skills
    CHECK(studentService.setSkillLevel(2, 1, 2).ok); // C++ = 2
    CHECK(studentService.setSkillLevel(2, 2, 2).ok); // DS = 2
    CHECK(studentService.setSkillLevel(2, 3, 1).ok); // Git = 1

    // Charlie sets skills (C++ = 1, not eligible for Systems Intern)
    CHECK(studentService.setSkillLevel(3, 1, 1).ok); // C++ = 1
    CHECK(studentService.setSkillLevel(3, 2, 0).ok); // DS = 0

    // ==========================================
    // 3. RECOMMEND
    // ==========================================
    auto recsAlice = studentService.getRecommendations(1);
    CHECK(recsAlice.size() == 1);
    CHECK(recsAlice.at(0).internshipId == internId);
    CHECK(recsAlice.at(0).match.eligible);
    CHECK(recsAlice.at(0).match.numerator == 66);
    CHECK(recsAlice.at(0).match.denominator == 66);
    CHECK(recsAlice.at(0).match.roundedPercent() == 100);

    auto recsCharlie = studentService.getRecommendations(3);
    CHECK(recsCharlie.size() == 1);
    CHECK(!recsCharlie.at(0).match.eligible);

    // ==========================================
    // 4. LEARNING PLAN
    // ==========================================
    auto planCharlie = studentService.getLearningPlan(3, internId);
    CHECK(planCharlie.size() >= 2);
    // C++ must appear before Data Structures in the plan
    bool foundCpp = false;
    bool foundDs = false;

    for (std::size_t i = 0; i < planCharlie.size(); i++)
    {
        if (planCharlie.at(i).skillId == 1)
        {
            foundCpp = true;
            CHECK(!foundDs); // C++ must be scheduled before DS
        }

        if (planCharlie.at(i).skillId == 2)
        {
            foundDs = true;
            CHECK(foundCpp);
        }
    }

    CHECK(foundCpp && foundDs);

    // ==========================================
    // 5. APPLY
    // ==========================================
    // Alice applies
    auto a1 = appService.apply(1, internId);
    CHECK(a1.ok && a1.createdId == 1);

    // Bob applies
    auto a2 = appService.apply(2, internId);
    CHECK(a2.ok && a2.createdId == 2);

    // Charlie applies (should be rejected because Charlie is not eligible)
    auto a3 = appService.apply(3, internId);
    CHECK(!a3.ok);
    CHECK(a3.error == ErrorCode::NotEligible);

    // Alice duplicate application attempt (should be rejected)
    auto aDup = appService.apply(1, internId);
    CHECK(!aDup.ok);
    CHECK(aDup.error == ErrorCode::Duplicate);

    CHECK(state.applications.size() == 2);
    CHECK(state.applications.at(0).fitNumeratorAtSubmit == 66);
    CHECK(state.applications.at(1).fitNumeratorAtSubmit == 66);

    // ==========================================
    // 6. CLOSE ROUND (Capacity = 1)
    // ==========================================
    CHECK(adminService.closeRound(internId).ok);

    // Internship status must be Closed
    CHECK(state.internships.at(0).status == InternshipStatus::Closed);

    // Alice (Seq 1) is Allocated, Bob (Seq 2) is Waitlisted
    CHECK(state.applications.at(0).status == ApplicationStatus::Allocated);
    CHECK(state.applications.at(0).rankAtClose == 1);
    CHECK(state.applications.at(1).status == ApplicationStatus::Waitlisted);
    CHECK(state.applications.at(1).rankAtClose == 2);

    // Bob should be in the waiting list
    WaitingList* wl = state.getWaitingList(internId);
    CHECK(wl != nullptr);
    CHECK(wl->size() == 1);
    CHECK(wl->contains(a2.createdId));

    // ==========================================
    // 7. WITHDRAW & PROMOTE
    // ==========================================
    // Alice withdraws from allocated position
    CHECK(appService.withdraw(a1.createdId).ok);
    CHECK(state.applications.at(0).status == ApplicationStatus::Withdrawn);

    // Bob must now be promoted from Waitlisted to Allocated!
    CHECK(state.applications.at(1).status == ApplicationStatus::Allocated);
    CHECK(wl->empty());

    // ==========================================
    // 8. SAVE SNAPSHOT
    // ==========================================
    std::string testSnapshotDir = "build/test_e2e_snapshot";
    auto saveRes = CsvStorage::saveSnapshot(state, testSnapshotDir);
    CHECK(saveRes.ok);

    // ==========================================
    // 9. RELOAD & INTEGRITY CHECK
    // ==========================================
    AppState reloadedState;
    auto loadRes = CsvStorage::loadSnapshot(reloadedState, testSnapshotDir);
    CHECK(loadRes.ok);

    CHECK(reloadedState.students.size() == 3);
    CHECK(reloadedState.skills.size() == 3);
    CHECK(reloadedState.companies.size() == 1);
    CHECK(reloadedState.internships.size() == 1);
    CHECK(reloadedState.applications.size() == 2);

    // Check Alice proficiency reloaded
    Student* reAlice = reloadedState.findStudent(1);
    CHECK(reAlice != nullptr);
    CHECK(reAlice->fullName == "Alice Smith");
    CHECK(reAlice->proficiency.at(0) == 2); // C++ = 2
    CHECK(reAlice->proficiency.at(1) == 2); // DS = 2
    CHECK(reAlice->proficiency.at(2) == 1); // Git = 1

    // Check application statuses reloaded
    CHECK(reloadedState.applications.at(0).status == ApplicationStatus::Withdrawn);
    CHECK(reloadedState.applications.at(1).status == ApplicationStatus::Allocated);

    // Check closed round is not in openDeadlines AVL tree
    DynamicArray<std::uint32_t> openIds;
    reloadedState.openDeadlines.range(20000101, 22000101, openIds);
    CHECK(openIds.empty());

    std::cout << "All E2E integration test assertions passed successfully!\n";
    return 0;
}
