#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>

using namespace std;

template<typename T>
class DynamicQueue
{
private:
	vector<int> q;

	mutex ph, p, lock_ph, lock_p;
	condition_variable conditionPH, conditionP;
	atomic<int> curSizePH = 0, curSizeP = 0;
	atomic<int> current = 0;

	int sum = 0;
	bool isCompleted;
	int p_num, t_num, c_num, size, ind = 0, ind1 = 0;

public:
	DynamicQueue(int size, int t_num, int p_num, int c_num) : t_num(t_num), p_num(p_num), c_num(c_num), size(size)
	{
		q.resize(size);
		for (int i = 0; i < q.size(); ++i)
			q[i] = 0;
		isCompleted = false;
	}

	void pop()
	{
		while (true)
		{
			unique_lock<mutex> lock(lock_p);
			if (curSizeP.load() >= c_num * t_num)
			{
				conditionPH.notify_all();
				return;
			}
			while (current.load() <= 0)
			{
				if (curSizeP.load() >= c_num * t_num)
				{
					conditionPH.notify_all();
					return;
				}
				conditionP.wait(lock);
			}
			if (curSizeP.load() >= c_num * t_num)
			{
				conditionPH.notify_all();
				return;
			}
			if (!q[ind % size])
				this_thread::sleep_for(chrono::milliseconds(1));
			sum += q[ind % size];
			q[ind % size] = 0;
			current--;
			conditionPH.notify_all();
			ind++;
			ind %= size;
			curSizeP++;
		}
	}

	void push()
	{
		while (true)
		{
			unique_lock<mutex> lock(lock_ph);
			if (curSizePH.load() >= p_num * t_num) 
			{
				conditionP.notify_all();
				conditionPH.notify_all();
				return;
			}
			while (current.load() >= 4) 
			{
				if (curSizePH.load() >= p_num * t_num) 
				{
					conditionPH.notify_all();
					return;
				}
				conditionPH.wait(lock);
			}
			if (curSizePH.load() >= p_num * t_num)
			{
				conditionPH.notify_all();
				return;
			}
			if (q[ind1 % size])
				this_thread::sleep_for(chrono::milliseconds(1));
			q[ind1 % size] = 1;
			current++;
			conditionP.notify_all();
			ind1++;
			ind1 %= size;
			curSizePH++;
		}
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

	DynamicQueue<int> q(4, 1024 * 100, 4, 4); //1.10
	q.startThreads();
	cout << q.returnSum() << endl;

	return 0;
}