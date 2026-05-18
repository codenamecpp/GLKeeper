#pragma once

#include <iosfwd>
#include <istream>

namespace cxx
{
    class memory_istream: public std::streambuf
    {
    public:
        memory_istream(char* memory_begin, char* memory_end) 
            : mBegin(memory_begin)
            , mEnd(memory_end)
        {
            this->setg(memory_begin, memory_begin, memory_end);
        }
    
    private:
        // override streambuf methods
        pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
        {
            switch (dir)
            {
                case std::ios_base::cur:
                        this->gbump(static_cast<int>(off));
                    break;
                case std::ios_base::end:
                        this->setg(mBegin, mEnd + off, mEnd);
                    break;
                case std::ios_base::beg:
                        this->setg(mBegin, mBegin + off, mEnd);
                    break;
            }

            return this->gptr() - this->eback();
        }
 
        pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
        {
            return this->seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
        }

    private:
        char* mBegin; 
        char* mEnd;
    };

} // namespace cxx