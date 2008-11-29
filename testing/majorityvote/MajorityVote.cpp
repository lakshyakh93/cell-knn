#include "SortedList.cpp"
#include <map>

int main() {
	SortedList<int, char> mylist(5);
	mylist.insert(5, '2');
	mylist.print();
	mylist.insert(6, '1');
	mylist.print();
	mylist.insert(3, '3');
	mylist.print();
	mylist.insert(5, '1');
	mylist.print();
	mylist.insert(8, '3');
	mylist.print();
	mylist.insert(8, '2');
	mylist.print();
	mylist.insert(8, '2');
	mylist.print();
	mylist.insert(1, '2');
	mylist.print();
	mylist.insert(2, '1');
	mylist.print();


	std::map<char, int> majorityVote;
	
	for (SortedList<int, char>::Iterator it = mylist.begin(); it != mylist.end(); ++it) {
		majorityVote[it->value]++;
	}

	int maxVal = 0;
	char max;
	
	for (std::map<char, int>::iterator it = majorityVote.begin(); it != majorityVote.end(); ++it) {
		if (it->second > maxVal) {
			maxVal = it->second;
			max = it->first;
		}
	}
	
	std::cout << max << " (votes: " << maxVal << ")\n";
}