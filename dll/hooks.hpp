#pragma once
#include <string>

namespace hooks {
    bool install();
    void uninstall();
    void queue_script(const std::string& script);
}
