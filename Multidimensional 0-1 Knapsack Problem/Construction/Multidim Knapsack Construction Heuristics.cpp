// Multidim Knapsack Construction Heuristics.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>

//todo: remove using namespace for increasing speed
using namespace std;

int chooseVariable(int n, double* rate, short* assigned)
{
	double max = -DBL_MAX;
	int imax = -1;
	for (int i = 0; i < n; i++)
	{
		if (!assigned[i] && rate[i] > max)
		{
			max = rate[i];
			imax = i;
		}
	}
	return imax;
}

void calculateRateAllTest(int n, int m, int* p, int** r, int* b, int* used, short* assigned, double* rate)
{
	for (int i = 0; i < n; i++)
	{
		double v = 1;
		bool canTake = true;
		for (int j = 0; j < m; j++)
		{
		
			v += r[j][i];
			
		}
		rate[i] = p[i] / v;
	}
}

void calculateRateAll(int n, int m, int* p, int** r, int* b, int* used, short* assigned, double* rate)
{
	for (int i = 0; i < n; i++)
	{
		double v = 0;
		bool canTake = true;
		for (int j = 0; j < m; j++)
		{
			if (b[j] - used[j] - r[j][i] >= 0)
			{
				v += r[j][i] * 1.0 / (double(b[j]) - double(used[j]));
			}
			else
			{
				canTake = false;
			}
		}
		rate[i] = canTake ? (p[i] / v) : 0;
	}
}

void calculateRateAllWeighted(int n, int m, int* p, int** r, int* b, int* used, short* assigned, double* rate, double* weight)
{
	for (int i = 0; i < n; i++)
	{
		double v = 0;
		bool canTake = true;
		for (int j = 0; j < m; j++)
		{
			if (b[j] - used[j] - r[j][i] >= 0)
			{
				v += r[j][i] * weight[j] / (double(b[j]) - double(used[j]));
			}
			else
			{
				canTake = false;
			}
		}
		rate[i] = canTake ? (p[i] / v) : 0;
	}
}

void calculateWeight(int m, double* weight, int* demand, int* b)
{
	for (int i = 0; i < m; i++)
	{
		if (demand[i] <= b[i])
		{
			weight[i] = 0;
		}
		else
		{
			weight[i] = demand[i] * 1.0 / b[i];
		}
	}
}

void calculateRateTopK(int n, int m, int* p, int** r, int* b, int* used, short* assigned, double* rate, int k)
{
	double* values = new double[m];
	for (int i = 0; i < n; i++)
	{
		double v = 0;
		for (int j = 0; j < m; j++)
		{
			values[j] = r[j][i] * 1.0 / (double(b[j]) - double(used[j]));
		}
		sort(values, values + m);
		for (int j = 0; j < k; j++)
		{
			v += values[j];
		}

		rate[i] = p[i] / v;
	}
}

void updatePossible(int n, int m, int** r, short* possible, short* assigned, int* used, int* b)
{
	for (int i = 0; i < n; i++)
	{
		if (possible[i] && !assigned[i])
		{
			short p = 1;
			for (int j = 0; j < m; j++)
			{
				if (b[j] < used[j] + r[j][i])
				{
					p = 0;
					break;
				}
			}
			possible[i] = p;
		}
		else
		{
			possible[i] = 0;
		}
	}
}

void calculateRateArticle(int n, int m, int* p, int** r, int* b, int* used, int* demand, short* assigned, double* rate, short* possible)
{
	updatePossible(n, m, r, possible, assigned, used, b);
	for (int j = 0; j < m; j++)
	{
		demand[j] = 0;
		for (int i = 0; i < n; i++)
		{
			demand[j] += possible[i] * r[j][i];
		}
		cout << demand[j] << '(' << b[j] - used[j] << ")\t";
	}
	cout << '\n';
	for (int i = 0; i < n; i++)
	{
		double v = 0;
		for (int j = 0; j < m; j++)
		{
			int consump = used[j] + r[j][i];
			int remaining = b[j] - used[j] - r[j][i];
			int dem = demand[j] - r[j][i];
			v += consump * 1.0 * dem / double(remaining);

			//v += r[j][i] * 1.0 * demand[j] - 1.0 * b[j];
		}
		rate[i] = p[i] / v;
	}
}

int main(int argc, char* argv[])
{
	//check input
	if (argc < 2)
	{
		cout << "Error. No file specified.\n";
		return 1;
	}

	ifstream fin;
	fin.open(argv[1]);

	if (!fin)
	{
		cout << "Unable to open " << argv[1] << '\n';
		return 1;
	}

	//start init
	int n, m, q, opt;
	int* p, * b, * x, * used;
	int** r;

	fin >> n >> m >> q >> opt;


	p = new int[n];
	for (int i = 0; i < n; i++)
	{
		fin >> p[i];
	}

	r = new int* [m];
	for (int i = 0; i < m; i++)
	{
		r[i] = new int[n];
		for (int j = 0; j < n; j++)
		{
			fin >> r[i][j];
		}
	}

	b = new int[m];
	for (int i = 0; i < m; i++)
	{
		fin >> b[i];
	}

	//end init
	auto start = chrono::high_resolution_clock::now();

	short* assigned = new short[n];
	for (int i = 0; i < n; i++)
	{
		assigned[i] = 0;
	}
	x = new int[n];
	used = new int[m];
	for (int i = 0; i < m; i++)
	{
		used[i] = 0;
	}

	//create rate
	double* rate = new double[n];
	/*
	for (int i = 0; i < n; i++)
	{
		double v = 1;
		for (int j = 0; j < m; j++)
		{
			v += r[j][i];
		}
		rate[i] = p[i] / v;
	}*/

	//start algo

	int nassigned = 0;

	int* demand = new int[m];
	for (int i = 0; i < m; i++)
	{
		demand[i] = 0;
		for (int j = 0; j < n; j++)
		{
			demand[i] += r[i][j];
		}
	}

	short* possible = new short[n];
	for (int i = 0; i < n; i++)
	{
		possible[i] = 1;
	}

	double* weight = new double[m];
	calculateWeight(m, weight, demand, b);

	while (nassigned < n)
	{
		calculateRateAll(n, m, p, r, b, used, assigned, rate);
		//calculateRateAllTest(n, m, p, r, b, used, assigned, rate);
		//calculateRateTopK(n, m, p, r, b, used, assigned, rate, 5);
		//calculateRateArticle(n, m, p, r, b, used, demand, assigned, rate, possible);
		//calculateRateAllWeighted(n, m, p, r, b, used, assigned, rate, weight);
		int j = chooseVariable(n, rate, assigned);
		bool canTake = true;
		for (int i = 0; i < m; i++)
		{
			if (used[i] + r[i][j] > b[i])
			{
				canTake = false;
				break;
			}
		}
		assigned[j] = 1;
		nassigned++;
		if (canTake)
		{
			x[j] = 1;
			for (int i = 0; i < m; i++)
			{
				used[i] += r[i][j];
				//demand[i] -= r[i][j];
			}
		}
		else
		{
			x[j] = 0;
		}
	}

	auto stop = chrono::high_resolution_clock::now();

	int s = 0;
	for (int i = 0; i < n; i++)
	{
		//cout << x[i] << ' ';
		s += x[i] * p[i];
	}

	auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    std::cout << s << "\t" << duration.count() * 1.0 / 1000 << " milliseconds\n";
}
