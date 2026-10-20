#include "internpath/core/Persistence.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace internpath
{
    namespace
    {
        std::string modeToString(WorkMode mode)
        {
            switch (mode)
            {
                case WorkMode::Onsite:
                    return "Onsite";
                case WorkMode::Remote:
                    return "Remote";
                case WorkMode::Hybrid:
                    return "Hybrid";
            }
            return "Onsite";
        }

        WorkMode stringToMode(const std::string& s)
        {
            if (s == "Remote")
            {
                return WorkMode::Remote;
            }

            if (s == "Hybrid")
            {
                return WorkMode::Hybrid;
            }

            return WorkMode::Onsite;
        }

        std::string internshipStatusToString(InternshipStatus st)
        {
            switch (st)
            {
                case InternshipStatus::Open:
                    return "Open";
                case InternshipStatus::Closed:
                    return "Closed";
                case InternshipStatus::Cancelled:
                    return "Cancelled";
            }
            return "Open";
        }

        InternshipStatus stringToInternshipStatus(const std::string& s)
        {
            if (s == "Closed")
            {
                return InternshipStatus::Closed;
            }

            if (s == "Cancelled")
            {
                return InternshipStatus::Cancelled;
            }

            return InternshipStatus::Open;
        }

        std::string appStatusToString(ApplicationStatus st)
        {
            switch (st)
            {
                case ApplicationStatus::Submitted:
                    return "Submitted";
                case ApplicationStatus::Allocated:
                    return "Allocated";
                case ApplicationStatus::Waitlisted:
                    return "Waitlisted";
                case ApplicationStatus::Withdrawn:
                    return "Withdrawn";
                case ApplicationStatus::Cancelled:
                    return "Cancelled";
            }
            return "Submitted";
        }

        ApplicationStatus stringToAppStatus(const std::string& s)
        {
            if (s == "Allocated")
            {
                return ApplicationStatus::Allocated;
            }

            if (s == "Waitlisted")
            {
                return ApplicationStatus::Waitlisted;
            }

            if (s == "Withdrawn")
            {
                return ApplicationStatus::Withdrawn;
            }

            if (s == "Cancelled")
            {
                return ApplicationStatus::Cancelled;
            }

            return ApplicationStatus::Submitted;
        }

        std::string trim(const std::string& s)
        {
            std::size_t start = 0;

            while (start < s.size() && (s[start] == ' ' || s[start] == '\r' || s[start] == '\n' || s[start] == '\t'))
            {
                start++;
            }

            std::size_t end = s.size();

            while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\r' || s[end - 1] == '\n' || s[end - 1] == '\t'))
            {
                end--;
            }

            return s.substr(start, end - start);
        }
    }

    ActionResult CsvStorage::saveSnapshot(const AppState& state, const std::string& directoryPath)
    {
        std::filesystem::path dir(directoryPath);
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);

        if (ec)
        {
            return ActionResult::failure(ErrorCode::StorageError, "Failed to create directory: " + ec.message());
        }

        // 1. meta.csv
        {
            std::ofstream file(dir / "meta.csv");

            if (!file.is_open())
            {
                return ActionResult::failure(ErrorCode::StorageError, "Failed to write meta.csv");
            }

            file << "key,value\n";
            file << "schemaVersion,1\n";
            file << "nextStudentId," << state.nextStudentId << "\n";
            file << "nextSkillId," << state.nextSkillId << "\n";
            file << "nextCompanyId," << state.nextCompanyId << "\n";
            file << "nextInternshipId," << state.nextInternshipId << "\n";
            file << "nextApplicationId," << state.nextApplicationId << "\n";
            file << "nextSubmitSequence," << state.nextSubmitSequence << "\n";
            file << "demoToday," << state.demoToday << "\n";
        }

        // 2. skills.csv
        {
            std::ofstream file(dir / "skills.csv");

            if (!file.is_open())
            {
                return ActionResult::failure(ErrorCode::StorageError, "Failed to write skills.csv");
            }

            file << "id,name,category\n";

            for (std::size_t i = 0; i < state.skills.size(); i++)
            {
                const Skill& s = state.skills.at(i);
                file << s.id << "," << s.name << "," << s.category << "\n";
            }
        }

        // 3. companies.csv
        {
            std::ofstream file(dir / "companies.csv");

            if (!file.is_open())
            {
                return ActionResult::failure(ErrorCode::StorageError, "Failed to write companies.csv");
            }

            file << "id,name,description\n";

            for (std::size_t i = 0; i < state.companies.size(); i++)
            {
                const Company& c = state.companies.at(i);
                file << c.id << "," << c.name << "," << c.description << "\n";
            }
        }

        // 4. internships.csv & internship_requirements.csv
        {
            std::ofstream internFile(dir / "internships.csv");
            std::ofstream reqFile(dir / "internship_requirements.csv");

            if (!internFile.is_open() || !reqFile.is_open())
            {
                return ActionResult::failure(ErrorCode::StorageError, "Failed to write internships CSVs");
            }

            internFile << "id,companyId,title,location,mode,stipend,capacity,deadline,status\n";
            reqFile << "internshipId,skillId,requiredLevel,weight,mandatory\n";

            for (std::size_t i = 0; i < state.internships.size(); i++)
            {
                const Internship& in = state.internships.at(i);
                internFile << in.id << "," << in.companyId << "," << in.title << ","
                           << in.location << "," << modeToString(in.mode) << ","
                           << in.stipend << "," << in.capacity << ","
                           << in.deadline << "," << internshipStatusToString(in.status) << "\n";

                for (std::size_t r = 0; r < in.requirements.size(); r++)
                {
                    const Requirement& req = in.requirements.at(r);
                    reqFile << in.id << "," << req.skillId << ","
                            << static_cast<int>(req.requiredLevel) << ","
                            << static_cast<int>(req.weight) << ","
                            << (req.mandatory ? "true" : "false") << "\n";
                }
            }
        }

        // 5. students.csv & student_proficiency.csv
        {
            std::ofstream studentFile(dir / "students.csv");
            std::ofstream profFile(dir / "student_proficiency.csv");

            if (!studentFile.is_open() || !profFile.is_open())
            {
                return ActionResult::failure(ErrorCode::StorageError, "Failed to write students CSVs");
            }

            studentFile << "id,name,email\n";
            profFile << "studentId,skillId,level\n";

            for (std::size_t i = 0; i < state.students.size(); i++)
            {
                const Student& s = state.students.at(i);
                studentFile << s.id << "," << s.fullName << "," << s.email << "\n";

                for (std::size_t p = 0; p < s.proficiency.size(); p++)
                {
                    std::uint8_t lvl = s.proficiency.at(p);

                    if (lvl > 0 && p < state.skills.size())
                    {
                        profFile << s.id << "," << state.skills.at(p).id << "," << static_cast<int>(lvl) << "\n";
                    }
                }
            }
        }

        // 6. skill_prerequisites.csv
        {
            std::ofstream file(dir / "skill_prerequisites.csv");

            if (!file.is_open())
            {
                return ActionResult::failure(ErrorCode::StorageError, "Failed to write skill_prerequisites.csv");
            }

            file << "fromSkillId,toSkillId,minLevel\n";

            for (std::size_t u = 0; u < state.skills.size(); u++)
            {
                SkillId fromId = state.skills.at(u).id;
                auto* node = state.skillPrerequisites.outgoing(static_cast<std::uint32_t>(u)).firstNode();

                while (node != nullptr)
                {
                    std::uint32_t v = node->value.toIndex;

                    if (v < state.skills.size())
                    {
                        SkillId toId = state.skills.at(v).id;
                        file << fromId << "," << toId << "," << static_cast<int>(node->value.minimumLevel) << "\n";
                    }

                    node = node->next;
                }
            }
        }

        // 7. applications.csv
        {
            std::ofstream file(dir / "applications.csv");

            if (!file.is_open())
            {
                return ActionResult::failure(ErrorCode::StorageError, "Failed to write applications.csv");
            }

            file << "id,studentId,internshipId,status,submitSequence,fitNumeratorAtSubmit,fitDenominatorAtSubmit,rankAtClose\n";

            for (std::size_t i = 0; i < state.applications.size(); i++)
            {
                const Application& app = state.applications.at(i);
                file << app.id << "," << app.studentId << "," << app.internshipId << ","
                     << appStatusToString(app.status) << "," << app.submitSequence << ","
                     << app.fitNumeratorAtSubmit << "," << app.fitDenominatorAtSubmit << ","
                     << app.rankAtClose << "\n";
            }
        }

        return ActionResult::success("Snapshot saved successfully");
    }

    ActionResult CsvStorage::loadSnapshot(AppState& state, const std::string& directoryPath)
    {
        std::filesystem::path dir(directoryPath);

        if (!std::filesystem::exists(dir))
        {
            return ActionResult::failure(ErrorCode::NotFound, "Directory not found: " + directoryPath);
        }

        state.clear();

        // 1. meta.csv
        {
            std::ifstream file(dir / "meta.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line); // header

                while (std::getline(file, line))
                {
                    std::stringstream ss(line);
                    std::string key, val;

                    if (std::getline(ss, key, ',') && std::getline(ss, val))
                    {
                        key = trim(key);
                        val = trim(val);

                        if (key == "nextStudentId") state.nextStudentId = std::stoul(val);
                        else if (key == "nextSkillId") state.nextSkillId = std::stoul(val);
                        else if (key == "nextCompanyId") state.nextCompanyId = std::stoul(val);
                        else if (key == "nextInternshipId") state.nextInternshipId = std::stoul(val);
                        else if (key == "nextApplicationId") state.nextApplicationId = std::stoul(val);
                        else if (key == "nextSubmitSequence") state.nextSubmitSequence = std::stoull(val);
                        else if (key == "demoToday") state.demoToday = std::stoi(val);
                    }
                }
            }
        }

        // 2. skills.csv
        {
            std::ifstream file(dir / "skills.csv");

            if (!file.is_open())
            {
                return ActionResult::failure(ErrorCode::NotFound, "skills.csv not found");
            }

            std::string line;
            std::getline(file, line); // header

            while (std::getline(file, line))
            {
                if (line.empty()) continue;
                std::stringstream ss(line);
                std::string idStr, name, cat;

                if (std::getline(ss, idStr, ',') && std::getline(ss, name, ',') && std::getline(ss, cat))
                {
                    Skill s;
                    s.id = std::stoul(trim(idStr));
                    s.name = trim(name);
                    s.category = trim(cat);
                    s.denseIndex = static_cast<std::uint32_t>(state.skills.size());
                    s.active = true;

                    state.skills.pushBack(std::move(s));
                    state.skillPrerequisites.addVertex();
                }
            }
        }

        // 3. companies.csv
        {
            std::ifstream file(dir / "companies.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line);

                while (std::getline(file, line))
                {
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string idStr, name, desc;

                    if (std::getline(ss, idStr, ',') && std::getline(ss, name, ',') && std::getline(ss, desc))
                    {
                        Company c;
                        c.id = std::stoul(trim(idStr));
                        c.name = trim(name);
                        c.description = trim(desc);
                        c.active = true;
                        state.companies.pushBack(std::move(c));
                    }
                }
            }
        }

        // 4. internships.csv
        {
            std::ifstream file(dir / "internships.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line);

                while (std::getline(file, line))
                {
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string idStr, compIdStr, title, loc, mode, stipStr, capStr, deadStr, statStr;

                    if (std::getline(ss, idStr, ',') && std::getline(ss, compIdStr, ',') &&
                        std::getline(ss, title, ',') && std::getline(ss, loc, ',') &&
                        std::getline(ss, mode, ',') && std::getline(ss, stipStr, ',') &&
                        std::getline(ss, capStr, ',') && std::getline(ss, deadStr, ',') &&
                        std::getline(ss, statStr))
                    {
                        Internship in;
                        in.id = std::stoul(trim(idStr));
                        in.companyId = std::stoul(trim(compIdStr));
                        in.title = trim(title);
                        in.location = trim(loc);
                        in.mode = stringToMode(trim(mode));
                        in.stipend = std::stoi(trim(stipStr));
                        in.capacity = std::stoul(trim(capStr));
                        in.deadline = std::stoi(trim(deadStr));
                        in.status = stringToInternshipStatus(trim(statStr));

                        state.internships.pushBack(std::move(in));
                        state.waitlists.pushBack(WaitingList{});
                    }
                }
            }
        }

        // Rebuild indexes so findInternship, etc. work
        state.rebuildIndexes();

        // 5. internship_requirements.csv
        {
            std::ifstream file(dir / "internship_requirements.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line);

                while (std::getline(file, line))
                {
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string inIdStr, skIdStr, reqLvlStr, wStr, mandStr;

                    if (std::getline(ss, inIdStr, ',') && std::getline(ss, skIdStr, ',') &&
                        std::getline(ss, reqLvlStr, ',') && std::getline(ss, wStr, ',') &&
                        std::getline(ss, mandStr))
                    {
                        InternshipId inId = std::stoul(trim(inIdStr));
                        Internship* in = state.findInternship(inId);

                        if (in != nullptr)
                        {
                            Requirement req;
                            req.skillId = std::stoul(trim(skIdStr));
                            req.requiredLevel = static_cast<std::uint8_t>(std::stoi(trim(reqLvlStr)));
                            req.weight = static_cast<std::uint8_t>(std::stoi(trim(wStr)));
                            req.mandatory = (trim(mandStr) == "true" || trim(mandStr) == "1");
                            in->requirements.pushBack(req);
                        }
                    }
                }
            }
        }

        // 6. students.csv
        {
            std::ifstream file(dir / "students.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line);

                while (std::getline(file, line))
                {
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string idStr, name, email;

                    if (std::getline(ss, idStr, ',') && std::getline(ss, name, ',') && std::getline(ss, email))
                    {
                        Student s;
                        s.id = std::stoul(trim(idStr));
                        s.fullName = trim(name);
                        s.email = trim(email);
                        s.active = true;

                        for (std::size_t i = 0; i < state.skills.size(); i++)
                        {
                            s.proficiency.pushBack(0);
                        }

                        state.students.pushBack(std::move(s));
                        state.undoHistories.pushBack(UndoHistory{});
                    }
                }
            }
        }

        state.rebuildIndexes();

        // 7. student_proficiency.csv
        {
            std::ifstream file(dir / "student_proficiency.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line);

                while (std::getline(file, line))
                {
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string stIdStr, skIdStr, lvlStr;

                    if (std::getline(ss, stIdStr, ',') && std::getline(ss, skIdStr, ',') && std::getline(ss, lvlStr))
                    {
                        StudentId stId = std::stoul(trim(stIdStr));
                        SkillId skId = std::stoul(trim(skIdStr));
                        std::uint8_t lvl = static_cast<std::uint8_t>(std::stoi(trim(lvlStr)));

                        Student* st = state.findStudent(stId);
                        Skill* sk = state.findSkill(skId);

                        if (st != nullptr && sk != nullptr && sk->denseIndex < st->proficiency.size())
                        {
                            st->proficiency.at(sk->denseIndex) = lvl;
                        }
                    }
                }
            }
        }

        // 8. skill_prerequisites.csv
        {
            std::ifstream file(dir / "skill_prerequisites.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line);

                while (std::getline(file, line))
                {
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string fromStr, toStr, lvlStr;

                    if (std::getline(ss, fromStr, ',') && std::getline(ss, toStr, ',') && std::getline(ss, lvlStr))
                    {
                        SkillId fromId = std::stoul(trim(fromStr));
                        SkillId toId = std::stoul(trim(toStr));
                        std::uint8_t lvl = static_cast<std::uint8_t>(std::stoi(trim(lvlStr)));

                        Skill* s1 = state.findSkill(fromId);
                        Skill* s2 = state.findSkill(toId);

                        if (s1 != nullptr && s2 != nullptr)
                        {
                            state.skillPrerequisites.addEdge(s1->denseIndex, s2->denseIndex, lvl);
                        }
                    }
                }
            }
        }

        // 9. applications.csv
        {
            std::ifstream file(dir / "applications.csv");

            if (file.is_open())
            {
                std::string line;
                std::getline(file, line);

                while (std::getline(file, line))
                {
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string idStr, stIdStr, inIdStr, statStr, seqStr, numStr, denStr, rankStr;

                    if (std::getline(ss, idStr, ',') && std::getline(ss, stIdStr, ',') &&
                        std::getline(ss, inIdStr, ',') && std::getline(ss, statStr, ',') &&
                        std::getline(ss, seqStr, ',') && std::getline(ss, numStr, ',') &&
                        std::getline(ss, denStr, ',') && std::getline(ss, rankStr))
                    {
                        Application app;
                        app.id = std::stoul(trim(idStr));
                        app.studentId = std::stoul(trim(stIdStr));
                        app.internshipId = std::stoul(trim(inIdStr));
                        app.status = stringToAppStatus(trim(statStr));
                        app.submitSequence = std::stoull(trim(seqStr));
                        app.fitNumeratorAtSubmit = std::stoi(trim(numStr));
                        app.fitDenominatorAtSubmit = std::stoi(trim(denStr));
                        app.rankAtClose = std::stoul(trim(rankStr));

                        state.applications.pushBack(std::move(app));
                    }
                }
            }
        }

        // Final index rebuild
        state.rebuildIndexes();

        // Re-enroll any waitlisted applications into their internship's WaitingList
        for (std::size_t i = 0; i < state.applications.size(); i++)
        {
            const Application& app = state.applications.at(i);

            if (app.status == ApplicationStatus::Waitlisted)
            {
                WaitingList* wl = state.getWaitingList(app.internshipId);

                if (wl != nullptr)
                {
                    wl->append(app.id);
                }
            }
        }

        return ActionResult::success("Snapshot loaded successfully");
    }
}
