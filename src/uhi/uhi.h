#pragma once
#include "../search/limits.h"

class UHI {
public:
    void uhi_loop();

private:
    void listen_for_options();

private:
    SearchLimits limits_;
};