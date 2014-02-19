#include "process_memory.h"
#include <algorithm>
#include <utility>

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

process_memory_iterator::process_memory_iterator(
        const process_memory_iterator& other)
{
    buf_ = std::unique_ptr<unsigned char[]>(new unsigned char[other.buf_sz_]);
    buf_sz_ = other.buf_sz_;
    std::copy(other.buf_.get(), other.buf_.get() + other.buf_sz_,
            buf_.get());

    proc_ = other.proc_;
    base_ = other.base_;
    buf_off_ = other.buf_off_;
}

process_memory_iterator::process_memory_iterator(
        process_memory_iterator&& other) noexcept
{
    buf_ = std::move(other.buf_);
    buf_sz_ = other.buf_sz_;

    proc_ = other.proc_;
    base_ = other.base_;
    buf_off_ = other.buf_off_;
}

process_memory_iterator&
process_memory_iterator::operator=(
        const process_memory_iterator& other) noexcept
{
    process_memory_iterator i(other);
    std::swap(*this, i);
    return *this;
}

process_memory_iterator&
process_memory_iterator::operator=(
        process_memory_iterator&& other) noexcept
{
    std::swap(buf_, other.buf_);
    buf_sz_ = other.buf_sz_;

    proc_ = other.proc_;
    base_ = other.base_;
    buf_off_ = other.buf_off_;

    return *this;
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

        if (info.State == MEM_COMMIT
                && (info.Type == MEM_MAPPED
                    || info.Type == MEM_PRIVATE))
            break;
        else
            base = static_cast<unsigned char*>(info.BaseAddress) + info.RegionSize;
    } while (true);

    read_memory(static_cast<void*>(info.BaseAddress),
            static_cast<std::size_t>(info.RegionSize));
}

void process_memory_iterator::read_memory(void* new_base, std::size_t new_size)
{
    std::unique_ptr<unsigned char[]> new_buf(new unsigned char[new_size]);
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
