#pragma once

#include <cstddef>
#include <cstdint>

namespace internpath
{
    // This is deliberately independent of repositories and the web interface.
    // Services will convert requirement IDs into student levels before calling it.
    struct SkillComparison
    {
        std::uint8_t studentLevel = 0;  // 0..3
        std::uint8_t requiredLevel = 1; // 1..3
        std::uint8_t weight = 1;        // 1..5
        bool mandatory = false;
    };

    struct MatchResult
    {
        bool valid = false;
        bool eligible = false;
        std::int32_t numerator = 0;
        std::int32_t denominator = 0;

        // Rounded integer percentage for display only; NEVER sort by this.
        int roundedPercent() const;
    };

    // Fit fraction = sum(w * min(student, required) * (6 / required)) /
    //                (6 * sum(w)). Exact with levels 1..3.
    // Returns invalid if no requirements or an out-of-range input is supplied.
    MatchResult calculateMatch(const SkillComparison* skills, std::size_t count);

    // Accurate ordering across internships without comparing rounded percentages.
    // Returns -1 if a fits less, 0 if equally, +1 if a fits more.
    int compareFit(const MatchResult& a, const MatchResult& b);
}
