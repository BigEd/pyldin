// ex1.cpp : Defines the entry point for the console application.

#include <iostream>
#include <string>

using namespace std;

class phonerec {
  public:

    string name;
    string phonenumber;

	
	// default constructor  
    phonerec(string myname ="Me",string myphone="555-1000")
	{
		name=myname;
		phonenumber=myphone;
		cout << "Default Constructor phonerec" << endl;
	}

	// Copy constructor
	phonerec(phonerec & rec) { 
		name=rec.name;
		phonenumber=rec.phonenumber;
		cout << "Copy Constructor phonerec" << endl;
	}

	// assignment operator
	phonerec & operator=(const phonerec & rhs) throw() {
		cout << "Assignment Operator phonerec" << endl;
		name= rhs.name;
		phonenumber = rhs.phonenumber;
		return *this;
	}

};


class phonebillrec : public phonerec {
public:
	double bill;

	// default constructor
	phonebillrec(double amount=0.0) {
		cout << "Default Constructor phonebillrec" << endl;
		bill=amount;
	}

	// Copy Constructor
	phonebillrec(phonebillrec & rec) : phonerec(rec)  {
		cout << "Copy Constructor phonebillrec" << endl;
		bill = rec.bill;
	}

	// assignment operator
	phonebillrec & operator=(const phonebillrec & rhs) throw() {
		phonerec::operator=(rhs);  // Call base class assignment operator
		cout << "Assignment Operator phonebillrec" << endl;
		bill= rhs.bill;
		return *this;
	}

};


int main(int argc, char* argv[])
{

	phonebillrec p; // calls a default parameter "Me"
	cout << "----" << endl;
	phonebillrec p2(23.45); 
	cout << "----" << endl;
	p2.name="Fred";

	phonebillrec p3(p2);
	cout << "----" << endl;
	phonebillrec p4;
	cout << "----" << endl;
	p4=p2;
	cout << "----" << endl;
	cout << "Name = " << p3.name << endl;
	return 0;
}

