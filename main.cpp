#include "disk.h"
#include <iostream>
using namespace std;

int main() {
    Disk disk("disk");
    string s = "One day, a monkey rides his bike near the river. This time he sees a lion under a tree. The lion runs at him. He is afraid and falls into the river. He can’t swim. He shouts. The rabbit hears him. He jumps into the river. The rabbit swims to the monkey, but he can’t help him. Luckily, an elephant comes along. He is very strong.";
    disk.createFile("root\\file", s.size(), s, NORMAL);
    disk.outPut();
    cout << disk.readFile("root\\file", NORMAL) << endl;
    return 0;
}