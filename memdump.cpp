#include "windows.h"

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstddef>

std::size_t dump_process_memory(DWORD pid);

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argc ? argv[0] << " <pid>\n";
        return 0;
    }

    DWORD pid = static_cast<DWORD>(std::strtoul(argv[1], nullptr, 1));
    if (pid == 0 || errno == ERANGE) {
        std::cerr << "Invalid argument: " << argv[1] << '\n';
        return 0;
    }

    try {
        static_cast<void>(dump_process_memory(pid));
    } catch (const process_open_exception& e) {
    }
}

std::size_t dump_process_memory(DWORD pid)
{
    HANDLE proc = OpenProcess(
            PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
            FALSE,
            pid)

    if (proc == NULL)
        throw process_open_exception(GetLastError);


}
