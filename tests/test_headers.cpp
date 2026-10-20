#include "TestChecks.h"
#include "internpath/domain/Types.h"
#include "internpath/domain/Date.h"
#include "internpath/domain/Result.h"
#include "internpath/domain/Models.h"
#include "internpath/domain/Matching.h"
#include "internpath/ds/DynamicArray.h"
#include "internpath/ds/SinglyLinkedList.h"
#include "internpath/ds/DoublyLinkedList.h"
#include "internpath/ds/Stack.h"
#include "internpath/ds/CircularQueue.h"
#include "internpath/ds/Hashing.h"
#include "internpath/ds/HashTable.h"
#include "internpath/ds/DeadlineAVL.h"
#include "internpath/ds/DirectedGraph.h"
#include "internpath/algo/Sorting.h"
#include "internpath/algo/Searching.h"
#include "internpath/algo/GraphAlgorithms.h"
#include "internpath/core/AppState.h"
#include "internpath/core/ServiceContracts.h"
#include "internpath/core/WaitingList.h"
#include "internpath/core/UndoHistory.h"

int main()
{
    using namespace internpath;

    // 1. DynamicArray & Rule of Five
    {
        DynamicArray<int> arr;
        for (int i = 0; i < 10; i++)
        {
            arr.pushBack(i * 10);
        }
        CHECK(arr.size() == 10);
        CHECK(arr.at(3) == 30);

        // Copy constructor
        DynamicArray<int> copyArr = arr;
        CHECK(copyArr.size() == 10);
        CHECK(copyArr.at(3) == 30);

        // Move constructor
        DynamicArray<int> movedArr = std::move(copyArr);
        CHECK(movedArr.size() == 10);
        CHECK(copyArr.size() == 0);

        // Erase
        arr.eraseAt(0);
        CHECK(arr.size() == 9);
        CHECK(arr.at(0) == 10);
    }

    // 2. SinglyLinkedList & removeIf
    {
        SinglyLinkedList<int> sll;
        sll.pushBack(100);
        sll.pushBack(200);
        sll.pushBack(300);
        CHECK(sll.size() == 3);

        CHECK(sll.removeFirst(100));
        CHECK(sll.size() == 2);

        CHECK(sll.removeIf([](int v) { return v == 300; }));
        CHECK(sll.size() == 1);
        CHECK(!sll.removeFirst(999));
    }

    // 3. DoublyLinkedList & Handles
    {
        DoublyLinkedList<int> dll;
        auto h1 = dll.pushBack(10);
        auto h2 = dll.pushBack(20);
        auto h3 = dll.pushBack(30);
        (void)h1;
        (void)h3;
        CHECK(dll.size() == 3);

        dll.erase(h2);
        CHECK(dll.size() == 2);

        int val = 0;
        CHECK(dll.popFront(val));
        CHECK(val == 10);
        CHECK(dll.popFront(val));
        CHECK(val == 30);
        CHECK(!dll.popFront(val));
    }

    // 4. Stack
    {
        Stack<int> st;
        st.push(5);
        st.push(10);
        CHECK(st.top() == 10);
        int popped = 0;
        CHECK(st.pop(popped));
        CHECK(popped == 10);
        CHECK(st.top() == 5);
        CHECK(st.pop(popped));
        CHECK(st.empty());
    }

    // 5. CircularQueue
    {
        CircularQueue<int> q;
        for (int i = 0; i < 20; i++)
        {
            q.enqueue(i);
        }
        CHECK(q.size() == 20);

        int out = 0;
        for (int i = 0; i < 10; i++)
        {
            CHECK(q.dequeue(out));
            CHECK(out == i);
        }
        CHECK(q.size() == 10);

        for (int i = 20; i < 30; i++)
        {
            q.enqueue(i);
        }
        CHECK(q.size() == 20);
    }

    // 6. Hashing
    {
        CHECK(hashNumber(42) != 0);
        CHECK(hashText("InternPath") != 0);
        CHECK(hashText("InternPath") == hashText("InternPath"));
        CHECK(hashText("A") != hashText("B"));
    }

    // 7. HashTable
    {
        HashTable<std::string, int> table;
        for (int i = 0; i < 50; i++)
        {
            table.insertOrAssign("key_" + std::to_string(i), i * 10);
        }
        CHECK(table.size() == 50);
        CHECK(table.validate());

        int res = 0;
        CHECK(table.tryGet("key_25", res));
        CHECK(res == 250);

        CHECK(table.erase("key_25"));
        CHECK(!table.tryGet("key_25", res));
        CHECK(table.size() == 49);
    }

    // 8. DeadlineAVL
    {
        DeadlineAVL avl;
        avl.insert({20261101, 1});
        avl.insert({20261015, 2});
        avl.insert({20261220, 3});
        avl.insert({20261025, 4});
        avl.insert({20261115, 5});
        CHECK(avl.size() == 5);
        CHECK(avl.validate());

        DynamicArray<std::uint32_t> inRange;
        avl.range(20261020, 20261110, inRange);
        CHECK(inRange.size() == 2); // 20261025 (4) and 20261101 (1)

        CHECK(avl.erase({20261015, 2}));
        CHECK(avl.size() == 4);
        CHECK(avl.validate());
    }

    // 9. DirectedGraph & GraphAlgorithms
    {
        DirectedGraph g;
        auto v0 = g.addVertex(); // 0
        auto v1 = g.addVertex(); // 1
        auto v2 = g.addVertex(); // 2
        auto v3 = g.addVertex(); // 3

        CHECK(g.addEdge(v0, v1, 1));
        CHECK(g.addEdge(v1, v2, 1));
        CHECK(g.addEdge(v2, v3, 1));

        // Adding v3 -> v0 should fail due to cycle prevention!
        CHECK(!g.addEdge(v3, v0, 1));

        CHECK(isReachable(g, v0, v3));
        CHECK(!isReachable(g, v3, v0));

        DynamicArray<std::uint32_t> topo;
        CHECK(topologicalOrder(g, topo));
        CHECK(topo.size() == 4);
        CHECK(topo.at(0) == v0);
        CHECK(topo.at(3) == v3);
    }

    // 10. WaitingList & UndoHistory
    {
        WaitingList wl;
        CHECK(wl.append(101));
        CHECK(wl.append(102));
        CHECK(wl.append(103));
        CHECK(wl.size() == 3);
        CHECK(wl.contains(102));

        CHECK(wl.withdraw(102));
        CHECK(wl.size() == 2);
        CHECK(!wl.contains(102));

        ApplicationId promoted = 0;
        CHECK(wl.promoteHead(promoted));
        CHECK(promoted == 101);
        CHECK(wl.size() == 1);

        UndoHistory undo;
        undo.recordEdit({1, 1, 1, 2});
        undo.recordEdit({1, 1, 2, 3});
        CHECK(undo.size() == 2);

        ProfileEdit edit;
        CHECK(undo.popUndo(edit));
        CHECK(edit.newLevel == 3);
        CHECK(undo.size() == 1);
    }

    std::cout << "All header contracts and custom DSA test suites passed!\n";
    return 0;
}
