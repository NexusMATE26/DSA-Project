#define WIN32_LEAN_AND_MEAN
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include "internpath/api/HttpServer.h"
#include "internpath/core/AppState.h"
#include "internpath/core/Persistence.h"
#include "internpath/core/ServiceContracts.h"

using namespace internpath;
using json = nlohmann::json;

int main()
{
    std::cout << "[API Smoke Test] Starting in-memory server on 127.0.0.1:8089 ...\n";

    AppState state;
    StudentService studentService(state);
    AdminService adminService(state);
    ApplicationService appService(state);

    // Initialize with seed data if available
    std::string dataDir = "data/seed";
    if (!std::filesystem::exists(dataDir) && std::filesystem::exists("../data/seed"))
    {
        dataDir = "../data/seed";
    }
    if (std::filesystem::exists(dataDir))
    {
        CsvStorage::loadSnapshot(state, dataDir);
    }
    else
    {
        // Seed manually
        studentService.registerStudent("Test Student", "test@example.com");
        adminService.addCompany("Test Corp", "Tech");
        Internship in;
        in.companyId = 1;
        in.title = "Software Engineer Intern";
        in.deadline = 20261115;
        in.capacity = 1;
        adminService.publishInternship(std::move(in));
    }

    HttpServer server(state, studentService, adminService, appService, "", dataDir);

    // Run server in background thread
    std::thread serverThread([&server]()
    {
        server.start("127.0.0.1", 8089);
    });

    // Wait for server to bind
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    httplib::Client cli("127.0.0.1", 8089);
    cli.set_connection_timeout(2);
    cli.set_read_timeout(2);

    // 1. Health check
    {
        auto res = cli.Get("/api/health");
        assert(res && res->status == 200);
        auto j = json::parse(res->body);
        assert(j["ok"] == true);
        assert(j["name"] == "InternPath");
        std::cout << "  [PASS] GET /api/health\n";
    }

    // 2. Students list
    {
        auto res = cli.Get("/api/students");
        assert(res && res->status == 200);
        auto j = json::parse(res->body);
        assert(j.is_array());
        assert(!j.empty());
        std::cout << "  [PASS] GET /api/students (" << j.size() << " students)\n";
    }

    // 3. Register student
    StudentId newStudentId = 0;
    {
        json req = {
            {"name", "Jane Doe"},
            {"email", "jane@example.com"}
        };
        auto res = cli.Post("/api/students", req.dump(), "application/json");
        assert(res && res->status == 201);
        auto j = json::parse(res->body);
        assert(j["ok"] == true);
        newStudentId = j["id"].get<StudentId>();
        assert(newStudentId > 0);
        std::cout << "  [PASS] POST /api/students (created id=" << newStudentId << ")\n";
    }

    // 4. Update student profile and test undo on skills
    {
        json updateReq = {
            {"name", "Jane Updated"}
        };
        auto res = cli.Put("/api/students/" + std::to_string(newStudentId), updateReq.dump(), "application/json");
        assert(res && res->status == 200);

        // Verify update
        auto getRes = cli.Get("/api/students/" + std::to_string(newStudentId));
        assert(getRes && getRes->status == 200);
        auto j = json::parse(getRes->body);
        assert(j["name"] == "Jane Updated");

        // Set skill level to 3
        json skillReq = {
            {"skillId", 1},
            {"level", 3}
        };
        auto skillRes = cli.Put("/api/students/" + std::to_string(newStudentId) + "/skills", skillReq.dump(), "application/json");
        assert(skillRes && skillRes->status == 200);

        // Verify skill level is 3
        auto getSkillRes = cli.Get("/api/students/" + std::to_string(newStudentId));
        auto jSkill = json::parse(getSkillRes->body);
        bool foundSkill = false;
        for (const auto& sk : jSkill["skills"])
        {
            if (sk["skillId"] == 1)
            {
                assert(sk["level"] == 3);
                foundSkill = true;
                break;
            }
        }
        assert(foundSkill);

        // Undo skill edit
        auto undoRes = cli.Post("/api/students/" + std::to_string(newStudentId) + "/undo", "", "application/json");
        assert(undoRes && undoRes->status == 200);

        // Verify reverted to level 0
        auto revertedRes = cli.Get("/api/students/" + std::to_string(newStudentId));
        auto jRev = json::parse(revertedRes->body);
        for (const auto& sk : jRev["skills"])
        {
            if (sk["skillId"] == 1)
            {
                assert(sk["level"] == 0);
                break;
            }
        }
        std::cout << "  [PASS] PUT /api/students/:id/skills and POST /api/students/:id/undo\n";
    }

    // 5. Internships catalogue
    {
        auto res = cli.Get("/api/internships");
        assert(res && res->status == 200);
        auto j = json::parse(res->body);
        assert(j.is_array());
        assert(!j.empty());
        std::cout << "  [PASS] GET /api/internships (" << j.size() << " internships)\n";
    }

    // 6. Recommendations
    {
        auto res = cli.Get("/api/students/" + std::to_string(newStudentId) + "/recommendations");
        assert(res && res->status == 200);
        auto j = json::parse(res->body);
        assert(j.is_array());
        std::cout << "  [PASS] GET /api/students/:id/recommendations (" << j.size() << " recommendations)\n";
    }

    // 7. Apply (ineligible rejection -> skill update -> eligible approval) & Withdraw
    {
        json applyReq = {
            {"studentId", newStudentId},
            {"internshipId", 1}
        };

        // Rejected when ineligible
        auto rejectRes = cli.Post("/api/applications", applyReq.dump(), "application/json");
        assert(rejectRes && rejectRes->status == 400);

        // Meet requirements: skill 1 level 2, skill 2 level 2
        json sk1 = {{"skillId", 1}, {"level", 2}};
        cli.Put("/api/students/" + std::to_string(newStudentId) + "/skills", sk1.dump(), "application/json");
        json sk2 = {{"skillId", 2}, {"level", 2}};
        cli.Put("/api/students/" + std::to_string(newStudentId) + "/skills", sk2.dump(), "application/json");

        // Now eligible -> approved
        auto res = cli.Post("/api/applications", applyReq.dump(), "application/json");
        assert(res && res->status == 201);
        auto j = json::parse(res->body);
        assert(j["ok"] == true);
        ApplicationId appId = j["id"].get<ApplicationId>();

        // Withdraw
        auto withdrawRes = cli.Post("/api/applications/" + std::to_string(appId) + "/withdraw", "", "application/json");
        assert(withdrawRes && withdrawRes->status == 200);
        std::cout << "  [PASS] POST /api/applications (eligibility check & approval) and withdraw\n";
    }

    // Stop server
    server.stop();
    serverThread.join();

    std::cout << "All API smoke tests passed successfully!\n";
    return 0;
}
