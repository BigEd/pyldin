#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
    int * a = new int[10];
    int * b = new int(89);
    cout << "*b=" << *b << endl;

    int *c = new (nothrow) int [1000000000];
    if (!c)
	cout << "Failed to allocate memory" << endl;

    a[5] = 9;
    cout << "a[5]=" << a[5] << endl;
    delete [] a;
    delete b;
    delete [] c;
    return 0;
}
