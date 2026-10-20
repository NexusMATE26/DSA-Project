#include <cassert>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "bench/benchmark_common.h"
#include "internpath/core/AppState.h"
#include "internpath/core/Persistence.h"
#include "internpath/core/ServiceContracts.h"

using namespace internpath;
using namespace internpath::bench;

int main()
{
    if (!checkSystemSafety())
    {
        return 1;
    }

    BenchmarkRunner runner("Complete InternPath Application Workflows");
    runner.printHeader();

    std::mt19937 rng(42);

    // Test across scales: Level 1 (1k students), Level 2 (10k students)
    const std::vector<std::size_t> studentCounts = {1000, 10000};

    for (std::size_t numStudents : studentCounts)
    {
        std::string label = "Students=" + std::to_string(numStudents);

        AppState state;
        StudentService studentService(state);
        AdminService adminService(state);
        ApplicationService appService(state);

        // ---------------------------------------------------------
        // 1. Setup Skill & Prerequisite Graph (50 skills, 100 DAG edges)
        // ---------------------------------------------------------
        for (int s = 1; s <= 50; s++)
        {
            adminService.addSkill("Skill Spec " + std::to_string(s), "Engineering");
        }
        for (int e = 1; e <= 40; e++)
        {
            adminService.addPrerequisite(static_cast<SkillId>(e), static_cast<SkillId>(e + 5), 1);
        }

        // Add 50 Companies
        for (int c = 1; c <= 50; c++)
        {
            adminService.addCompany("Company Group " + std::to_string(c), "Engineering Division");
        }

        // Add 100 Internships
        for (int i = 1; i <= 100; i++)
        {
            Internship in;
            in.companyId = static_cast<CompanyId>((i % 50) + 1);
            in.title = "Software Engineering Intern " + std::to_string(i);
            in.capacity = 2;
            in.deadline = 20261115 + (i % 20);
            in.requirements.pushBack({1, 2, 5, true});  // Skill 1, Level 2, weight 5, mandatory
            in.requirements.pushBack({2, 1, 3, false}); // Skill 2, Level 1, weight 3, optional
            adminService.publishInternship(std::move(in));
        }

        // ---------------------------------------------------------
        // 2. Workflow: Student Registration & Profile Indexing
        // ---------------------------------------------------------
        runner.run("Workflow::register_students", label, numStudents, 1, [&]() {
            for (std::size_t s = 1; s <= numStudents; s++)
            {
                std::string name = "Student " + std::to_string(s);
                std::string email = "student" + std::to_string(s) + "@university.edu";
                ActionResult res = studentService.registerStudent(name, email);
                assert(res.ok);
            }
            assert(state.students.size() == numStudents);
        }, true, false);

        // ---------------------------------------------------------
        // 3. Workflow: Skill Level Updates & Undo Stack
        // ---------------------------------------------------------
        runner.run("Workflow::skill_edits_and_undo", label, numStudents, 1, [&]() {
            for (std::size_t s = 1; s <= numStudents; s++)
            {
                StudentId sid = static_cast<StudentId>(s);
                // Update skill 1 to level 2
                studentService.setSkillLevel(sid, 1, 2);
                // Update skill 2 to level 1
                studentService.setSkillLevel(sid, 2, 1);
                // Undo skill 2 edit via stack
                studentService.undoLastProfileEdit(sid);
            }
        }, true, false);

        // ---------------------------------------------------------
        // 4. Workflow: Recommendation Generation (Matching + MergeSort)
        // ---------------------------------------------------------
        runner.run("Workflow::get_recommendations", label, 200, 3, [&]() {
            for (int s = 1; s <= 200; s++)
            {
                StudentId sid = static_cast<StudentId>((s % numStudents) + 1);
                auto recs = studentService.getRecommendations(sid);
                assert(recs.size() == state.internships.size());
            }
        });

        // ---------------------------------------------------------
        // 5. Workflow: Prerequisite Learning Plan (Kahn's Sort + Propagation)
        // ---------------------------------------------------------
        runner.run("Workflow::get_learning_plan", label, 200, 3, [&]() {
            for (int s = 1; s <= 200; s++)
            {
                StudentId sid = static_cast<StudentId>((s % numStudents) + 1);
                auto plan = studentService.getLearningPlan(sid, 1);
                (void)plan;
            }
        });

        // ---------------------------------------------------------
        // 6. Workflow: Application Submission (Duplicate Check Scaling)
        // ---------------------------------------------------------
        std::size_t appTarget = (numStudents == 1000) ? 2000 : 5000;
        runner.run("Workflow::submit_applications", label, appTarget, 1, [&]() {
            for (std::size_t a = 1; a <= appTarget; a++)
            {
                StudentId sid = static_cast<StudentId>((a % numStudents) + 1);
                InternshipId inId = static_cast<InternshipId>((a % 50) + 1);
                appService.apply(sid, inId);
            }
        }, true, false);

        // ---------------------------------------------------------
        // 7. Workflow: Round Closure (Applicant Ranking & Allocation)
        // ---------------------------------------------------------
        runner.run("Workflow::close_round_allocation", label, 20, 1, [&]() {
            for (InternshipId inId = 1; inId <= 20; inId++)
            {
                ActionResult res = adminService.closeRound(inId);
                (void)res;
            }
        }, true, false);

        // ---------------------------------------------------------
        // 8. Workflow: Waitlist Unlink & Promotion
        // ---------------------------------------------------------
        runner.run("Workflow::waitlist_ops", label, 50, 1, [&]() {
            for (InternshipId inId = 1; inId <= 20; inId++)
            {
                WaitingList* wl = state.getWaitingList(inId);
                if (wl != nullptr && !wl->empty())
                {
                    ApplicationId promotedId = 0;
                    wl->promoteHead(promotedId);
                }
            }
        }, true, false);

        // ---------------------------------------------------------
        // 9. Workflow: CSV Serialization & Snapshot Reload
        // ---------------------------------------------------------
        std::string snapshotDir = "build/benchmark_data/temp_snapshot_" + std::to_string(numStudents);
        std::filesystem::create_directories(snapshotDir);

        runner.run("Workflow::save_csv_snapshot", label, 1, 1, [&]() {
            ActionResult res = CsvStorage::saveSnapshot(state, snapshotDir);
            assert(res.ok);
        }, true, false);

        AppState reloadedState;
        runner.run("Workflow::load_csv_snapshot", label, 1, 1, [&]() {
            ActionResult res = CsvStorage::loadSnapshot(reloadedState, snapshotDir);
            assert(res.ok);
            assert(reloadedState.students.size() == state.students.size());
            assert(reloadedState.internships.size() == state.internships.size());
        }, true, false);

        // Cleanup temporary snapshot
        std::filesystem::remove_all(snapshotDir);
    }

    runner.saveCsv("bench_workflows_results.csv");
    return 0;
}
