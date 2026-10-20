#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

#include "internpath/core/AppState.h"
#include "internpath/core/Persistence.h"
#include "internpath/core/ServiceContracts.h"

using namespace internpath;

namespace
{
    std::string modeToString(WorkMode mode)
    {
        switch (mode)
        {
            case WorkMode::Remote: return "Remote";
            case WorkMode::Hybrid: return "Hybrid";
            case WorkMode::Onsite:
            default: return "Onsite";
        }
    }

    std::string appStatusToString(ApplicationStatus status)
    {
        switch (status)
        {
            case ApplicationStatus::Allocated: return "ALLOCATED";
            case ApplicationStatus::Waitlisted: return "WAITLISTED";
            case ApplicationStatus::Withdrawn: return "WITHDRAWN";
            case ApplicationStatus::Cancelled: return "CANCELLED";
            case ApplicationStatus::Submitted:
            default: return "SUBMITTED";
        }
    }

    std::string inStatusToString(InternshipStatus status)
    {
        switch (status)
        {
            case InternshipStatus::Closed: return "CLOSED";
            case InternshipStatus::Cancelled: return "CANCELLED";
            case InternshipStatus::Open:
            default: return "OPEN";
        }
    }

    void printBanner()
    {
        std::cout << "\n======================================================================\n";
        std::cout << "          InternPath — Terminal Management & Career Platform          \n";
        std::cout << "======================================================================\n";
    }

    void printMenu(StudentId activeStudentId, const AppState& state)
    {
        const Student* current = state.findStudent(activeStudentId);
        std::string studentName = current ? current->fullName : "None";

        std::cout << "\n[Active Candidate: " << studentName << " (ID: " << activeStudentId << ")]\n";
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << " 1. List Candidates               8. Submit Application\n";
        std::cout << " 2. Switch Candidate Profile      9. View My Applications\n";
        std::cout << " 3. View Candidate Skill Matrix  10. Withdraw Application\n";
        std::cout << " 4. Update Skill Level (0-3)     11. Close Application Round (Admin)\n";
        std::cout << " 5. Undo Last Profile Change     12. Inspect / Promote Waitlist (Admin)\n";
        std::cout << " 6. Browse Internship Catalogue  13. Save / Reload CSV Snapshot\n";
        std::cout << " 7. Ranked Recommendations       14. Register Candidate / Role / Company\n";
        std::cout << " 0. Exit Console\n";
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << "Enter option [0-14]: ";
    }

    void listStudents(const AppState& state)
    {
        std::cout << "\n--- Candidate Directory (" << state.students.size() << ") ---\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(28) << "Full Name"
                  << std::setw(32) << "University Email"
                  << "Status\n";
        std::cout << std::string(72, '-') << "\n";

        for (std::size_t i = 0; i < state.students.size(); i++)
        {
            const Student& s = state.students.at(i);
            std::cout << std::left << std::setw(6) << s.id
                      << std::setw(28) << s.fullName
                      << std::setw(32) << s.email
                      << (s.active ? "Active" : "Inactive") << "\n";
        }
    }

    void viewStudentSkills(StudentId studentId, const AppState& state)
    {
        const Student* s = state.findStudent(studentId);
        if (!s)
        {
            std::cout << "Candidate not found.\n";
            return;
        }

        std::cout << "\n--- Declared Skill Matrix for " << s->fullName << " ---\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(24) << "Skill"
                  << std::setw(18) << "Category"
                  << "Proficiency Level\n";
        std::cout << std::string(62, '-') << "\n";

        for (std::size_t i = 0; i < state.skills.size(); i++)
        {
            const Skill& sk = state.skills.at(i);
            int lvl = 0;
            if (sk.denseIndex < s->proficiency.size())
            {
                lvl = s->proficiency.at(sk.denseIndex);
            }

            std::string levelLabel = (lvl == 0) ? "0 (Unset)" :
                                     (lvl == 1) ? "1 (Novice)" :
                                     (lvl == 2) ? "2 (Competent)" : "3 (Proficient)";

            std::cout << std::left << std::setw(6) << sk.id
                      << std::setw(24) << sk.name
                      << std::setw(18) << sk.category
                      << levelLabel << "\n";
        }
    }

    void browseCatalogue(const AppState& state)
    {
        std::cout << "\n--- Verified Internship Catalogue (" << state.internships.size() << ") ---\n";

        for (std::size_t i = 0; i < state.internships.size(); i++)
        {
            const Internship& in = state.internships.at(i);
            const Company* comp = state.findCompany(in.companyId);

            std::cout << "\n[" << in.id << "] " << in.title << " @ " << (comp ? comp->name : "Unknown") << "\n";
            std::cout << "    Location: " << in.location << " | Mode: " << modeToString(in.mode)
                      << " | Stipend: LKR " << in.stipend << "/mo | Capacity: " << in.capacity
                      << " | Deadline: " << in.deadline << " | Status: " << inStatusToString(in.status) << "\n";
            std::cout << "    Mandatory & Preferred Criteria:\n";

            for (std::size_t r = 0; r < in.requirements.size(); r++)
            {
                const Requirement& req = in.requirements.at(r);
                const Skill* sk = state.findSkill(req.skillId);
                std::cout << "      • " << (sk ? sk->name : "Skill")
                          << ": Level >= " << static_cast<int>(req.requiredLevel)
                          << " (Weight: " << static_cast<int>(req.weight)
                          << (req.mandatory ? ", MANDATORY" : ", Optional") << ")\n";
            }
        }
    }

    void showRecommendations(StudentId studentId, StudentService& studentService, const AppState& state)
    {
        const Student* s = state.findStudent(studentId);
        if (!s)
        {
            std::cout << "Candidate not found.\n";
            return;
        }

        DynamicArray<Recommendation> recs = studentService.getRecommendations(studentId);
        std::cout << "\n--- Ranked Recommendations for " << s->fullName << " (" << recs.size() << ") ---\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(28) << "Role"
                  << std::setw(18) << "Company"
                  << std::setw(14) << "Exact Fit"
                  << std::setw(10) << "Match %"
                  << std::setw(12) << "Eligible"
                  << "Deadline\n";
        std::cout << std::string(96, '-') << "\n";

        for (std::size_t i = 0; i < recs.size(); i++)
        {
            const Recommendation& r = recs.at(i);
            const Internship* in = state.findInternship(r.internshipId);
            const Company* comp = in ? state.findCompany(in->companyId) : nullptr;

            std::string fitRatio = std::to_string(r.match.numerator) + "/" + std::to_string(r.match.denominator);

            std::cout << std::left << std::setw(6) << r.internshipId
                      << std::setw(28) << (in ? in->title : "Unknown")
                      << std::setw(18) << (comp ? comp->name : "Unknown")
                      << std::setw(14) << fitRatio
                      << std::setw(10) << (std::to_string(r.match.roundedPercent()) + "%")
                      << std::setw(12) << (r.match.eligible ? "YES" : "GAP")
                      << r.deadline << "\n";
        }
    }

    void showLearningPlan(StudentId studentId, InternshipId inId, StudentService& studentService, const AppState& state)
    {
        const Student* s = state.findStudent(studentId);
        const Internship* in = state.findInternship(inId);
        if (!s || !in)
        {
            std::cout << "Invalid candidate or internship ID.\n";
            return;
        }

        DynamicArray<LearningTask> plan = studentService.getLearningPlan(studentId, inId);
        std::cout << "\n--- Career Readiness Roadmap: " << in->title << " ---\n";

        if (plan.size() == 0)
        {
            std::cout << "Candidate is already fully qualified for all prerequisite and role requirements!\n";
            return;
        }

        std::cout << "Topologically scheduled learning milestones:\n";
        for (std::size_t i = 0; i < plan.size(); i++)
        {
            const LearningTask& t = plan.at(i);
            std::cout << "  Step " << (i + 1) << " [Phase " << t.stage << "]: "
                      << t.skillName << " (Current: Lvl " << static_cast<int>(t.currentLevel)
                      << " -> Target: Lvl " << static_cast<int>(t.targetLevel) << ")\n";
        }
    }

    void showMyApplications(StudentId studentId, const AppState& state)
    {
        const Student* s = state.findStudent(studentId);
        if (!s)
        {
            std::cout << "Candidate not found.\n";
            return;
        }

        std::cout << "\n--- Submitted Applications for " << s->fullName << " ---\n";
        std::cout << std::left << std::setw(8) << "App ID"
                  << std::setw(28) << "Internship Title"
                  << std::setw(18) << "Company"
                  << std::setw(10) << "Sequence"
                  << std::setw(12) << "Submit Fit"
                  << "Status\n";
        std::cout << std::string(88, '-') << "\n";

        std::size_t count = 0;
        for (std::size_t i = 0; i < state.applications.size(); i++)
        {
            const Application& app = state.applications.at(i);
            if (app.studentId == studentId)
            {
                count++;
                const Internship* in = state.findInternship(app.internshipId);
                const Company* comp = in ? state.findCompany(in->companyId) : nullptr;
                std::string fit = std::to_string(app.fitNumeratorAtSubmit) + "/" + std::to_string(app.fitDenominatorAtSubmit);

                std::cout << std::left << std::setw(8) << app.id
                          << std::setw(28) << (in ? in->title : "Unknown")
                          << std::setw(18) << (comp ? comp->name : "Unknown")
                          << std::setw(10) << ("#" + std::to_string(app.submitSequence))
                          << std::setw(12) << fit
                          << appStatusToString(app.status) << "\n";
            }
        }

        if (count == 0)
        {
            std::cout << "No applications submitted yet.\n";
        }
    }
}

int main(int argc, char* argv[])
{
    AppState state;
    StudentService studentService(state);
    AdminService adminService(state);
    ApplicationService appService(state);

    // Locate and load snapshot
    std::string dataDir = "data/seed";
    if (!std::filesystem::exists(dataDir) && std::filesystem::exists("../data/seed"))
    {
        dataDir = "../data/seed";
    }

    if (std::filesystem::exists(dataDir))
    {
        CsvStorage::loadSnapshot(state, dataDir);
    }

    // CLI batch command handling
    if (argc > 1)
    {
        std::string cmd = argv[1];

        if (cmd == "--recommend" && argc > 2)
        {
            StudentId sId = static_cast<StudentId>(std::stoul(argv[2]));
            showRecommendations(sId, studentService, state);
            return 0;
        }
        else if (cmd == "--plan" && argc > 3)
        {
            StudentId sId = static_cast<StudentId>(std::stoul(argv[2]));
            InternshipId inId = static_cast<InternshipId>(std::stoul(argv[3]));
            showLearningPlan(sId, inId, studentService, state);
            return 0;
        }
        else if (cmd == "--catalogue")
        {
            browseCatalogue(state);
            return 0;
        }
        else if (cmd == "--help")
        {
            std::cout << "InternPath CLI Usage:\n";
            std::cout << "  internpath_cli.exe                       (interactive mode)\n";
            std::cout << "  internpath_cli.exe --catalogue           (list all internships)\n";
            std::cout << "  internpath_cli.exe --recommend <sId>     (view ranked recommendations)\n";
            std::cout << "  internpath_cli.exe --plan <sId> <inId>   (view career learning roadmap)\n";
            return 0;
        }
    }

    // Interactive Console Mode
    printBanner();
    std::cout << "Loaded dataset from: " << dataDir << " (" << state.students.size() << " students, "
              << state.internships.size() << " internships, " << state.skills.size() << " skills)\n";

    StudentId activeStudentId = 1;
    if (state.students.size() > 0)
    {
        activeStudentId = state.students.at(0).id;
    }

    bool running = true;
    while (running)
    {
        printMenu(activeStudentId, state);
        int choice = -1;
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::string discard;
            std::cin >> discard;
            continue;
        }

        switch (choice)
        {
            case 0:
            {
                std::cout << "Exiting InternPath CLI. Goodbye!\n";
                running = false;
                break;
            }
            case 1:
            {
                listStudents(state);
                break;
            }
            case 2:
            {
                listStudents(state);
                std::cout << "\nEnter Candidate ID to activate: ";
                StudentId id = 0;
                if (std::cin >> id && state.findStudent(id))
                {
                    activeStudentId = id;
                    std::cout << "Active profile switched to candidate ID: " << id << "\n";
                }
                else
                {
                    std::cout << "Invalid candidate ID.\n";
                }
                break;
            }
            case 3:
            {
                viewStudentSkills(activeStudentId, state);
                break;
            }
            case 4:
            {
                viewStudentSkills(activeStudentId, state);
                std::cout << "\nEnter Skill ID: ";
                SkillId skillId = 0;
                std::cin >> skillId;
                std::cout << "Enter Proficiency Level [0=Unset, 1=Novice, 2=Competent, 3=Proficient]: ";
                int level = 0;
                std::cin >> level;

                ActionResult res = studentService.setSkillLevel(activeStudentId, skillId, level);
                if (res.ok)
                {
                    std::cout << "✓ " << res.message << " (Change recorded in undo history)\n";
                }
                else
                {
                    std::cout << "✗ Error: " << res.message << "\n";
                }
                break;
            }
            case 5:
            {
                ActionResult res = studentService.undoLastProfileEdit(activeStudentId);
                if (res.ok)
                {
                    std::cout << "✓ " << res.message << " (Last skill change reverted)\n";
                }
                else
                {
                    std::cout << "✗ " << res.message << "\n";
                }
                break;
            }
            case 6:
            {
                browseCatalogue(state);
                break;
            }
            case 7:
            {
                showRecommendations(activeStudentId, studentService, state);
                break;
            }
            case 8:
            {
                browseCatalogue(state);
                std::cout << "\nEnter Internship ID to apply: ";
                InternshipId inId = 0;
                std::cin >> inId;

                ActionResult res = appService.apply(activeStudentId, inId);
                if (res.ok)
                {
                    std::cout << "✓ Application submitted successfully! (App ID: " << res.createdId << ")\n";
                }
                else
                {
                    std::cout << "✗ Application rejected: " << res.message << "\n";
                    std::cout << "Would you like to inspect the learning roadmap to bridge gaps? [y/n]: ";
                    char ans;
                    std::cin >> ans;
                    if (ans == 'y' || ans == 'Y')
                    {
                        showLearningPlan(activeStudentId, inId, studentService, state);
                    }
                }
                break;
            }
            case 9:
            {
                showMyApplications(activeStudentId, state);
                break;
            }
            case 10:
            {
                showMyApplications(activeStudentId, state);
                std::cout << "\nEnter Application ID to withdraw: ";
                ApplicationId appId = 0;
                std::cin >> appId;

                ActionResult res = appService.withdraw(appId);
                if (res.ok)
                {
                    std::cout << "✓ Application withdrawn. If you were allocated, the next waitlisted candidate has been automatically promoted.\n";
                }
                else
                {
                    std::cout << "✗ Withdrawal error: " << res.message << "\n";
                }
                break;
            }
            case 11:
            {
                browseCatalogue(state);
                std::cout << "\nEnter Internship ID to close round: ";
                InternshipId inId = 0;
                std::cin >> inId;

                ActionResult res = adminService.closeRound(inId);
                if (res.ok)
                {
                    std::cout << "✓ " << res.message << "\n";
                }
                else
                {
                    std::cout << "✗ Error closing round: " << res.message << "\n";
                }
                break;
            }
            case 12:
            {
                browseCatalogue(state);
                std::cout << "\nEnter Internship ID to inspect waiting list: ";
                InternshipId inId = 0;
                std::cin >> inId;

                WaitingList* wl = state.getWaitingList(inId);
                if (!wl || wl->size() == 0)
                {
                    std::cout << "Waiting list queue is currently empty.\n";
                }
                else
                {
                    DynamicArray<ApplicationId> snapshot = wl->snapshot();
                    std::cout << "\nRanked Waiting List Queue (" << snapshot.size() << " candidates):\n";
                    for (std::size_t i = 0; i < snapshot.size(); i++)
                    {
                        ApplicationId aId = snapshot.at(i);
                        const Application* app = state.findApplication(aId);
                        const Student* s = app ? state.findStudent(app->studentId) : nullptr;
                        std::cout << "  Rank #" << (i + 1) << ": " << (s ? s->fullName : "Unknown")
                                  << " (Candidate ID: " << (app ? app->studentId : 0)
                                  << ", App ID: " << aId << ")\n";
                    }

                    std::cout << "\nPromote next candidate from head of queue? [y/n]: ";
                    char ans;
                    std::cin >> ans;
                    if (ans == 'y' || ans == 'Y')
                    {
                        ApplicationId promotedId = 0;
                        if (wl->promoteHead(promotedId))
                        {
                            Application* app = state.findApplication(promotedId);
                            if (app)
                            {
                                app->status = ApplicationStatus::Allocated;
                                const Student* s = state.findStudent(app->studentId);
                                std::cout << "✓ Promoted candidate " << (s ? s->fullName : "")
                                          << " (App ID: " << promotedId << ") to ALLOCATED status!\n";
                            }
                        }
                    }
                }
                break;
            }
            case 13:
            {
                std::cout << "1. Save snapshot to disk (" << dataDir << ")\n";
                std::cout << "2. Reload snapshot from disk (" << dataDir << ")\n";
                std::cout << "Enter option [1/2]: ";
                int sub = 0;
                std::cin >> sub;
                if (sub == 1)
                {
                    ActionResult res = CsvStorage::saveSnapshot(state, dataDir);
                    std::cout << (res.ok ? "✓ Snapshot saved cleanly to CSV." : ("✗ " + res.message)) << "\n";
                }
                else if (sub == 2)
                {
                    ActionResult res = CsvStorage::loadSnapshot(state, dataDir);
                    std::cout << (res.ok ? "✓ Snapshot reloaded from CSV." : ("✗ " + res.message)) << "\n";
                }
                break;
            }
            case 14:
            {
                std::cout << "1. Register New Candidate\n";
                std::cout << "2. Register Partner Company\n";
                std::cout << "Enter option [1/2]: ";
                int sub = 0;
                std::cin >> sub;
                if (sub == 1)
                {
                    std::cin.ignore();
                    std::cout << "Full Name: ";
                    std::string name;
                    std::getline(std::cin, name);
                    std::cout << "Email: ";
                    std::string email;
                    std::getline(std::cin, email);

                    ActionResult res = studentService.registerStudent(name, email);
                    if (res.ok)
                    {
                        std::cout << "✓ Candidate registered! New ID: " << res.createdId << "\n";
                        activeStudentId = res.createdId;
                    }
                    else
                    {
                        std::cout << "✗ " << res.message << "\n";
                    }
                }
                else if (sub == 2)
                {
                    std::cin.ignore();
                    std::cout << "Company Name: ";
                    std::string name;
                    std::getline(std::cin, name);
                    std::cout << "Description: ";
                    std::string desc;
                    std::getline(std::cin, desc);

                    ActionResult res = adminService.addCompany(name, desc);
                    if (res.ok)
                    {
                        std::cout << "✓ Company registered! New ID: " << res.createdId << "\n";
                    }
                    else
                    {
                        std::cout << "✗ " << res.message << "\n";
                    }
                }
                break;
            }
            default:
            {
                std::cout << "Invalid option. Please choose between 0 and 14.\n";
                break;
            }
        }
    }

    return 0;
}
