#define WIN32_LEAN_AND_MEAN
#include <httplib.h>
#include <iostream>

#include "internpath/api/HttpServer.h"
#include "internpath/api/Routes.h"
#include "internpath/core/AppState.h"
#include "internpath/core/ServiceContracts.h"

namespace internpath
{
    struct HttpServer::Impl
    {
        httplib::Server server;
        AppState& state;
        StudentService& studentService;
        AdminService& adminService;
        ApplicationService& appService;
        std::string staticDir;
        std::string dataDir;

        Impl(AppState& s,
             StudentService& ss,
             AdminService& as,
             ApplicationService& apps,
             std::string sDir,
             std::string dDir)
            : state(s),
              studentService(ss),
              adminService(as),
              appService(apps),
              staticDir(std::move(sDir)),
              dataDir(std::move(dDir))
        {
            if (!staticDir.empty())
            {
                server.set_mount_point("/", staticDir);
            }

            registerRoutes(server, state, studentService, adminService, appService, dataDir);
        }
    };

    HttpServer::HttpServer(AppState& state,
                           StudentService& studentService,
                           AdminService& adminService,
                           ApplicationService& appService,
                           std::string staticDir,
                           std::string dataDir)
        : impl_(std::make_unique<Impl>(state,
                                       studentService,
                                       adminService,
                                       appService,
                                       std::move(staticDir),
                                       std::move(dataDir)))
    {
    }

    HttpServer::~HttpServer() = default;

    bool HttpServer::start(const std::string& host, int port)
    {
        std::cout << "InternPath server listening on http://" << host << ":" << port << "\n";
        return impl_->server.listen(host.c_str(), port);
    }

    void HttpServer::stop()
    {
        impl_->server.stop();
    }
}
