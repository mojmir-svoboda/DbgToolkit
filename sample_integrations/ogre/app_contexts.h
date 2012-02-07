#pragma once

namespace trace {

    typedef unsigned context_t;

    static context_t const CTX_Default = (1 << 0);
    static context_t const CTX_Render  = (1 << 1);
    // ...
}
