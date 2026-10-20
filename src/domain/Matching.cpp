#include "internpath/domain/Matching.h"

namespace internpath
{
    int MatchResult::roundedPercent() const
    {
        if (!valid || denominator <= 0)
        {
            return 0;
        }

        return static_cast<int>((100LL * numerator + denominator / 2) / denominator);
    }

    MatchResult calculateMatch(const SkillComparison* skills, std::size_t count)
    {
        MatchResult result;

        if (skills == nullptr || count == 0)
        {
            return result;
        }

        int totalWeight = 0;
        int numerator = 0;
        bool eligible = true;

        for (std::size_t i = 0; i < count; i++)
        {
            const SkillComparison& item = skills[i];

            if (item.studentLevel > 3 || item.requiredLevel < 1 ||
                item.requiredLevel > 3 || item.weight < 1 || item.weight > 5)
            {
                return {};
            }

            int earnedLevel = item.studentLevel;

            if (earnedLevel > item.requiredLevel)
            {
                earnedLevel = item.requiredLevel;
            }

            numerator += item.weight * earnedLevel * (6 / item.requiredLevel);
            totalWeight += item.weight;

            if (item.mandatory && item.studentLevel < item.requiredLevel)
            {
                eligible = false;
            }
        }

        result.valid = true;
        result.eligible = eligible;
        result.numerator = numerator;
        result.denominator = 6 * totalWeight;
        return result;
    }

    int compareFit(const MatchResult& a, const MatchResult& b)
    {
        if (!a.valid || !b.valid || a.denominator <= 0 || b.denominator <= 0)
        {
            return 0;
        }

        long long left = static_cast<long long>(a.numerator) * b.denominator;
        long long right = static_cast<long long>(b.numerator) * a.denominator;

        if (left < right)
        {
            return -1;
        }

        if (left > right)
        {
            return 1;
        }

        return 0;
    }
}
