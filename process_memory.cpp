#include "process_memory.h"
#include <algorithm>
#include <utility>

#include <iostream>

process_memory_iterator::process_memory_iterator(DWORD pid)
    : process_memory_iterator()
{
    proc_ = std::shared_ptr<HANDLE>(
            new HANDLE(
                OpenProcess(
                    PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                    FALSE,
                    pid)
                ),
            [](HANDLE* p) { CloseHandle(*p); }
    );

    if (*proc_ == NULL)
        throw process_open_exception("Unable to obtain process handle.");

    update_pagerange();
}

unsigned char process_memory_iterator::operator*() const
{
    if (!*this)
        throw invalid_iterator("Attempt to dereference bad iterator.");

    return buf_.get()[buf_off_];
}

process_memory_iterator& process_memory_iterator::operator++()
{
    if (buf_off_ == buf_sz_)
        update_pagerange();
    else
        ++buf_off_;

    return *this;
}

void process_memory_iterator::update_pagerange()
{
    MEMORY_BASIC_INFORMATION info;
    // technically this isn't portable because it is possible that nullptr != 0
    // but then, look around
    void *base = static_cast<unsigned char*>(base_) + buf_sz_;

    do { // query until we find a committed private or mapped region
        std::size_t res = VirtualQueryEx(
                *proc_,
                base,
                &info,
                sizeof(info)
        );

        if (!res) {
            if (GetLastError() == ERROR_INVALID_PARAMETER) {
                // we've walked off the end of the process's
                // address space
                process_memory_iterator bad;
                std::swap(*this, bad);
                return;
            } else {
                // something else (bad) happened
                throw memory_query_exception("Unable to query virtual memory.");
            }
        }

        // TODO: it may be possible to remove PAGE_GUARD with VirtualProtectEx
        if (info.State == MEM_COMMIT
                && (info.Type == MEM_MAPPED
                    || info.Type == MEM_PRIVATE)
                && !(info.Protect == 0
                    || info.Protect & PAGE_NOACCESS
                    || info.Protect & PAGE_GUARD))
            break;
        else
            base = static_cast<unsigned char*>(info.BaseAddress) + info.RegionSize;
    } while (true);

    read_memory(static_cast<void*>(info.BaseAddress),
            static_cast<std::size_t>(info.RegionSize));
}

void process_memory_iterator::read_memory(void* new_base, std::size_t new_size)
{
    std::shared_ptr<unsigned char> new_buf(
            new unsigned char[new_size],
            std::default_delete<unsigned char[]>());
    if (ReadProcessMemory(
                *proc_,
                new_base,
                new_buf.get(),
                new_size,
                nullptr) == 0)
        throw memory_read_exception("Unable to read memory.");

    base_ = new_base;
    buf_ = std::move(new_buf);
    buf_sz_ = new_size;
    buf_off_ = 0;
}
