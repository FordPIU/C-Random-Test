// Timed Functions
#include <windows.h>

// LUA Functions
#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

// Base
#include <iostream>
#include <string>

// Algo
#include <algorithm>

// Namespaces
using namespace luabridge;
using namespace std;

// Global Vars
int lastLineType = 0;

/////////////////////////////////////////////////////////////////////////////////////////

void print(string printMessage) { cout << "\n" + printMessage; lastLineType = 1; }
void print(int printNum) { cout << "\n" + to_string(printNum); lastLineType = 1; }

void printsl(string printMessage) { if (lastLineType == 1) { cout << "\n"; }; cout << "\033[F\r" << printMessage; lastLineType = 0; }
void printsl(int printNum) { if (lastLineType == 1) { cout << "\n"; }; cout << "\033[F\r" << to_string(printNum); lastLineType = 0; }

/////////////////////////////////////////////////////////////////////////////////////////

int getTickCount() { return static_cast<int>(GetTickCount64()); }
int getTickDifferent(int tick1, int tick2) { return tick2 - tick1; }

/////////////////////////////////////////////////////////////////////////////////////////

int timerStart = 0;
void startTimer() {
	if (timerStart != 0) {
		throw runtime_error("ATTEMPT TO START TIMER WHILE TIMER RUNNING\n");
	}

	timerStart = getTickCount();
}
int endTimer() {
	int returnTime = getTickDifferent(timerStart, getTickCount());

	timerStart = 0;

	return returnTime;
}

/////////////////////////////////////////////////////////////////////////////////////////

void countTo1000() { for (int i = 0; i != (1000+1); i++) { print(i); } }
void countTo10000() { for (int i = 0; i != (10000 + 1); i++) { print(i); } }

/////////////////////////////////////////////////////////////////////////////////////////

class SpeedTest_Math {
public:
    int CPP(int n) {
        int endTime;
        startTimer();

        for (int i = 0; i != (n * 1000 + 1); i++) {
            int result = 1;
            for (int j = 1; j <= i; j++) {
                result *= j;
            }
        }

        endTime = endTimer();
        return endTime;
    }

    int LUA(int n) {
        int endTime;
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);

        luabridge::getGlobalNamespace(L)
            .beginNamespace("cpp")
            .addFunction("startTimer", &startTimer)
            .addFunction("printsl", static_cast<void(*)(string)>(&printsl))
            .addFunction("endTimer", &endTimer)
            .addVariable("numOfThousands", &n)
            .endNamespace();

        // Create a string containing the lua code to be executed
        string luaCode = R"(
        cpp.startTimer()
        for i = 0, cpp.numOfThousands * 1000 do
            local result = 1
            for j = 1, i do
                result = result * j
            end
        end
        endTime = cpp.endTimer()
        )";

        // Load and execute the lua code
        int status = luaL_dostring(L, luaCode.c_str());
        if (status) {
            print("Error with lua code.\n" + string(lua_tostring(L, -1)));
        }

        endTime = (int)luabridge::getGlobal(L, "endTime");

        lua_close(L);
        return endTime;
    }
};

class SpeedTest_Algo {
public:
    int CPP(int n) {
        vector<int> arr(n * 100000);
        // randomize array
        for (int i = 0; i < n * 100000; i++) {
            arr[i] = rand() % 100000;
        }

        int endTime;
        startTimer();

        sort(arr.begin(), arr.end());

        endTime = endTimer();
        return endTime;
    }

    int LUA(int n) {
        int endTime;
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);

        luabridge::getGlobalNamespace(L)
            .beginNamespace("cpp")
            .addFunction("startTimer", &startTimer)
            .addFunction("printsl", static_cast<void(*)(string)>(&printsl))
            .addFunction("endTimer", &endTimer)
            .addVariable("numOfThousands", &n)
            .endNamespace();

        // Create a string containing the lua code to be executed
        string luaCode = R"(
        local arr = {}
        for i = 1, cpp.numOfThousands * 100000 do
            arr[i] = math.random(100000)
        end
        cpp.startTimer()
        table.sort(arr)
        endTime = cpp.endTimer()
        )";

        // Load and execute the lua code
        int status = luaL_dostring(L, luaCode.c_str());
        if (status) {
            print("Error with lua code.\n" + string(lua_tostring(L, -1)));
        }

        endTime = (int)luabridge::getGlobal(L, "endTime");

        lua_close(L);
        return endTime;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////

int NumOfMathSpeedTests = 100;
int NumOfAlgoSpeedTests = 15;
int main() {
    print("\n\n-----------------------------------------------");
    print("-----------------------------------------------");
    print("STARTING MATH SPEED TEST. 1000X");
    print("Mathematical Performance Test");
    print("-----------------------------------------------");
    SpeedTest_Math mathSpeedTest;

    int cppTimeSum = 0;
    int luaTimeSum = 0;
    int spdDiffSum = 0;
    for (int i = 1; i < (NumOfMathSpeedTests + 1); i++) {
        print("-----------------------------------------------");
       
        print("Starting Test (" + to_string(i * 1000) + ") for CPP");
        int cppTimeToComplete = mathSpeedTest.CPP(i);
        cppTimeSum += cppTimeToComplete;
        print("Test Finished :: CPP Test (" + to_string(i * 1000) + ") Finished in " + to_string(cppTimeToComplete)+"ms.");
        
        print("Starting Test (" + to_string(i * 1000) + ") for LUA");
        int luaTimeToComplete = mathSpeedTest.LUA(i);
        luaTimeSum += luaTimeToComplete;
        print("Test Finished :: LUA Test (" + to_string(i * 1000) + ") Finished in " + to_string(luaTimeToComplete)+"ms.");
        
        int speedDifference = (luaTimeToComplete | 1) / (cppTimeToComplete | 1);
        spdDiffSum += speedDifference;
        print("C++ is " + to_string(speedDifference) + "x faster than LUA");
        
        print("-----------------------------------------------\n");
    }

    print("\n\n-----------------------------------------------");
    print("-----------------------------------------------");
    print("STARTING ALGO SPEED TEST. 100000X");
    print("Algorithm Performance Test");
    print("-----------------------------------------------");
    SpeedTest_Algo algoSpeedTest;

    int cppTimeSum2 = 0;
    int luaTimeSum2 = 0;
    int spdDiffSum2 = 0;
    for (int i = 1; i < (NumOfAlgoSpeedTests + 1); i++) {
        print("-----------------------------------------------");
        
        print("Starting Test (" + to_string(i * 100000) + ") for CPP");
        int cppTimeToComplete = algoSpeedTest.CPP(i);
        cppTimeSum2 += cppTimeToComplete;
        print("Test Finished :: CPP Test (" + to_string(i * 100000) + ") Finished in " + to_string(cppTimeToComplete) + "ms.");
        
        print("Starting Test (" + to_string(i * 100000) + ") for LUA");
        int luaTimeToComplete = algoSpeedTest.LUA(i);
        luaTimeSum2 += luaTimeToComplete;
        print("Test Finished :: LUA Test (" + to_string(i * 100000) + ") Finished in " + to_string(luaTimeToComplete) + "ms.");
        
        int speedDifference = (luaTimeToComplete | 1) / (cppTimeToComplete | 1);
        spdDiffSum2 += speedDifference;
        print("C++ is " + to_string(speedDifference) + "x faster than LUA");
        
        print("-----------------------------------------------\n");
    }

    print("\n\n-----------------------------------------------");
    
    print("\033[1;34m----- Summary for Math Speed Test -----\033[0m");
    print("CPP average time to complete is \033[1;32m" + to_string(cppTimeSum / NumOfMathSpeedTests) + "\033[0mms.");
    print("LUA average time to complete is \033[1;31m" + to_string(luaTimeSum / NumOfMathSpeedTests) + "\033[0mms.");
    print("CPP was on average \033[1;32m" + to_string((luaTimeSum - cppTimeSum) / NumOfMathSpeedTests) + "\033[0mms faster than LUA.");
    print("CPP was on average \033[1;32m" + to_string(spdDiffSum / NumOfMathSpeedTests) + "\033[0mx faster than LUA.");

    print("\n\033[1;34m----- Summary for Algo Speed Test -----\033[0m");
    print("CPP average time to complete is \033[1;32m" + to_string(cppTimeSum2 / NumOfAlgoSpeedTests) + "\033[0mms.");
    print("LUA average time to complete is \033[1;31m" + to_string(luaTimeSum2 / NumOfAlgoSpeedTests) + "\033[0mms.");
    print("CPP was on average \033[1;32m" + to_string((luaTimeSum2 - cppTimeSum2) / NumOfAlgoSpeedTests) + "\033[0mms faster than LUA.");
    print("CPP was on average \033[1;32m" + to_string(spdDiffSum2 / NumOfAlgoSpeedTests) + "\033[0mx faster than LUA.");

    print("\n\n");
}