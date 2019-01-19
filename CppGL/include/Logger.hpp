#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

typedef std::shared_ptr<spdlog::logger> Console;

class Logger {
public:
    virtual ~Logger() = default;

    explicit Logger(const std::string &name) {
        if ((console = spdlog::get(name)) == nullptr) {
            console = spdlog::stdout_color_mt(name);
        }
    }
protected:
    Console console;
};
