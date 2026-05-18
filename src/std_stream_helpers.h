#pragma once

namespace cxx
{

    // stream helpers
    template<typename TElement>
    inline bool read_elements(std::istream& instream, TElement* elements, int elements_count)
    {
        if (!instream.read((char*) elements, elements_count * sizeof(TElement)))
            return false;

        return true;
    }

    template<typename TElement>
    inline int read_some_elements(std::istream& instream, TElement* elements, int elements_count)
    {
        int numElements = 0;
        while (instream.good())
        {
            if (numElements == elements_count)
                break;

            if (!instream.read((char*)elements, sizeof(TElement)))
                break;

            ++elements;
            ++numElements;
        }
        
        return numElements;
    }

    inline unsigned char read_int8(std::istream& instream)
    {
        unsigned char resultData = 0;
        bool isSuccess = read_elements(instream, &resultData, 1);
        cxx_assert(isSuccess);
        return resultData;
    }

    inline unsigned short read_int16(std::istream& instream)
    {
        unsigned short resultData = 0;
        bool isSuccess = read_elements(instream, &resultData, 1);
        cxx_assert(isSuccess);
        return resultData;
    }

    inline unsigned int read_int32(std::istream& instream)
    {
        unsigned int resultData = 0;
        bool isSuccess = read_elements(instream, &resultData, 1);
        cxx_assert(isSuccess);
        return resultData;
    }

    inline bool read_cstring(std::istream& instream, char* output_buffer, int output_buffer_length)
    {
        if (!instream.getline(output_buffer, output_buffer_length, 0))
            return false;

        return true;
    }

} // namespace cxx