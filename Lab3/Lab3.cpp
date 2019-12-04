#include <stdio.h>;
#include <omp.h>;
#include <cstring>
#include <conio.h>
#include <iostream>
#include <ctime>

using namespace std;

//pragma func
double* conv(int* x, int N, int* h, int M)
{
	double* result = new double[N + M - 1];
	memset(result, 0, sizeof(double) * (N + M - 1));

	int chunk = 10000;

//OMG PRAGMA SPPED x2
#pragma omp parallel for default(shared) private(i) schedule(auto ) reduction(+:result)
	for (int i = 0; i < N + M - 1; i++)
	{
		for (int j = 0; j < M; j++)
		{
			if (i - j >= 0 && i - j < N)
				result[i] = result[i] + x[i - j] * h[j];
		}
	}

	return result;
}

//usual function
double* conv2(int* x, int N, int* h, int M)
{
	double* result = new double[N + M - 1];
	memset(result, 0, sizeof(double) * (N + M - 1));

	for (int i = 0; i < N + M - 1; i++)
	{
		for (int j = 0; j < M; j++)
		{
			if (i - j >= 0 && i - j < N)
				result[i] += x[i - j] * h[j];
		}
	}

	return result;
}


int main()
{
	int x[50000];
	int h[50000];
	srand(time(NULL));//time

	for (int i = 0; i < 5000; ++i)
	{
		x[i] = rand() % 100 + 1;
		//cout << x[i] << endl;
	}

	for (int i = 0; i < 5000; ++i)
	{
		h[i] = rand() % 100 + 1;
		//cout << h[i] << endl;
	}

	srand(time(0));
	conv(x, 50000, h, 50000);
	cout << "runtime = " << clock() / 1000.0 << endl;

	conv2(x, 50000, h, 50000);
	cout << "runtime = " << clock() / 1000.0 << endl;


	system("PAUSE");
	return 0;

}
