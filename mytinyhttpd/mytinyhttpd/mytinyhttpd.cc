#include "mytinyhttpd/mytinyhttpd/mytinyhttpd.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <cstdint>
#include <iostream>
#include <memory>

#include "mytinyhttpd/http/http_server.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/utils/program_options.h"

using namespace mytinyhttpd;
using namespace mytinyhttpd::net;
using namespace boost::program_options;

const char* MyTinyHttpd::op_desc_ =
    "MyTinyHttpd version: 0.0.1 developed by NiGo\nUsage: mytinyhttpd [-?hv] "
    "[-p port] [-r docroot] [-m domain] [-f filename]\nOptions:";
const char* MyTinyHttpd::version_desc_ =
    "MyTinyHttpd version: 0.0.1 developed by NiGo";

void MyTinyHttpd::VerifyOptions(
    const boost::program_options::variables_map& vm) {
  VerifyExclusiveOption(vm, "help");
  VerifyExclusiveOption(vm, "version");
  VerifyExclusiveOption(vm, "conf");
}

int main(int argc, char* argv[]) {
  try {
    std::string config_file;
    HttpServerConfig cli_config;

    options_description desc(MyTinyHttpd::op_desc());
    desc.add_options()("help,h", "print usage message")(
        "version,v", "show version of mytinyhttpd")(
        "port,p",
        value<uint16_t>(&cli_config.port_)
            ->value_name("port")
            ->default_value(HttpServerConfig::default_port_),
        "set listening port")(
        "docroot,r",
        value<std::string>(&cli_config.docroot_)
            ->value_name("docroot")
            ->default_value(HttpServerConfig::default_docroot_),
        "set docroot path")(
        "domain,m",
        value<std::string>(&cli_config.domain_)
            ->value_name("domain")
            ->default_value(HttpServerConfig::default_domain_),
        "set domain")("conf,f",
                      value<std::string>(&config_file)
                          ->value_name("filename")
                          ->default_value(HttpServerConfig::default_conf_file_),
                      "set configuration file");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    vm.notify();
    MyTinyHttpd::VerifyOptions(vm);

    HttpServerConfig file_config(config_file);
    if (file_config.IsValid()) {
      cli_config.ReplaceIfNotDefault(file_config);
    }

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }
    if (vm.count("version")) {
      std::cout << MyTinyHttpd::version_desc() << std::endl;
      return 0;
    }

    EventLoop loop;
    HttpServer server(&loop, "MyTinyHttpd", &cli_config);
    server.SetThreadNum(8);
    server.Start();
    loop.Loop();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}