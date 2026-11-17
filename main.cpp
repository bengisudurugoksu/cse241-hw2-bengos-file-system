#include "bengOS.h"
#include "bengOS.cpp"
int main(){
    time_t currentTime = time(nullptr);
    Directory root("/home", currentTime);
    load("filesystem.txt", root);
    Directory* currentDir = &root;
    Shell(currentDir);
    save("filesystem.txt", root);
    return 0;
}