// ─────────────────────────────────────────────────────────────────────────────
// tests.cpp  –  Unit tests for CampusCompass
//
// Compile (with real Catch2 installed):
//   g++ -std=c++17 -DCATCH_TESTING -I<catch2-include-dir> \
//       tests.cpp Graph.cpp -o tests && ./tests
//
// The CATCH_TESTING define suppresses the duplicate main() in main.cpp.
// ─────────────────────────────────────────────────────────────────────────────
 
#define CATCH_TESTING
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include "main.cpp"
#include "graphs.h"
#include "students.h"
using namespace std;
 
// ─── Helper that builds a loaded CampusCompass ────────────────────────────────────
static CampusCompass makeCC() {
    CampusCompass cc;
    cc.loadEdges("../data/edges.csv");
    cc.loadClasses("../data/classes.csv");
    return cc;
}

// 1. INCORRECT / INVALID COMMANDS  (≥ 5 cases)  [1 point]
 
TEST_CASE("Invalid insert - name contains digits", "[invalid]") {
    CampusCompass cc = makeCC();
    // "A11y" contains '1' – violates [a-z, A-Z, space] rule
    string result = cc.processLine(R"(insert "A11y" 45679999 1 1 COP3530)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - student ID too short (7 digits)", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "Alice" 1234567 1 1 COP3530)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - student ID too long (9 digits)", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "Alice" 123456789 1 1 COP3530)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - class count N=0 (below minimum)", "[invalid]") {
    CampusCompass cc = makeCC();
    // N must be between 1 and 6
    string result = cc.processLine(R"(insert "Alice" 12345678 1 0)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - class count N=7 (above maximum)", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(
        R"(insert "Alice" 12345678 1 7 COP3530 COP3503 COP3504 CDA3101 MAC2311 MAC2312 MAC2313)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - malformed class code (lowercase letters)", "[invalid]") {
    CampusCompass cc = makeCC();
    // class code must be 3 uppercase letters + 4 digits
    string result = cc.processLine(R"(insert "Bob" 12345678 1 1 cop3530)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - class code that does not exist in classes.csv", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "Bob" 12345678 1 1 XYZ9999)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid command spelling is rejected", "[invalid]") {
    CampusCompass cc = makeCC();
    // Misspelled command
    string result = cc.processLine("insrt \"Alice\" 12345678 1 1 COP3530");
    REQUIRE(result == "unsuccessful");
}
 
// 2. EDGE CASES  (≥ 3 cases)  [1 point]
 
TEST_CASE("Edge case - remove student that does not exist", "[edge]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine("remove 99999999");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Edge case - duplicate student ID is rejected on second insert", "[edge]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 COP3530)");
    // Same ID a second time must fail
    string result = cc.processLine(R"(insert "Bob" 12345678 1 1 MAC2311)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Edge case - dropClass on last class auto-removes student", "[edge]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Solo" 11111111 1 1 COP3530)");
    REQUIRE(cc.students.count("11111111") == 1);
    // Drop the only class → student should be erased automatically
    cc.processLine("dropClass 11111111 COP3530");
    REQUIRE(cc.students.count("11111111") == 0);
}
 
TEST_CASE("Edge case - removeClass with no students enrolled returns unsuccessful", "[edge]") {
    CampusCompass cc = makeCC();
    // COP3530 exists in classes.csv but no students are enrolled
    string result = cc.processLine("removeClass COP3530");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Edge case - replaceClass fails when student already has target class", "[edge]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 2 COP3530 MAC2311)");
    // Try to replace COP3530 with MAC2311 which she already has
    string result = cc.processLine("replaceClass 12345678 COP3530 MAC2311");
    REQUIRE(result == "unsuccessful");
}
 
// 3. dropClass / removeClass / remove / replaceClass  [1 point]
 
TEST_CASE("dropClass - successfully drops one of multiple classes", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 2 COP3530 MAC2311)");
    string result = cc.processLine("dropClass 12345678 COP3530");
    REQUIRE(result == "successful");
    // student still exists with one class
    REQUIRE(cc.students.count("12345678") == 1);
    REQUIRE(cc.students.at("12345678").classes.count("COP3530") == 0);
    REQUIRE(cc.students.at("12345678").classes.count("MAC2311") == 1);
}
 
TEST_CASE("dropClass - fails when student does not have the class", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 MAC2311)");
    string result = cc.processLine("dropClass 12345678 COP3530");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("removeClass - drops class from all enrolled students and returns count", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 10000001 1 1 COP3530)");
    cc.processLine(R"(insert "Bob"   10000002 1 2 COP3530 MAC2311)");
    cc.processLine(R"(insert "Carol" 10000003 1 1 MAC2311)");
    // Only Alice and Bob are in COP3530
    string result = cc.processLine("removeClass COP3530");
    REQUIRE(result == "2");
    // Alice had only COP3530 → auto-removed
    REQUIRE(cc.students.count("10000001") == 0);
    // Bob still exists with MAC2311
    REQUIRE(cc.students.count("10000002") == 1);
    REQUIRE(cc.students.at("10000002").classes.count("COP3530") == 0);
    // Carol unaffected
    REQUIRE(cc.students.count("10000003") == 1);
}
 
TEST_CASE("remove - successfully removes an existing student", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 COP3530)");
    string result = cc.processLine("remove 12345678");
    REQUIRE(result == "successful");
    REQUIRE(cc.students.count("12345678") == 0);
}
 
TEST_CASE("remove - fails for non-existent student", "[commands]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine("remove 00000000");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("replaceClass - successfully swaps one class for another", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 COP3530)");
    string result = cc.processLine("replaceClass 12345678 COP3530 MAC2311");
    REQUIRE(result == "successful");
    REQUIRE(cc.students.at("12345678").classes.count("COP3530") == 0);
    REQUIRE(cc.students.at("12345678").classes.count("MAC2311") == 1);
}
 
TEST_CASE("replaceClass - fails when old class not in student schedule", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 MAC2311)");
    string result = cc.processLine("replaceClass 12345678 COP3530 MAC2312");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("replaceClass - fails when new class code does not exist", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 COP3530)");
    string result = cc.processLine("replaceClass 12345678 COP3530 XYZ9999");
    REQUIRE(result == "unsuccessful");
}
 
// 4. printShortestEdges: reachable → close edges → unreachable  [2 points]
//
// Campus graph path from node 1 (Hume Hall) to node 14 (Turlington Hall)
// where COP3530 is located:
//   1 ──(2)──> 4 ──(6)──> 5  ──(...)  shortest = 1->4->... ->14
//   Also: 1->2->4->... etc.
//
// Strategy: find which edges are on ALL paths from node 1 to node 14,
// then close ALL of them so the student truly cannot reach COP3530.
//
// From the edge list, shortest path 1→14:
//   1→4 (2) → 2→4 alternative ... let's trace:
//   1→2 (3), 2→4 (3), 4→5 (6), ... long
//   1→4 (2), 4→5 (6) → 5→7 (5) → ... not toward 14
//   Actually shortest: 1→4(2), 2→4... let's use the graph directly.
//
// Rather than manual tracing, we use node 49 (New Engineering Building)
// which connects ONLY via node 7 (CSE) and node 56 (New Physics Building).
// STA3032 and EEL3701 are both at node 49.
// Node 49 connects to: 7 (weight 6) and 56 (weight 1).
// Close BOTH those edges → node 49 becomes unreachable.
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("printShortestEdges - reachable then unreachable after closing bridges", "[shortest]") {
    CampusCompass cc = makeCC();
 
    // Student lives at node 1 (Hume Hall), enrolled in EEL3701 (location 49)
    // Edges to node 49: 7↔49 (weight 6) and 49↔56 (weight 1)
    // Close both → node 49 becomes a disconnected island
    cc.processLine(R"(insert "Manav Sanghvi" 99991111 1 1 EEL3701)");
 
    // ── Before closing: student can reach EEL3701 at node 49 ─────────────────
    string before = cc.processLine("printShortestEdges 99991111");
    // The time should be a positive integer (path exists)
    REQUIRE(before.find("EEL3701: -1") == string::npos);
    REQUIRE(before.find("EEL3701:") != string::npos);
 
    // ── Close both bridges to node 49 ────────────────────────────────────────
    // Edge 7↔49 and edge 49↔56
    cc.processLine("toggleEdgesClosure 2 7 49 49 56");
 
    // ── After closing: node 49 is unreachable → time must be -1 ──────────────
    string after = cc.processLine("printShortestEdges 99991111");
    REQUIRE(after.find("EEL3701: -1") != string::npos);
 
    // ── Re-open one bridge → reachable again ─────────────────────────────────
    cc.processLine("toggleEdgesClosure 1 7 49");
    string restored = cc.processLine("printShortestEdges 99991111");
    REQUIRE(restored.find("EEL3701: -1") == string::npos);
}
 

//5. END-TO-END parse Input output match

 
TEST_CASE("End-to-end parseInput matches expected output", "[e2e]") {
    CampusCompass cc = makeCC();
 
    string input = R"(6
insert "Student A" 10000001 1 1 COP3502
insert "Student B" 10000002 1 1 COP3502
insert "Student C" 10000003 1 2 COP3502 MAC2311
dropClass 10000001 COP3502
remove 10000001
removeClass COP3502
)";
    // dropClass on Student A's only class → auto-removes the student
    // remove 10000001 → now gone, so remove returns unsuccessful
    // removeClass COP3502 → only Student B still had it → "2" was pre-drop state
    // After dropClass+remove, only Student B (10000002) has COP3502
    // Wait — Student A was auto-removed by dropClass. Then remove 10000001 → unsuccessful.
    // removeClass COP3502: Student B has it → count = 1... but template says "2".
    // Re-reading: dropClass 10000001 COP3502 succeeds (auto-removes student A).
    // remove 10000001 → already gone → unsuccessful.
    // removeClass COP3502: Student B (10000002) and Student C (10000003) still have it → 2.
    string expected = R"(successful
successful
successful
successful
unsuccessful
2
)";
 
    string actual = cc.parseInput(input);
    REQUIRE(actual == expected);
}
