#pragma once

#include <md4c.h>

namespace startgit
{

int md_html(
    const MD_CHAR* input,
    MD_SIZE input_size,
    void (*process_output)(const MD_CHAR*, MD_SIZE, void*),
    void* userdata,
    unsigned parser_flags,
    unsigned renderer_flags
);

}  // namespace startgit
