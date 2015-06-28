#include <iostream>

using namespace std;

template <typename T>
class BaseT
{
	T a;

	public:
		virtual T A() = 0;

		class NestedT
		{
			public:
				NestedT(T elem)
				{
					n = elem;
				}

				T n;

				virtual T N();
		};
};

class Derived : public BaseT<int>
{
	public:

	int A()
	{
		return 0;
	}

	typedef NestedT Nested;	
};

template <>
int Derived::Nested::N()
{
	return 10;
}

int main()
{
	Derived d1;
	Derived::Nested d2(5);

	cout << d1.A() << endl;
	cout << d2.N() << endl;
	return 0;
}
