#ifndef UTIL_H
#define UTIL_H

namespace btremote {
    class Direction {
    public:
        static const Direction Front, Left, Back, Right, LeftTurn, RightTurn;
        const char commandChar;
    private:
        explicit constexpr Direction(char c) : commandChar(c) { }
        Direction(const Direction&) = delete;
    };

    constexpr const char CarStop[] = "Q;";
};


#endif // UTIL_H
