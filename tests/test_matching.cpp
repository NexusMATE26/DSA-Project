#include "TestChecks.h"
#include <iostream>

#include "internpath/domain/Matching.h"

using namespace std;
using namespace internpath;

int main()
{
    SkillComparison requirements[] = {
        {2, 2, 5, true},
        {1, 2, 4, true},
        {1, 1, 2, false}
    };

    MatchResult result = calculateMatch(requirements, 3);
    CHECK(result.valid);
    CHECK(!result.eligible);
    CHECK(result.numerator == 54);
    CHECK(result.denominator == 66);
    CHECK(result.roundedPercent() == 82);

    SkillComparison fullyReady[] = {
        {3, 2, 5, true},
        {2, 2, 4, true},
        {1, 1, 2, false}
    };

    MatchResult ready = calculateMatch(fullyReady, 3);
    CHECK(ready.valid && ready.eligible);
    CHECK(ready.numerator == ready.denominator);
    CHECK(compareFit(ready, result) > 0);
    CHECK(compareFit(result, ready) < 0);

    SkillComparison invalid[] = {{4, 1, 3, false}};
    CHECK(!calculateMatch(invalid, 1).valid);
    CHECK(!calculateMatch(nullptr, 0).valid);
    cout << "Matching tests passed" << endl;
    return 0;
}
