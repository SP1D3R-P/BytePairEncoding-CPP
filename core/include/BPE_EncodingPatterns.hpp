# pragma once



// GPT4 pattern
#define __GPT4_REGEX_PATTERN_1__ "[^\\r\\n\\p{L}\\p{N}]?[\\p{Lu}\\p{Lt}\\p{Lm}\\p{Lo}\\p{M}]*[\\p{Ll}\\p{Lm}\\p{Lo}\\p{M}]+(?i:'s|'t|'re|'ve|'m|'ll|'d)?"
#define __GPT4_REGEX_PATTERN_2__ "[^\\r\\n\\p{L}\\p{N}]?[\\p{Lu}\\p{Lt}\\p{Lm}\\p{Lo}\\p{M}]+[\\p{Ll}\\p{Lm}\\p{Lo}\\p{M}]*(?i:'s|'t|'re|'ve|'m|'ll|'d)?"
#define __GPT4_REGEX_PATTERN_3__ "[0-9]{1,3}"
#define __GPT4_REGEX_PATTERN_4__ " ?[^\\s\\p{L}\\p{N}]+[\\r\\n/]*"
#define __GPT4_REGEX_PATTERN_5__ "\\s*[\\r\\n]+"
#define __GPT4_REGEX_PATTERN_6__ "\\s+(?!\\S)"
#define __GPT4_REGEX_PATTERN_7__ "\\s+"


#define GPT4_PATTERN {           \
    __GPT4_REGEX_PATTERN_1__ "|" \
    __GPT4_REGEX_PATTERN_2__ "|" \
    __GPT4_REGEX_PATTERN_3__ "|" \
    __GPT4_REGEX_PATTERN_4__ "|" \
    __GPT4_REGEX_PATTERN_5__ "|" \
    __GPT4_REGEX_PATTERN_6__ "|" \
    __GPT4_REGEX_PATTERN_7__     \
}




// clk100k_BASE
#define CL100K_BASE_PATTERN \
            R"('(?i:[sdmt]|ll|ve|re)|[^\r\n\p{L}\p{N}]?+\p{L}++|\p{N}{1,3}+| ?[^\s\p{L}\p{N}]++[\r\n]*+|\s++$|\s*[\r\n]|\s+(?!\S)|\s)"