#pragma once

namespace cxx
{
    // noncopyable
    struct noncopyable
    {
    public:
        noncopyable() = default;
        noncopyable(const noncopyable&) = delete;
        noncopyable(noncopyable&&) = delete;
        noncopyable& operator = (const noncopyable&) = delete;
        noncopyable& operator = (noncopyable&&) = delete;
    };

} // namespace cxx