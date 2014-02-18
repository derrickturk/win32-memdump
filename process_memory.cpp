#include "process_memory.h"
#include <algorithm>
#include <utility>

process_memory_iterator::process_memory_iterator(DWORD pid)
    : process_memory_iterator()
{
    proc_ = OpenProcess(
            PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
            FALSE,
            pid)

    if (proc == NULL)
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
}

process_memory_iterator::process_memory_iterator(
        process_memory_iterator&& other) noexcept
{
    buf_ = std::move(other.buf_);
    buf_sz_ = other.buf_sz_;
    proc_ = other.proc_;
    base_ = other.base_;
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

    return *this;
}
