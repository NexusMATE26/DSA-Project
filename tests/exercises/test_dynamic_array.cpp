#include "../TestChecks.h"
#include <stdexcept>
#include <iostream>
#include "internpath/ds/DynamicArray.h"

using namespace internpath;

int main()
{
    try
    {
    DynamicArray<int> values;
    values.pushBack(10);
    values.pushBack(20);
    CHECK(values.size() == 2);
    CHECK(values.at(0) == 10);
    CHECK(values.at(1) == 20);
    values.eraseAt(0);
    CHECK(values.size() == 1 && values.at(0) == 20);
    std::cout << "DynamicArray exercise passed\n";
    return 0;
    }
    catch (const std::logic_error& error)
    {
        std::cerr << "Incomplete exercise: " << error.what() << "\n";
        return 1;
    }
}
