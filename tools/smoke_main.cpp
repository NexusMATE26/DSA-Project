#include <iostream>

#include "internpath/domain/Matching.h"

using namespace std;

int main()
{
    internpath::SkillComparison example[] = {
        {2, 2, 5, true},  // C++
        {1, 2, 4, true},  // Data Structures
        {1, 1, 2, false}  // Git
    };

    internpath::MatchResult match = internpath::calculateMatch(example, 3);

    cout << "InternPath — Milestone 0 developer smoke check" << endl;
    cout << "Match = " << match.numerator << "/" << match.denominator;
    cout << " (about " << match.roundedPercent() << "%)" << endl;
    cout << "Eligible = " << (match.eligible ? "yes" : "no") << endl;
    cout << "Next: finish custom DSA modules, then services and local web API." << endl;
    return match.valid ? 0 : 1;
}
