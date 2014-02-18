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

    if (*proc == NULL)
        throw process_open_exception(GetLastError);
}

process_memory_iterator::process_memory_iterator(
        const process_memory_iterator& other)
{
    buf_ = std::make_unique(other.buf_sz_);
    buf_sz_ = other.buf_sz_;
    std::copy(other.buf_.get(), other.buf_.get() + other.buf_sz,
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
    base_ = other.base_;
    buf_ = other.buf_;
    buf_sz_ = other.buf_sz_;
    buf_off_ = other.buf_off_;

    return *this;
}

unsigned char process_memory_iterator::operator*()
{
    if (!this)
        throw invalid_iterator();

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
    void *base = base_ + buf_sz;

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
                std::swap(*this, process_memory_iterator());
                return;
            } else {
                // something else (bad) happened
                throw memory_query_exception();
            }
        }

        if (info.State == MEM_COMMIT
                && (info.Protect == MEM_MAPPED
                    || info.Protect == MEM_PRIVATE))
            break;
        else
            base = info.BaseAddress + info.RegionSize;
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
        throw memory_read_exception();

    base_ = new_base;
    buf_ = std::move(new_buf);
    buf_sz_ = new_size;
    buf_off_ = 0;
}
