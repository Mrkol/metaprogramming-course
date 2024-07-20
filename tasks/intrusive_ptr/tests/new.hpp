#pragma once

#include <memory>

static int AllocCount = 0;
static int DeleteCount = 0;

void* operator new(std::size_t sz)
{
    ++AllocCount;

    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}

void operator delete(void* ptr) noexcept
{
    ++DeleteCount;
    std::free(ptr);
}
