#include "TestChecks.h"
#include <iostream>
#include "internpath/domain/Date.h"

using namespace internpath;

int main()
{
    CHECK(isValidDate(20261020));
    CHECK(isValidDate(20240229));
    CHECK(!isValidDate(20230229));
    CHECK(!isValidDate(20261301));
    CHECK(!isValidDate(20260431));
    CHECK(!isValidDate(0));
    std::cout << "Date tests passed\n";
    return 0;
}
