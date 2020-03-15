#pragma once

namespace cxx
{
    
    // remove from file path forbidden characters
    inline void path_remove_forbidden_chars(std::string& input_string)
    {
        static const std::string forbidden_chars {"\\/:?\"<>|"};
        std::transform(input_string.begin(), input_string.end(), input_string.begin(), [](const char c)
            {
                if (forbidden_chars.find(c) != std::string::npos)
                    return '_';
                    
                return c;
            });
    }

    // remove file extension (including dot symbol)
    inline void path_remove_extension(std::string& input_string)
    {
        auto dot_character_pos = input_string.find_last_of('.');
        if (dot_character_pos != std::string::npos)
        {
            input_string.erase(dot_character_pos, input_string.length() - dot_character_pos);
        }
    }

} // namespace cxx