#define WIN32_LEAN_AND_MEAN
#include <httplib.h>
#include <nlohmann/json.hpp>

#include "internpath/api/Routes.h"
#include "internpath/core/AppState.h"
#include "internpath/core/Persistence.h"
#include "internpath/core/ServiceContracts.h"

using json = nlohmann::json;

namespace internpath
{
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

        WorkMode parseWorkMode(const std::string& str)
        {
            if (str == "Remote") return WorkMode::Remote;
            if (str == "Hybrid") return WorkMode::Hybrid;
            return WorkMode::Onsite;
        }

        std::string applicationStatusToString(ApplicationStatus status)
        {
            switch (status)
            {
                case ApplicationStatus::Allocated: return "Allocated";
                case ApplicationStatus::Waitlisted: return "Waitlisted";
                case ApplicationStatus::Withdrawn: return "Withdrawn";
                case ApplicationStatus::Cancelled: return "Cancelled";
                case ApplicationStatus::Submitted:
                default: return "Submitted";
            }
        }

        std::string internshipStatusToString(InternshipStatus status)
        {
            switch (status)
            {
                case InternshipStatus::Closed: return "Closed";
                case InternshipStatus::Cancelled: return "Cancelled";
                case InternshipStatus::Open:
                default: return "Open";
            }
        }

        void jsonResponse(httplib::Response& res, const json& data, int status = 200)
        {
            res.status = status;
            res.set_content(data.dump(2), "application/json; charset=utf-8");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
        }

        void errorResponse(httplib::Response& res, const std::string& message, int status = 400)
        {
            json err;
            err["ok"] = false;
            err["error"] = message;
            jsonResponse(res, err, status);
        }
    }

    void registerRoutes(httplib::Server& server,
                        AppState& state,
                        StudentService& studentService,
                        AdminService& adminService,
                        ApplicationService& appService,
                        const std::string& dataDir)
    {
        // CORS preflight
        server.Options(".*", [](const httplib::Request&, httplib::Response& res)
        {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.status = 204;
        });

        // 1. Health check
        server.Get("/api/health", [&state](const httplib::Request&, httplib::Response& res)
        {
            json data;
            data["ok"] = true;
            data["name"] = "InternPath";
            data["today"] = state.demoToday;
            data["studentsCount"] = state.students.size();
            data["internshipsCount"] = state.internships.size();
            data["companiesCount"] = state.companies.size();
            data["skillsCount"] = state.skills.size();
            jsonResponse(res, data);
        });

        // 2. Students API
        server.Get("/api/students", [&state](const httplib::Request&, httplib::Response& res)
        {
            json list = json::array();
            for (std::size_t i = 0; i < state.students.size(); i++)
            {
                const Student& s = state.students.at(i);
                json item;
                item["id"] = s.id;
                item["name"] = s.fullName;
                item["email"] = s.email;
                item["active"] = s.active;
                list.push_back(item);
            }
            jsonResponse(res, list);
        });

        server.Get(R"(/api/students/(\d+))", [&state](const httplib::Request& req, httplib::Response& res)
        {
            StudentId id = static_cast<StudentId>(std::stoul(req.matches[1]));
            const Student* s = state.findStudent(id);
            if (!s)
            {
                errorResponse(res, "Student not found", 404);
                return;
            }

            json skillsList = json::array();
            for (std::size_t i = 0; i < state.skills.size(); i++)
            {
                const Skill& sk = state.skills.at(i);
                std::uint8_t lvl = 0;
                if (sk.denseIndex < s->proficiency.size())
                {
                    lvl = s->proficiency.at(sk.denseIndex);
                }
                json item;
                item["skillId"] = sk.id;
                item["name"] = sk.name;
                item["category"] = sk.category;
                item["level"] = lvl;
                skillsList.push_back(item);
            }

            json data;
            data["id"] = s->id;
            data["name"] = s->fullName;
            data["email"] = s->email;
            data["active"] = s->active;
            data["skills"] = skillsList;
            jsonResponse(res, data);
        });

        server.Post("/api/students", [&studentService](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                std::string name = body.value("name", "");
                std::string email = body.value("email", "");

                if (name.empty() || email.empty())
                {
                    errorResponse(res, "Name and email are required");
                    return;
                }

                ActionResult result = studentService.registerStudent(name, email);
                if (result.ok)
                {
                    json okData;
                    okData["ok"] = true;
                    okData["id"] = result.createdId;
                    jsonResponse(res, okData, 201);
                }
                else
                {
                    errorResponse(res, result.message, 400);
                }
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        server.Put(R"(/api/students/(\d+))", [&state](const httplib::Request& req, httplib::Response& res)
        {
            StudentId id = static_cast<StudentId>(std::stoul(req.matches[1]));
            Student* s = state.findStudent(id);
            if (!s)
            {
                errorResponse(res, "Student not found", 404);
                return;
            }

            try
            {
                auto body = json::parse(req.body);

                if (body.contains("name"))
                {
                    s->fullName = body["name"].get<std::string>();
                }
                if (body.contains("email"))
                {
                    s->email = body["email"].get<std::string>();
                }

                json okData;
                okData["ok"] = true;
                okData["message"] = "Profile updated";
                jsonResponse(res, okData);
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        server.Post(R"(/api/students/(\d+)/undo)", [&studentService](const httplib::Request& req, httplib::Response& res)
        {
            StudentId id = static_cast<StudentId>(std::stoul(req.matches[1]));
            ActionResult result = studentService.undoLastProfileEdit(id);
            if (result.ok)
            {
                json okData;
                okData["ok"] = true;
                okData["message"] = result.message;
                jsonResponse(res, okData);
            }
            else
            {
                errorResponse(res, result.message, 400);
            }
        });

        server.Put(R"(/api/students/(\d+)/skills)", [&studentService](const httplib::Request& req, httplib::Response& res)
        {
            StudentId id = static_cast<StudentId>(std::stoul(req.matches[1]));
            try
            {
                auto body = json::parse(req.body);
                SkillId skillId = body.value("skillId", static_cast<SkillId>(0));
                int level = body.value("level", 0);

                ActionResult result = studentService.setSkillLevel(id, skillId, level);
                if (result.ok)
                {
                    json okData;
                    okData["ok"] = true;
                    okData["message"] = result.message;
                    jsonResponse(res, okData);
                }
                else
                {
                    errorResponse(res, result.message, 400);
                }
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        server.Get(R"(/api/students/(\d+)/recommendations)", [&studentService, &state](const httplib::Request& req, httplib::Response& res)
        {
            StudentId id = static_cast<StudentId>(std::stoul(req.matches[1]));
            DynamicArray<Recommendation> recs = studentService.getRecommendations(id);

            json list = json::array();
            for (std::size_t i = 0; i < recs.size(); i++)
            {
                const Recommendation& r = recs.at(i);
                const Internship* in = state.findInternship(r.internshipId);
                const Company* comp = (in != nullptr) ? state.findCompany(in->companyId) : nullptr;

                int pct = r.match.roundedPercent();

                json item;
                item["internshipId"] = r.internshipId;
                item["title"] = in ? in->title : "Unknown";
                item["description"] = in ? in->description : "";
                item["companyName"] = comp ? comp->name : "Unknown";
                item["fitNumerator"] = r.match.numerator;
                item["fitDenominator"] = r.match.denominator;
                item["matchPercentage"] = pct;
                item["isEligible"] = r.match.eligible;
                item["deadline"] = r.deadline;
                item["stipend"] = in ? in->stipend : 0;
                item["location"] = in ? in->location : "";
                item["mode"] = in ? modeToString(in->mode) : "Onsite";
                list.push_back(item);
            }

            jsonResponse(res, list);
        });

        server.Get(R"(/api/students/(\d+)/plan/(\d+))", [&studentService](const httplib::Request& req, httplib::Response& res)
        {
            StudentId studentId = static_cast<StudentId>(std::stoul(req.matches[1]));
            InternshipId internshipId = static_cast<InternshipId>(std::stoul(req.matches[2]));

            DynamicArray<LearningTask> tasks = studentService.getLearningPlan(studentId, internshipId);

            json taskList = json::array();
            for (std::size_t i = 0; i < tasks.size(); i++)
            {
                const LearningTask& t = tasks.at(i);
                json item;
                item["skillId"] = t.skillId;
                item["skillName"] = t.skillName;
                item["currentLevel"] = t.currentLevel;
                item["targetLevel"] = t.targetLevel;
                item["stage"] = t.stage;
                taskList.push_back(item);
            }

            json data;
            data["studentId"] = studentId;
            data["internshipId"] = internshipId;
            data["tasks"] = taskList;
            jsonResponse(res, data);
        });

        server.Get(R"(/api/students/(\d+)/applications)", [&state](const httplib::Request& req, httplib::Response& res)
        {
            StudentId studentId = static_cast<StudentId>(std::stoul(req.matches[1]));
            json list = json::array();

            for (std::size_t i = 0; i < state.applications.size(); i++)
            {
                const Application& app = state.applications.at(i);
                if (app.studentId == studentId)
                {
                    const Internship* in = state.findInternship(app.internshipId);
                    const Company* comp = in ? state.findCompany(in->companyId) : nullptr;

                    json item;
                    item["id"] = app.id;
                    item["internshipId"] = app.internshipId;
                    item["internshipTitle"] = in ? in->title : "Unknown";
                    item["companyName"] = comp ? comp->name : "Unknown";
                    item["status"] = applicationStatusToString(app.status);
                    item["submitSequence"] = app.submitSequence;
                    item["fitNumerator"] = app.fitNumeratorAtSubmit;
                    item["fitDenominator"] = app.fitDenominatorAtSubmit;
                    item["rankAtClose"] = app.rankAtClose;
                    list.push_back(item);
                }
            }

            jsonResponse(res, list);
        });

        // 3. Companies API
        server.Get("/api/companies", [&state](const httplib::Request&, httplib::Response& res)
        {
            json list = json::array();
            for (std::size_t i = 0; i < state.companies.size(); i++)
            {
                const Company& c = state.companies.at(i);
                json item;
                item["id"] = c.id;
                item["name"] = c.name;
                item["description"] = c.description;
                item["active"] = c.active;
                list.push_back(item);
            }
            jsonResponse(res, list);
        });

        server.Post("/api/companies", [&adminService](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                std::string name = body.value("name", "");
                std::string desc = body.value("description", "");

                if (name.empty())
                {
                    errorResponse(res, "Company name is required");
                    return;
                }

                ActionResult result = adminService.addCompany(name, desc);
                if (result.ok)
                {
                    json okData;
                    okData["ok"] = true;
                    okData["id"] = result.createdId;
                    jsonResponse(res, okData, 201);
                }
                else
                {
                    errorResponse(res, result.message, 400);
                }
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        // 4. Internships & Catalogue API
        server.Get("/api/internships", [&state](const httplib::Request&, httplib::Response& res)
        {
            json list = json::array();
            for (std::size_t i = 0; i < state.internships.size(); i++)
            {
                const Internship& in = state.internships.at(i);
                const Company* comp = state.findCompany(in.companyId);

                json reqs = json::array();
                for (std::size_t r = 0; r < in.requirements.size(); r++)
                {
                    const Requirement& reqItem = in.requirements.at(r);
                    const Skill* sk = state.findSkill(reqItem.skillId);
                    json rObj;
                    rObj["skillId"] = reqItem.skillId;
                    rObj["skillName"] = sk ? sk->name : "Unknown";
                    rObj["requiredLevel"] = reqItem.requiredLevel;
                    rObj["weight"] = reqItem.weight;
                    rObj["mandatory"] = reqItem.mandatory;
                    reqs.push_back(rObj);
                }

                json item;
                item["id"] = in.id;
                item["companyId"] = in.companyId;
                item["companyName"] = comp ? comp->name : "Unknown";
                item["title"] = in.title;
                item["description"] = in.description;
                item["location"] = in.location;
                item["mode"] = modeToString(in.mode);
                item["stipend"] = in.stipend;
                item["capacity"] = in.capacity;
                item["deadline"] = in.deadline;
                item["status"] = internshipStatusToString(in.status);
                item["requirements"] = reqs;
                list.push_back(item);
            }
            jsonResponse(res, list);
        });

        server.Get(R"(/api/internships/(\d+))", [&state](const httplib::Request& req, httplib::Response& res)
        {
            InternshipId id = static_cast<InternshipId>(std::stoul(req.matches[1]));
            const Internship* in = state.findInternship(id);
            if (!in)
            {
                errorResponse(res, "Internship not found", 404);
                return;
            }

            const Company* comp = state.findCompany(in->companyId);
            json reqs = json::array();
            for (std::size_t r = 0; r < in->requirements.size(); r++)
            {
                const Requirement& reqItem = in->requirements.at(r);
                const Skill* sk = state.findSkill(reqItem.skillId);
                json rObj;
                rObj["skillId"] = reqItem.skillId;
                rObj["skillName"] = sk ? sk->name : "Unknown";
                rObj["requiredLevel"] = reqItem.requiredLevel;
                rObj["weight"] = reqItem.weight;
                rObj["mandatory"] = reqItem.mandatory;
                reqs.push_back(rObj);
            }

            json data;
            data["id"] = in->id;
            data["companyId"] = in->companyId;
            data["companyName"] = comp ? comp->name : "Unknown";
            data["title"] = in->title;
            data["description"] = in->description;
            data["location"] = in->location;
            data["mode"] = modeToString(in->mode);
            data["stipend"] = in->stipend;
            data["capacity"] = in->capacity;
            data["deadline"] = in->deadline;
            data["status"] = internshipStatusToString(in->status);
            data["requirements"] = reqs;
            jsonResponse(res, data);
        });

        server.Post("/api/internships", [&adminService](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                Internship in;
                in.companyId = body.value("companyId", static_cast<CompanyId>(1));
                in.title = body.value("title", "");
                in.description = body.value("description", "");
                in.location = body.value("location", "Colombo");
                in.mode = parseWorkMode(body.value("mode", "Onsite"));
                in.stipend = body.value("stipend", 0);
                in.capacity = body.value("capacity", 1);
                in.deadline = body.value("deadline", 20261231);
                in.status = InternshipStatus::Open;

                if (in.title.empty())
                {
                    errorResponse(res, "Title is required");
                    return;
                }

                if (body.contains("requirements") && body["requirements"].is_array())
                {
                    for (const auto& item : body["requirements"])
                    {
                        Requirement r;
                        r.skillId = item.value("skillId", static_cast<SkillId>(1));
                        r.requiredLevel = item.value("requiredLevel", static_cast<std::uint8_t>(1));
                        r.weight = item.value("weight", static_cast<std::uint8_t>(1));
                        r.mandatory = item.value("mandatory", false);
                        in.requirements.pushBack(r);
                    }
                }

                ActionResult result = adminService.publishInternship(std::move(in));
                if (result.ok)
                {
                    json okData;
                    okData["ok"] = true;
                    okData["id"] = result.createdId;
                    jsonResponse(res, okData, 201);
                }
                else
                {
                    errorResponse(res, result.message, 400);
                }
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        server.Get(R"(/api/internships/(\d+)/applications)", [&state](const httplib::Request& req, httplib::Response& res)
        {
            InternshipId inId = static_cast<InternshipId>(std::stoul(req.matches[1]));
            json list = json::array();

            for (std::size_t i = 0; i < state.applications.size(); i++)
            {
                const Application& app = state.applications.at(i);
                if (app.internshipId == inId)
                {
                    const Student* s = state.findStudent(app.studentId);
                    json item;
                    item["id"] = app.id;
                    item["studentId"] = app.studentId;
                    item["studentName"] = s ? s->fullName : "Unknown";
                    item["studentEmail"] = s ? s->email : "";
                    item["status"] = applicationStatusToString(app.status);
                    item["submitSequence"] = app.submitSequence;
                    item["fitNumerator"] = app.fitNumeratorAtSubmit;
                    item["fitDenominator"] = app.fitDenominatorAtSubmit;
                    item["rankAtClose"] = app.rankAtClose;
                    list.push_back(item);
                }
            }

            jsonResponse(res, list);
        });

        server.Get(R"(/api/internships/(\d+)/waitlist)", [&state](const httplib::Request& req, httplib::Response& res)
        {
            InternshipId inId = static_cast<InternshipId>(std::stoul(req.matches[1]));
            WaitingList* wl = state.getWaitingList(inId);

            json list = json::array();
            if (wl)
            {
                DynamicArray<ApplicationId> entries = wl->snapshot();
                for (std::size_t i = 0; i < entries.size(); i++)
                {
                    ApplicationId appId = entries.at(i);
                    const Application* app = state.findApplication(appId);
                    const Student* s = (app != nullptr) ? state.findStudent(app->studentId) : nullptr;

                    json item;
                    item["applicationId"] = appId;
                    item["studentId"] = app ? app->studentId : 0;
                    item["studentName"] = s ? s->fullName : "Unknown";
                    item["rank"] = i + 1;
                    list.push_back(item);
                }
            }

            jsonResponse(res, list);
        });

        server.Post(R"(/api/internships/(\d+)/close)", [&adminService](const httplib::Request& req, httplib::Response& res)
        {
            InternshipId inId = static_cast<InternshipId>(std::stoul(req.matches[1]));
            ActionResult result = adminService.closeRound(inId);
            if (result.ok)
            {
                json okData;
                okData["ok"] = true;
                okData["message"] = result.message;
                jsonResponse(res, okData);
            }
            else
            {
                errorResponse(res, result.message, 400);
            }
        });

        server.Post(R"(/api/internships/(\d+)/promote)", [&state](const httplib::Request& req, httplib::Response& res)
        {
            InternshipId inId = static_cast<InternshipId>(std::stoul(req.matches[1]));
            WaitingList* wl = state.getWaitingList(inId);
            if (!wl || wl->size() == 0)
            {
                errorResponse(res, "Waiting list is empty", 400);
                return;
            }

            ApplicationId promotedId = 0;
            if (wl->promoteHead(promotedId))
            {
                Application* app = state.findApplication(promotedId);
                if (app)
                {
                    app->status = ApplicationStatus::Allocated;
                    const Student* s = state.findStudent(app->studentId);
                    json okData;
                    okData["ok"] = true;
                    okData["promotedApplicationId"] = promotedId;
                    okData["studentId"] = app->studentId;
                    okData["studentName"] = s ? s->fullName : "Unknown";
                    jsonResponse(res, okData);
                    return;
                }
            }
            errorResponse(res, "Failed to promote from waiting list", 400);
        });

        // 5. Applications API
        server.Post("/api/applications", [&appService](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                StudentId studentId = body.value("studentId", static_cast<StudentId>(0));
                InternshipId internshipId = body.value("internshipId", static_cast<InternshipId>(0));

                if (studentId == 0 || internshipId == 0)
                {
                    errorResponse(res, "studentId and internshipId are required");
                    return;
                }

                ActionResult result = appService.apply(studentId, internshipId);
                if (result.ok)
                {
                    json okData;
                    okData["ok"] = true;
                    okData["id"] = result.createdId;
                    okData["message"] = result.message;
                    jsonResponse(res, okData, 201);
                }
                else
                {
                    errorResponse(res, result.message, 400);
                }
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        server.Post(R"(/api/applications/(\d+)/withdraw)", [&appService](const httplib::Request& req, httplib::Response& res)
        {
            ApplicationId appId = static_cast<ApplicationId>(std::stoul(req.matches[1]));
            ActionResult result = appService.withdraw(appId);
            if (result.ok)
            {
                json okData;
                okData["ok"] = true;
                okData["message"] = result.message;
                jsonResponse(res, okData);
            }
            else
            {
                errorResponse(res, result.message, 400);
            }
        });

        // 6. Skills & Prerequisite Graph API
        server.Get("/api/skills", [&state](const httplib::Request&, httplib::Response& res)
        {
            json skillsList = json::array();
            for (std::size_t i = 0; i < state.skills.size(); i++)
            {
                const Skill& sk = state.skills.at(i);
                json item;
                item["id"] = sk.id;
                item["name"] = sk.name;
                item["category"] = sk.category;
                item["denseIndex"] = sk.denseIndex;
                item["active"] = sk.active;
                skillsList.push_back(item);
            }

            json prereqsList = json::array();
            for (std::uint32_t u = 0; u < state.skillPrerequisites.vertexCount(); u++)
            {
                auto* node = state.skillPrerequisites.outgoing(u).firstNode();
                while (node != nullptr)
                {
                    std::uint32_t v = node->value.toIndex;
                    std::uint8_t minLvl = node->value.minimumLevel;

                    SkillId uId = 0;
                    SkillId vId = 0;
                    for (std::size_t s = 0; s < state.skills.size(); s++)
                    {
                        if (state.skills.at(s).denseIndex == u) uId = state.skills.at(s).id;
                        if (state.skills.at(s).denseIndex == v) vId = state.skills.at(s).id;
                    }

                    json edgeObj;
                    edgeObj["firstSkillId"] = uId;
                    edgeObj["nextSkillId"] = vId;
                    edgeObj["minimumLevel"] = minLvl;
                    prereqsList.push_back(edgeObj);

                    node = node->next;
                }
            }

            json data;
            data["skills"] = skillsList;
            data["prerequisites"] = prereqsList;
            jsonResponse(res, data);
        });

        server.Post("/api/skills", [&adminService](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                std::string name = body.value("name", "");
                std::string cat = body.value("category", "General");

                ActionResult result = adminService.addSkill(name, cat);
                if (result.ok)
                {
                    json okData;
                    okData["ok"] = true;
                    okData["id"] = result.createdId;
                    jsonResponse(res, okData, 201);
                }
                else
                {
                    errorResponse(res, result.message, 400);
                }
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        server.Post("/api/skills/prerequisites", [&adminService](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                SkillId first = body.value("first", static_cast<SkillId>(0));
                SkillId next = body.value("next", static_cast<SkillId>(0));
                int minLvl = body.value("minimumLevel", 1);

                ActionResult result = adminService.addPrerequisite(first, next, minLvl);
                if (result.ok)
                {
                    json okData;
                    okData["ok"] = true;
                    okData["message"] = "Prerequisite added successfully";
                    jsonResponse(res, okData);
                }
                else
                {
                    errorResponse(res, result.message, 400);
                }
            }
            catch (const std::exception& e)
            {
                errorResponse(res, std::string("Invalid JSON: ") + e.what(), 400);
            }
        });

        // 7. Persistence Snapshot API
        server.Post("/api/admin/save", [&state, dataDir](const httplib::Request&, httplib::Response& res)
        {
            ActionResult result = CsvStorage::saveSnapshot(state, dataDir);
            if (result.ok)
            {
                json okData;
                okData["ok"] = true;
                okData["message"] = "Snapshot saved to " + dataDir;
                jsonResponse(res, okData);
            }
            else
            {
                errorResponse(res, "Failed to save CSV snapshot: " + result.message, 500);
            }
        });

        server.Post("/api/admin/reload", [&state, dataDir](const httplib::Request&, httplib::Response& res)
        {
            ActionResult result = CsvStorage::loadSnapshot(state, dataDir);
            if (result.ok)
            {
                json okData;
                okData["ok"] = true;
                okData["message"] = "Snapshot reloaded from " + dataDir;
                jsonResponse(res, okData);
            }
            else
            {
                errorResponse(res, "Failed to load CSV snapshot: " + result.message, 500);
            }
        });
    }
}
