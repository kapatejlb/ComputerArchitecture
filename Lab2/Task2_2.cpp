#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>

using namespace std;

template<typename T>
class FixSizeQueueCondM
{
private:
	queue<T> q;
	mutex lock1, lock2, lock3;

	int p_num, t_num, c_num, i = 0, j = 0;
	long long sum = 0;

public:
	FixSizeQueueCondM(int t_num, int p_num, int c_num) : t_num(t_num), p_num(p_num), c_num(c_num) { }

	void pop()
	{
		lock2.lock();
		while (i >= j * t_num)
		{
			if (j >= p_num && q.empty())
			{
				lock2.unlock();
				return;
			}
		}
		for (int k = 0; i < t_num; ++k)
		{
			lock3.lock();
			sum += q.front();
			q.pop();
			lock3.unlock();
		}
		i++;
		lock2.unlock();
	}

	void push()
	{
		lock1.lock();
		for (int k = 0; i < t_num; ++k) 
		{
			lock3.lock();
			q.push(1);
			lock3.unlock();
		}
		j++;
		lock1.unlock();
	}

	void startThreads()
	{
		vector<thread> threadsPH, threadsP;
		auto ph = [&]()
		{ 
			push();
		};
		auto p = [&]() 
		{
			pop();
		};

		for (int i = 0; i < p_num; ++i)
		{
			thread thr(ph);
			threadsPH.push_back(move(thr));
		}

		for (int i = 0; i < c_num; ++i)
		{
			thread thr(p);
			threadsP.push_back(move(thr));
		}

		for (int i = 0; i < p_num; ++i)
			if (threadsPH[i].joinable())
				threadsPH[i].join();
		for (int i = 0; i < c_num; ++i)
			if (threadsP[i].joinable())
				threadsP[i].join();
	}

	long long returnSum() 
	{
		return sum;
	}
};

int main() 
{
	FixSizeQueueCondM<uint8_t> q(4 * 1024 * 1024, 4, 4);//1.20
	q.startThreads();
	cout << q.returnSum() << endl;

	return 0;
}