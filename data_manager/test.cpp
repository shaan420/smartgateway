#include <iostream>
#include <vector>

using namespace std;

template <typename T>
class A
{
	private:
		int a;

	public:
		A() 
		{
			a = -1;
		}

		A(int val) : a(val) 
		{
			cout << "in const A" << endl;
		}

		~A() 
		{
			cout << "in destr A" << endl;
		}

		int get_a()
		{
			cout << "in get_a\n";
			return a;
		}

		virtual void func(void) 
		{
			cout << "in A. func" << endl;
		}

		class Iterator : public iterator<forward_iterator_tag, T>
		{
			private:
				T elem_;

			public:
				Iterator() {}
				Iterator(T elem) : elem_(elem) {}
				~Iterator() {}

				virtual Iterator& operator=(const Iterator& other) {}
				virtual bool operator==(const Iterator& other) {}
				virtual Iterator& operator++() {}
				virtual T operator*() {}

				T GetElem()
				{
					return elem_;
				}

				void SetElem(T e)
				{
					elem_ = e;
				}
		};
};

class B : public A<vector<int>::iterator >
{
	private:
		int b;
		vector<int> m_vec;
	
	public:
		B(int val) : A(val), b(val)
		{
			cout << "in const B" << endl;
		}

		~B() 
		{
			cout << "in destr B" << endl;
		}

		void func(void)
		{
			cout << "in B. func() a=" << get_a() << endl;
		}
		
		class Iterator : public A<vector<int>::iterator >::Iterator
		{
			public:
				Iterator() : A<vector<int>::iterator >::Iterator() {}
				Iterator(vector<int>::iterator elem) : A<vector<int>::iterator >::Iterator(elem) {}
				~Iterator() {}

				Iterator& operator=(const Iterator& other) 
				{
					cout << "operator= called " << *other.GetElem() << endl;
					SetElem(other.GetElem());
					return *this;
				}

				bool operator==(Iterator other) 
				{
					cout << "operator== called" << endl;
					return (GetElem() == (other.GetElem()));
				}

				bool operator!=(Iterator other) 
				{
					cout << "operator!= called" << endl;
					return (GetElem() != (other.GetElem()));
				}

				Iterator& operator++() 
				{
					cout << "operator++ called" << endl;
					SetElem(++GetElem());
					return *this;
				}
				
				Iterator& operator++(int) 
				{
					cout << "operator++ postfix called" << endl;
					SetElem(++GetElem());
					return *this;
				}

				vector<int>::iterator operator*() 
				{
					cout << "operator* called" << endl;
					return GetElem();
				}
		};

		int InsertVal(int val)
		{
			m_vec.push_back(val);
			return 0;
		}

		Iterator begin()
		{
			return Iterator(m_vec.begin());
		}

		Iterator end()
		{
			return Iterator(m_vec.end());
		}
};

int main()
{
	int i;
	B tmp(1);
	tmp.func();

	for (i = 0; i < 10; i++)
	{
		tmp.InsertVal(i);
	}

	B::Iterator it;

	for (it = tmp.begin(); it != tmp.end(); it++)
	{
		cout << "Vals " << **it << endl;
	}

	return 0;
}
