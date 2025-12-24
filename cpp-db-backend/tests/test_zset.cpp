#include "kv/zset.h"
#include <cassert>
#include <iostream>

int main() {
    kv::ZSet zset;

    // Test 1: Add members
    std::cout << "Test 1: Adding members...\n";
    assert(zset.add("alice", 100.0));
    assert(zset.add("bob", 200.0));
    assert(zset.add("charlie", 150.0));
    assert(zset.add("dave", 250.0));
    std::cout << "✓ Added 4 members\n";

    // Test 2: Check scores
    std::cout << "\nTest 2: Checking scores...\n";
    assert(zset.score("alice").value() == 100.0);
    assert(zset.score("bob").value() == 200.0);
    assert(zset.score("charlie").value() == 150.0);
    assert(zset.score("dave").value() == 250.0);
    assert(!zset.score("nonexistent").has_value());
    std::cout << "✓ All scores correct\n";

    // Test 3: Check ranks (sorted by score)
    std::cout << "\nTest 3: Checking ranks...\n";
    // Order should be: alice(100) < charlie(150) < bob(200) < dave(250)
    assert(zset.rank("alice").value() == 0);
    assert(zset.rank("charlie").value() == 1);
    assert(zset.rank("bob").value() == 2);
    assert(zset.rank("dave").value() == 3);
    assert(!zset.rank("nonexistent").has_value());
    std::cout << "✓ All ranks correct\n";

    // Test 4: Update score (should remove and re-add)
    std::cout << "\nTest 4: Updating score...\n";
    assert(zset.add("alice", 300.0));  // alice now has highest score
    assert(zset.score("alice").value() == 300.0);
    assert(zset.rank("alice").value() == 3);  // Now last (highest score)
    std::cout << "✓ Score update works\n";

    // Test 5: Range queries
    std::cout << "\nTest 5: Range queries...\n";
    auto range1 = zset.range(0, 1);  // First 2 members
    assert(range1.size() == 2);
    assert(range1[0].first == "charlie");
    assert(range1[0].second == 150.0);
    assert(range1[1].first == "bob");
    assert(range1[1].second == 200.0);
    std::cout << "✓ Range [0,1] correct\n";

    // Test 6: Negative indices
    std::cout << "\nTest 6: Negative indices...\n";
    auto range2 = zset.range(-2, -1);  // Last 2 members
    assert(range2.size() == 2);
    assert(range2[0].first == "dave");
    assert(range2[0].second == 250.0);
    assert(range2[1].first == "alice");
    assert(range2[1].second == 300.0);
    std::cout << "✓ Negative indices work\n";

    // Test 7: Get all elements
    std::cout << "\nTest 7: Get all elements...\n";
    auto all = zset.range(0, -1);
    assert(all.size() == 4);
    std::cout << "✓ Range [0,-1] returns all elements\n";

    // Test 8: Remove member
    std::cout << "\nTest 8: Removing member...\n";
    assert(zset.remove("bob"));
    assert(!zset.score("bob").has_value());
    assert(!zset.rank("bob").has_value());
    auto after_remove = zset.range(0, -1);
    assert(after_remove.size() == 3);
    std::cout << "✓ Remove works\n";

    // Test 9: Remove non-existent
    std::cout << "\nTest 9: Remove non-existent...\n";
    assert(!zset.remove("nonexistent"));
    std::cout << "✓ Remove non-existent returns false\n";

    // Test 10: Edge cases
    std::cout << "\nTest 10: Edge cases...\n";
    kv::ZSet empty_zset;
    auto empty_range = empty_zset.range(0, 10);
    assert(empty_range.empty());
    assert(!empty_zset.score("anything").has_value());
    std::cout << "✓ Empty zset handled correctly\n";

    std::cout << "\n✅ All ZSet tests passed!\n";
    return 0;
}
