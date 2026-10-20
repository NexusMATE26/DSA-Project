#include "../TestChecks.h"
#include <stdexcept>
#include <iostream>
#include "internpath/algo/Sorting.h"
#include "internpath/algo/Searching.h"

using namespace internpath;

int main()
{
    try
    {
    int numbers[] = {7, 2, 9, 2, 5};
    auto ascending = [](int first, int second)
    {
        return first < second;
    };

    mergeSort(numbers, 5, ascending);
    CHECK(numbers[0] == 2 && numbers[4] == 9);
    CHECK(linearSearch(numbers, 5, 5) == 2);
    CHECK(lowerBound(numbers, 5, 5, ascending) == 2);
    std::cout << "Sort/search exercises passed\n";
    return 0;
    }
    catch (const std::logic_error& error)
    {
        std::cerr << "Incomplete exercise: " << error.what() << "\n";
        return 1;
    }
}
