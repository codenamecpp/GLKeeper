#pragma once

namespace cxx
{

    struct noncopyable 
    {
    public:
        noncopyable() = default;
        noncopyable(const noncopyable& rhs) = delete;
        noncopyable(noncopyable&& rhs) = delete;
        noncopyable& operator = (const noncopyable& rhs) = delete;
        noncopyable& operator = (noncopyable&& rhs) = delete;
    };

} // namespace cxx