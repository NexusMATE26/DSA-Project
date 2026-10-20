#include <filesystem>
#include <iostream>
#include <string>

#include "internpath/api/HttpServer.h"
#include "internpath/core/AppState.h"
#include "internpath/core/Persistence.h"
#include "internpath/core/ServiceContracts.h"

using namespace internpath;

int main(int argc, char* argv[])
{
    std::cout << "==============================================================\n";
    std::cout << "  InternPath Backend Server (Milestone 4)\n";
    std::cout << "==============================================================\n";

    AppState state;
    StudentService studentService(state);
    AdminService adminService(state);
    ApplicationService appService(state);

    // Locate data directory
    std::string dataDir = "data/seed";
    if (!std::filesystem::exists(dataDir) && std::filesystem::exists("../data/seed"))
    {
        dataDir = "../data/seed";
    }

    if (std::filesystem::exists(dataDir))
    {
        std::cout << "Loading dataset from: " << dataDir << " ...\n";
        ActionResult res = CsvStorage::loadSnapshot(state, dataDir);
        if (res.ok)
        {
            std::cout << "Loaded " << state.students.size() << " students, "
                      << state.internships.size() << " internships, "
                      << state.companies.size() << " companies, "
                      << state.skills.size() << " skills.\n";
        }
        else
        {
            std::cout << "Warning: Failed to load snapshot cleanly from " << dataDir << "\n";
        }
    }
    else
    {
        std::cout << "Data directory not found. Starting with empty state.\n";
    }

    // Locate web directory
    std::string webDir = "web";
    if (!std::filesystem::exists(webDir) && std::filesystem::exists("../web"))
    {
        webDir = "../web";
    }
    std::cout << "Serving static assets from: " << webDir << "\n";

    int port = 8080;
    if (argc > 1)
    {
        try
        {
            port = std::stoi(argv[1]);
        }
        catch (...)
        {
            std::cout << "Invalid port argument, defaulting to 8080.\n";
        }
    }

    HttpServer server(state, studentService, adminService, appService, webDir, dataDir);

    std::cout << "InternPath running at http://127.0.0.1:" << port << "/\n";
    std::cout << "Press Ctrl+C to stop.\n";

    if (!server.start("127.0.0.1", port))
    {
        std::cerr << "Failed to bind or listen on 127.0.0.1:" << port << "\n";
        return 1;
    }

    return 0;
}
