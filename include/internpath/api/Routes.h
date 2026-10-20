#pragma once

#include <string>

namespace httplib
{
    class Server;
}

namespace internpath
{
    class AppState;
    class StudentService;
    class AdminService;
    class ApplicationService;

    void registerRoutes(httplib::Server& server,
                        AppState& state,
                        StudentService& studentService,
                        AdminService& adminService,
                        ApplicationService& appService,
                        const std::string& dataDir);
}
