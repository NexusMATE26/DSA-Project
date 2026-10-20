#pragma once

#include <memory>
#include <string>

namespace internpath
{
    class AppState;
    class StudentService;
    class AdminService;
    class ApplicationService;

    class HttpServer
    {
    public:
        HttpServer(AppState& state,
                   StudentService& studentService,
                   AdminService& adminService,
                   ApplicationService& appService,
                   std::string staticDir,
                   std::string dataDir);

        ~HttpServer();

        bool start(const std::string& host, int port);
        void stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
