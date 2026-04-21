// ─────────────────────────────────────────────────────────────────────────────
// tests.cpp  –  Unit tests for CampusCompass
//
// Compile (with real Catch2 installed):
//   g++ -std=c++17 -DCATCH_TESTING -I<catch2-include-dir> \
//       tests.cpp Graph.cpp -o tests && ./tests
//
// The CATCH_TESTING define suppresses the duplicate main() in main.cpp.
// ─────────────────────────────────────────────────────────────────────────────
 
#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"
#include <iostream>
#include "../src/graph.h"
#include "../src/student.h"
#include "../src/campusCompass.h"
using namespace std;
 
// ─── Helper that builds a loaded CampusCompass ────────────────────────────────────
static CampusCompass makeCC() {
    CampusCompass cc;
    cc.loadEdges("../data/edges.csv");
    cc.loadClasses("../data/classes.csv");
    return cc;
}

//PART 1 INCORRECT/INVALID COMMANDS
 
TEST_CASE("Invalid insert bc name contains digits", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "E11a" 10252004 1 1 COP3530)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - student ID too short (7 digits)", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "Amber" 1234567 1 1 COP3530)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert bc student ID too long (9 digits)", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "Sydney" 123456789 1 1 COP3530)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert bc class count = 0 (below min)", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "Corena" 12345678 1 0)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert - class count N=7 (above max)", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(
        R"(insert "Amber" 12345678 1 7 COP3530 COP3503 COP3504 CDA3101 MAC2311 MAC2312 MAC2313)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Invalid insert bc malformed class code (lowercase)", "[invalid]") {
    CampusCompass cc = makeCC();
    //class code must be 3 uppercase letters and 4 digs
    string result = cc.processLine(R"(insert "Austin" 12345678 1 1 cop3530)");
    REQUIRE(result == "unsuccessful");
}

TEST_CASE("Invalid command spelling is rejected", "[invalid]") {
    CampusCompass cc = makeCC();
    // Misspelled command
    string result = cc.processLine("insrt \"Amber\" 12345678 1 1 COP3530");
    REQUIRE(result == "unsuccessful");
}

TEST_CASE("Invalid insert bc class code that does not exist in classes.csv", "[invalid]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine(R"(insert "Quinn" 12345678 1 1 BBC6969)");
    REQUIRE(result == "unsuccessful");
}
 
//PART 2 EDGE CASES
 
TEST_CASE("Edge case to remove student that dne", "[edge]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine("remove 99999999");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Edge case of dupe student ID rejected on second insert", "[edge]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Amber" 12345678 1 1 COP3530)");
    //Same ID a second time fails
    string result = cc.processLine(R"(insert "Jake" 12345678 1 1 MAC2311)");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Edge case so dropClass on last class autoremoves student", "[edge]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Jake Schnada" 01010101 1 1 COP3530)");
    REQUIRE(cc.getStudents().count("01010101") == 1);
    //Drop only class so student should be erased automatically
    cc.processLine("dropClass 01010101 COP3530");
    REQUIRE(cc.getStudents().count("01010101") == 0);
}
 
TEST_CASE("Edge case so removeClass with no students enrolled returns unsuccessful", "[edge]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine("removeClass COP3530");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("Edge case but replaceClass fails when student already has target class", "[edge]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Sydney" 12345678 1 2 COP3530 MAC2311)");
    //replace COP3530 with MAC2311 but she already has
    string result = cc.processLine("replaceClass 12345678 COP3530 MAC2311");
    REQUIRE(result == "unsuccessful");
}
 
//PART 3 dropClass/removeClass/remove/replaceClass
 
TEST_CASE("dropClass - successfully drops one of multiple classes", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Amber" 12345678 1 2 COP3530 MAC2311)");
    string result = cc.processLine("dropClass 12345678 COP3530");
    REQUIRE(result == "successful");
    //student still exists wit one class
    REQUIRE(cc.getStudents().count("12345678") == 1);
    REQUIRE(cc.getStudents().at("12345678").classes.count("COP3530") == 0);
    REQUIRE(cc.getStudents().at("12345678").classes.count("MAC2311") == 1);
}
 
TEST_CASE("dropClass fails when student doesnt have the class", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Nicole" 12345678 1 1 MAC2311)");
    string result = cc.processLine("dropClass 12345678 COP3530");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("removeClass drops class from all enrolled students and returns count", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Amber" 10000001 1 1 COP3530)");
    cc.processLine(R"(insert "Nicole"   10000002 1 2 COP3530 MAC2311)");
    cc.processLine(R"(insert "Corena" 10000003 1 1 MAC2311)");
    //Only Amber and Nicole are in COP3530
    string result = cc.processLine("removeClass COP3530");
    REQUIRE(result == "2");
    //Amber had only COP3530 → auto-removed
    REQUIRE(cc.getStudents().count("10000001") == 0);
    //Nicole still exists with MAC2311
    REQUIRE(cc.getStudents().count("10000002") == 1);
    REQUIRE(cc.getStudents().at("10000002").classes.count("COP3530") == 0);
    //Corena unaffected
    REQUIRE(cc.getStudents().count("10000003") == 1);
}
 
TEST_CASE("remove successfully removes an existing student", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Sydney" 12345678 1 1 COP3530)");
    string result = cc.processLine("remove 12345678");
    REQUIRE(result == "successful");
    REQUIRE(cc.getStudents().count("12345678") == 0);
}
 
TEST_CASE("remove fails for non-existent student", "[commands]") {
    CampusCompass cc = makeCC();
    string result = cc.processLine("remove 00000000");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("replaceClass successfully swaps one class for another", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Meghan" 12345678 1 1 COP3530)");
    string result = cc.processLine("replaceClass 12345678 COP3530 MAC2311");
    REQUIRE(result == "successful");
    REQUIRE(cc.getStudents().at("12345678").classes.count("COP3530") == 0);
    REQUIRE(cc.getStudents().at("12345678").classes.count("MAC2311") == 1);
}
 
TEST_CASE("replaceClass fails when old class not in schedule", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 MAC2311)");
    string result = cc.processLine("replaceClass 12345678 COP3530 MAC2312");
    REQUIRE(result == "unsuccessful");
}
 
TEST_CASE("replaceClass fails when new class code DNE", "[commands]") {
    CampusCompass cc = makeCC();
    cc.processLine(R"(insert "Alice" 12345678 1 1 COP3530)");
    string result = cc.processLine("replaceClass 12345678 COP3530 XYZ9999");
    REQUIRE(result == "unsuccessful");
}
 
//PART 4 PRINTSHORTESTEDGES

TEST_CASE("printShortestEdges reachable then unreachable after closing bridges", "[shortest]") {
    CampusCompass cc = makeCC();
    //Student lives at node 1 enrolled in EEL3701
    //Close both and node 49 becomes a disconnected island
    cc.processLine(R"(insert "Jake Schnada" 69696969 1 1 EEL3701)");
 
    //Before closing
    string before = cc.processLine("printShortestEdges 99991111");
    // The time should be a positive integer bc path exists
    REQUIRE(before.find("EEL3701: -1") == string::npos);
    REQUIRE(before.find("EEL3701:") != string::npos);
 
    //Close both bridges
    cc.processLine("toggleEdgesClosure 2 7 49 49 56");
 
    //After closing
    string after = cc.processLine("printShortestEdges 99991111");
    REQUIRE(after.find("EEL3701: -1") != string::npos);
 
    //Re-open one bridge
    cc.processLine("toggleEdgesClosure 1 7 49");
    string restored = cc.processLine("printShortestEdges 99991111");
    REQUIRE(restored.find("EEL3701: -1") == string::npos);
}
 

//5. ENDTOEND PARSE MATCH

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
    //dropClass on Student A's only class autoremoves the student
    //remove 10000001 and now gone so remove returns unsuccessful
    //removeClass COP3502 only Student B still had it and 2 was pre-drop state
    //After dropClass and remove nly Student B (10000002) has COP3502
    //removeClass COP3502 but Student B has it so count = 1 but template says "2"
    //dropClass 10000001 COP3502 succeeds (auto-removes student A).
    //remove 10000001 its already gone so unsuccessful
    //removeClass COP3502 Student B (10000002) and Student C (10000003) still have it so 2
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
