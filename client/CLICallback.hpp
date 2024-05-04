#ifndef CLI_CALLBACK_H
#define CLI_CALLBACK_H

#include <memory>

#include "Connection.hpp"

class CLICallback {
public:
    CLICallback(std::shared_ptr<Connection>);
};

#endif
