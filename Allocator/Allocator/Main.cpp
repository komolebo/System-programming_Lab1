#include "Functions.h"
#include <iostream>

using namespace std;

void menu();

int main()
{
	previous_load();

	while (true)
	{
		mem_dump();
		menu();
	}

	return 0;
}

void menu()
{
	size_t n, size, addr;
	
	cout << endl  << "Action: 1 - alloc, 2 - realloc, 3 - free, 4 - defragment" << endl;

	cin >> n;

	switch (n)
	{
	case 1:
		cout << "Memory size: " << endl;
		cin >> size;
		mem_alloc(size);
		break;
	case 2:
		cout << "Memory size: " << endl;
		cin >> size;
		cout << "Block address: " << endl;
		cin >> addr;
		mem_realloc((int *)addr, size);
		break;
	case 3:
		cout << "Block address: " << endl;
		cin >> addr;
		mem_free((int *)addr);
		break;
	case 4:
		mem_defrag();
		break;
	default:
		break;
	}
}