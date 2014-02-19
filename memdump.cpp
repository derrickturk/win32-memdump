#include "windows.h"

#include "process_memory.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cerrno>
#include <cstddef>
#include <cstdio>

std::size_t dump_process_memory(DWORD pid);

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << (argc ? argv[0] : "memdump")
            << " <pid> [output-file]\n";
        return 0;
    }

    DWORD pid = static_cast<DWORD>(std::strtoul(argv[1], nullptr, 10));
    if (pid == 0 || errno == ERANGE) {
        std::cerr << "Invalid argument: " << argv[1] << '\n';
        return 0;
    }

    std::ostream *os = nullptr;
    std::ofstream out;
    if (argc < 3) {
        FILE *bin_out = std::freopen(nullptr, "wb", stdout);
        if (bin_out != stdout) {
            std::cerr << "Warning: unable to reopen standard out in "
                "binary mode.\n";
        }
        os = &std::cout;
    } else {
        out.open(argv[2], std::ios_base::out | std::ios_base::binary);
        if (!out) {
            std::cerr << "Unable to open " << argv[2]
                << " for binary output.\n";
            return 0;
        }
        os = &out;
    }

    try {
        process_memory_iterator i(pid), end;
        for (; i != end; ++i)
            (*os) << *i;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}
