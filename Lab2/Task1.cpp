#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>

using namespace std;

template<typename T>
class ThreadArray
{
private:
	vector<vector<T>> array;
	vector<T> arr_atomic;
	vector<thread> threads;
	unordered_map<thread::id, double> tm;
	
	mutex mutex_lock;
	atomic<int> i_atomic = 0;

	int i, cnt;

	void funM(vector<vector<T>>& v)
	{
		while(true)
		{
			mutex_lock.lock();
			i++;
			if (i >= 1024 * 1024)
			{
				mutex_lock.unlock();
				break;
			}
			arr_atomic[i]++;
			mutex_lock.unlock();
		}
	}

	void funA(vector<T>& v)
	{
		while (true)
		{
			auto tmp = i_atomic.fetch_add(1, memory_order_relaxed);
			if (tmp >= 1024 * 1024)
				return;
			v[tmp]++;
		}
	}

public:
	ThreadArray(vector<vector<T>>&& v, int n) : array(move(v)), i(-1), i_atomic(0), cnt(n) { }
	ThreadArray(const vector<vector<T>>& v, int n) : array(v), i(-1), i_atomic(0), cnt(n) { }
	ThreadArray(vector<T>&& v, int n) : arr_atomic(move(v)), i(-1), i_atomic(0), cnt(n) { }
	ThreadArray(const vector<T>& v, int n) : arr_atomic(v), i(-1), i_atomic(0), cnt(n) { }

	void startThreadsM()
	{
		auto t = [&](vector<vector<T>>& v) 
		{ 
			funM(v);
		};
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < cnt; ++i)
		{
			thread thr(t, ref(array));
			threads.push_back(move(thr));
		}

		for (int j = 0; j < cnt; ++j)
			if (threads[j].joinable())
				threads[j].join();
		auto stop = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
		cout << duration.count() << endl;
	}

	void startThreadsALinear()
	{
		auto t = [&](vector<T>& v)
		{ 
			funA(v); 
		};
		auto start = chrono::high_resolution_clock::now();
		for (int i = 0; i < cnt; ++i)
		{
			thread thr(t, ref(arr_atomic));
			threads.push_back(move(thr));
		}

		for (int j = 0; j < cnt; ++j)
			if (threads[j].joinable())
				threads[j].join();
		auto stop = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
		cout << duration.count() << endl;
	}

	void printVector()
	{
		for (int i = 0; i < array.size(); ++i) 
		{
			for (int j = 0; i < array[0].size(); ++j)
				cout << array[i][j];
			cout << endl;
		}
	}

	void printLinear()
	{
		for (int i = 0; i < arr_atomic.size(); ++i)
			cout << arr_atomic[i] << " ";
		cout << endl;
	}

	void printTime()
	{
		for (auto i = tm.begin(); i != tm.end(); ++i)
			cout << i->first << " " << i->second << endl;
	}
};


int main()
{
	int n = 1024, m = 1024, k;
	cin >> k;
	vector<int> tm(n * n);

	ThreadArray<int> vc(tm, k);
	vc.startThreadsM();
	ThreadArray<int> c(tm, k);
	c.startThreadsALinear();

	return 0;
}