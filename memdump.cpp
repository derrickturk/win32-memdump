#include "windows.h"

#include "process_memory.h"

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstddef>

std::size_t dump_process_memory(DWORD pid);

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << (argc ? argv[0] : "memdump") << " <pid>\n";
        return 0;
    }

    DWORD pid = static_cast<DWORD>(std::strtoul(argv[1], nullptr, 1));
    if (pid == 0 || errno == ERANGE) {
        std::cerr << "Invalid argument: " << argv[1] << '\n';
        return 0;
    }

    process_memory_iterator i(pid), end;

    while (i != end) {
        std::cout << *i;
        ++i;
    }
}
