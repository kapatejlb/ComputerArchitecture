#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>

using namespace std;

template<typename T>
class FixSizeQueueCondA
{
private:
	vector<T> q;

	atomic<int> sum, curSizePH = 0, curSizeP = 0;
	mutex lock, lock1, locktest1;
	atomic<T> ind1 = 0, ind2 = 0;

	int p_num, t_num, c_num, size;

public:
	FixSizeQueueCondA(int size, int t_num, int p_num, int c_num) : t_num(t_num), p_num(p_num), c_num(c_num), size(size)
	{
		q.resize(size);
	}

	void pop()
	{
		while (true)
		{
			if (curSizeP.load() >= c_num * t_num)
				return;
			T i = ind2.load(memory_order_relaxed);
			while (!ind2.compare_exchange_weak(i,(ind2 + 1) % q.size(), memory_order_release, memory_order_relaxed));
			{
				unique_lock<mutex> un(locktest1);
				if (curSizeP.load() >= c_num * t_num)
					return;
				if (!q[(i % q.size())])
					continue;
				sum += q[(i % q.size())];
				q[(i % q.size())] = 0;
			}
			curSizeP++;
		}
	}

	void push()
	{
		while (true)
		{
			if (curSizePH.load() >= p_num * t_num)
				return;
			T i = ind1.load(memory_order_relaxed);
			while (!ind1.compare_exchange_weak(i, (ind1 + 1) % q.size(), memory_order_release, memory_order_relaxed));
			{
				unique_lock<mutex> un(locktest1);
				if (curSizePH.load() >= p_num * t_num)
					return;
				if (q[(i % q.size())])
					continue;
				q[(i % q.size())]++;
			}
			curSizePH++;
		}
	}

	void print() 
	{
		int ans = 0;
		for (int i = 0; i < q.size(); ++i)
			cout << q[i] << " ", ans += q[i];
		cout << endl;
		cout << ans << endl;
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

	int returnSum() 
	{
		return sum;
	}
};

int main()
{
	FixSizeQueueCondA<int> q(16, 1024 * 1024, 4, 4);
	q.startThreads();
	cout << q.returnSum() << endl;

	return 0;
}