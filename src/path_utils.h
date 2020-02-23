#pragma once

namespace cxx
{
    
    // remove from file path forbidden characters
    inline void path_remove_forbidden_chars(std::string& input_string)
    {
        static const std::string forbiddenChars {"\\/:?\"<>|"};
        std::transform(input_string.begin(), input_string.end(), input_string.begin(), [](const char c)
            {
                if (forbiddenChars.find(c) != std::string::npos)
                    return '_';
                    
                return c;
            });
    }

} // namespace cxx