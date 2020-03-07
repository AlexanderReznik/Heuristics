// Multidim Knapsack Construction Heuristics.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <time.h>
#include <chrono>

void copyArray(int size, int* source, int* destination)
{
	for (int i = 0; i < size; i++)
	{
		destination[i] = source[i];
	}
}

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

void calculateRateEasy(int n, int m, int* p, int** r, int* b, int* used, short* assigned, double* rate)
{
	for (int i = 0; i < n; i++)
	{
		double v = 0;
		bool canTake = true;
		for (int j = 0; j < m; j++)
		{
			if (b[j] - used[j] - r[j][i] >= 0)
			{
				v += r[j][i] * 1.0 / (double(b[j]));
			}
			else
			{
				canTake = false;
			}
		}
		rate[i] = canTake ? (p[i] / v) : 0;
	}
}

void calculateRate(int n, int m, int* p, int** r, int* b, int* used, short* assigned, double* rate)
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

void calculateRateNorm(int n, int m, int* p, int** r, int* b, int* used, short* assigned, double* rate)
{
	for (int i = 0; i < n; i++)
	{
		double v = 0;
		bool canTake = true;
		for (int j = 0; j < m; j++)
		{
			if (b[j] - used[j] - r[j][i] >= 0)
			{
				double size = r[j][i] * 1.0 / (double(b[j]) - double(used[j]));
				v += pow(size, 2);
			}
			else
			{
				canTake = false;
			}
		}
		rate[i] = canTake ? (p[i] / sqrt(v)) : 0;
	}
}

std::ifstream openFile(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Error. No file specified.\n";
		exit(1);
	}

	std::ifstream fin;
	fin.open(argv[1]);

	if (!fin)
	{
		std::cout << "Unable to open " << argv[1] << '\n';
		exit(1);
	}

	return fin;
}

bool checkPossibility(int j, int m, int* used, int** r, int* b)
{
	bool canTake = true;
	for (int i = 0; i < m; i++)
	{
		if (used[i] + r[i][j] > b[i])
		{
			canTake = false;
			break;
		}
	}

	return canTake;
}

void assignValue(int j, bool canTake, int m, int& nassigned, short* assigned, int* x, int* used, int** r)
{
	assigned[j] = 1;
	nassigned++;
	if (canTake)
	{
		x[j] = 1;
		for (int i = 0; i < m; i++)
		{
			used[i] += r[i][j];
		}
	}
	else
	{
		x[j] = 0;
	}
}

void initSolution(int n, int m, short* assigned, int* used)
{
	for (int i = 0; i < n; i++)
	{
		assigned[i] = 0;
	}

	for (int i = 0; i < m; i++)
	{
		used[i] = 0;
	}
}

void solveConstruction(int n, int m, int* x, int* b, int* p, int** r, void calculateRate(int, int, int*, int**, int*, int*, short*, double*))
{
	short* assigned = new short[n]; //1 if we already assigned value for a variable
	int* used = new int[m]; //amount of used space in every dimension
	double* rate = new double[n];//rate used to choose variable
	int nassigned = 0; //number of assigned variables
	initSolution(n, m, assigned, used);

	//start algo
	while (nassigned < n)
	{
		calculateRate(n, m, p, r, b, used, assigned, rate);
		int j = chooseVariable(n, rate, assigned);
		bool canTake = checkPossibility(j, m, used, r, b);
		assignValue(j, canTake, m, nassigned, assigned, x, used, r);	
	}

	delete[] assigned;
	delete[] used;
	delete[] rate;
}

int getCost(int n, int* x, int* p)
{
	int s = 0;
	for (int i = 0; i < n; i++)
	{
		s += x[i] * p[i];
	}

	return s;
}

int* chooseBestSolution(int n, int m, int* x, int* b, int* p, int** r)
{
	int* solution1 = new int[n]; //solution using sum as weight
	solveConstruction(n, m, solution1, b, p, r, calculateRate);

	int* solution2 = new int[n]; //solution using sqrt from sum of squares as weight
	solveConstruction(n, m, solution2, b, p, r, calculateRateNorm);

	if (getCost(n, solution1, p) > getCost(n, solution2, p))
	{
		delete[] solution2;
		return solution1;
	}
	else
	{
		delete[] solution1;
		return solution2;
	}
}

void solveConstructionBest(int n, int m, int* x, int* b, int* p, int** r)
{
	int* solution = chooseBestSolution(n, m, x, b, p, r);
	copyArray(n, solution, x);
	delete[] solution;
}

std::vector<int> getRandomPermutation(int n)
{
	std::vector<int> variables;
	for (int i = 0; i < n; i++)
	{
		variables.push_back(i);
	}
	std::random_shuffle(variables.begin(), variables.end());

	return variables;
}

void solveConstructionRandom(int n, int m, int* x, int* b, int* p, int** r)
{
	short* assigned = new short[n]; //1 if we already assigned value for a variable
	int* used = new int[m]; //amount of used space in every dimension
	int nassigned = 0; //number of assigned variables
	initSolution(n, m, assigned, used);

	std::vector<int> variables = getRandomPermutation(n);

	for (int i = 0; i < n; i++)
	{
		int j = variables[i];
		bool canTake = checkPossibility(j, m, used, r, b);
		assignValue(j, canTake, m, nassigned, assigned, x, used, r);
	}

	delete[] assigned;
	delete[] used;
}

double solveConstructionBestWithTime(int n, int m, int* x, int* b, int* p, int** r)
{
	// returns execution time in milliseconds

	auto start = std::chrono::high_resolution_clock::now();

	solveConstructionBest(n, m, x, b, p, r);

	auto stop = std::chrono::high_resolution_clock::now();

	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
	return microseconds * 1.0 / 1000;
}

void initImprovement(int n, int m, int* x, int* used, int** r)
{
	for (int i = 0; i < m; i++)
	{
		used[i] = 0;
		for (int j = 0; j < n; j++)
		{
			used[i] += x[j] * r[i][j];
		}
	}

}

void solveImprovement1(int n, int m, int* x, int* b, int* p, int** r)
{
	//1-flip

	int* used = new int[m]; //amount of used space in every dimension
	initImprovement(n, m, x, used, r);
	int currentCost = getCost(n, x, p);
	int bestImprovementCost;

	while (true)
	{
		bestImprovementCost = currentCost;
		int swap1 = -1;
		for (int c1 = 0; c1 < n; c1++)
		{	
			bool possible = true;
			for (int i = 0; i < m; i++)
			{
				int dif = (!x[c1] - x[c1]) * r[i][c1];
				if (used[i] + dif > b[i])
				{
					possible = false;
					break;
				}
			}
			if (possible)
			{
				int dif = (!x[c1] - x[c1]) * p[c1];
				if (currentCost + dif > bestImprovementCost)
				{
					bestImprovementCost = currentCost + dif;
					swap1 = c1;
				}
			}
		}
		if (bestImprovementCost > currentCost)
		{
			for (int i = 0; i < m; i++)
			{
				int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1];
				used[i] += constraintDif;
			}
			x[swap1] = !x[swap1];
			currentCost = bestImprovementCost;
		}
		else
		{
			break;
		}
	}

	delete[] used;
}

void solveImprovement2(int n, int m, int* x, int* b, int* p, int** r)
{
	//2-flip

	int* used = new int[m]; //amount of used space in every dimension
	initImprovement(n, m, x, used, r);
	int currentCost = getCost(n, x, p);
	int bestImprovementCost;

	while (true)
	{
		bestImprovementCost = currentCost;
		int swap1 = -1, swap2 = -1;
		for (int c1 = 0; c1 < n; c1++)
		{
			for (int c2 = c1 + 1; c2 < n; c2++)
			{
				bool possible = true;
				for (int i = 0; i < m; i++)
				{
					int dif = (!x[c1] - x[c1]) * r[i][c1] + (!x[c2] - x[c2]) * r[i][c2];
					if (used[i] + dif > b[i])
					{
						possible = false;
						break;
					}
				}
				if (possible)
				{
					int dif = (!x[c1] - x[c1]) * p[c1] + (!x[c2] - x[c2]) * p[c2];
					if (currentCost + dif > bestImprovementCost)
					{
						bestImprovementCost = currentCost + dif;
						swap1 = c1;
						swap2 = c2;
					}
				}
			}
		}
		if (bestImprovementCost > currentCost)
		{
			for (int i = 0; i < m; i++)
			{
				int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1] + (!x[swap2] - x[swap2]) * r[i][swap2];
				used[i] += constraintDif;
			}
			x[swap1] = !x[swap1];
			x[swap2] = !x[swap2];
			currentCost = bestImprovementCost;
		}
		else
		{
			break;
		}
	}
	
	delete[] used;
}

void solveImprovement3(int n, int m, int* x, int* b, int* p, int** r)
{
	//3-flip

	int* used = new int[m]; //amount of used space in every dimension
	initImprovement(n, m, x, used, r);
	int currentCost = getCost(n, x, p);
	int bestImprovementCost;

	while (true)
	{
		bestImprovementCost = currentCost;
		int swap1 = -1, swap2 = -1, swap3 = -1;
		for (int c1 = 0; c1 < n; c1++)
		{
			for (int c2 = c1 + 1; c2 < n; c2++)
			{
				for (int c3 = c2 + 1; c3 < n; c3++)
				{
					bool possible = true;
					for (int i = 0; i < m; i++)
					{
						int dif = (!x[c1] - x[c1]) * r[i][c1] + (!x[c2] - x[c2]) * r[i][c2] + (!x[c3] - x[c3]) * r[i][c3];
						if (used[i] + dif > b[i])
						{
							possible = false;
							break;
						}
					}
					if (possible)
					{
						int dif = (!x[c1] - x[c1]) * p[c1] + (!x[c2] - x[c2]) * p[c2] + (!x[c3] - x[c3]) * p[c3];
						if (currentCost + dif > bestImprovementCost)
						{
							bestImprovementCost = currentCost + dif;
							swap1 = c1;
							swap2 = c2;
							swap3 = c3;
						}
					}
				}
			}
		}
		if (bestImprovementCost > currentCost)
		{
			for (int i = 0; i < m; i++)
			{
				int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1] + (!x[swap2] - x[swap2]) * r[i][swap2] + (!x[swap3] - x[swap3]) * r[i][swap3];
				used[i] += constraintDif;
			}
			x[swap1] = !x[swap1];
			x[swap2] = !x[swap2];
			x[swap3] = !x[swap3];
			currentCost = bestImprovementCost;
		}
		else
		{
			break;
		}
	}

	delete[] used;
}

void solveImprovement4(int n, int m, int* x, int* b, int* p, int** r)
{
	//4-flip

	int* used = new int[m]; //amount of used space in every dimension
	initImprovement(n, m, x, used, r);
	int currentCost = getCost(n, x, p);
	int bestImprovementCost;

	while (true)
	{
		bestImprovementCost = currentCost;
		int swap1 = -1, swap2 = -1, swap3 = -1, swap4 = -1;
		for (int c1 = 0; c1 < n; c1++)
		{
			for (int c2 = c1 + 1; c2 < n; c2++)
			{
				for (int c3 = c2 + 1; c3 < n; c3++)
				{
					for (int c4 = c3 + 1; c4 < n; c4++)
					{
						bool possible = true;
						for (int i = 0; i < m; i++)
						{
							int dif = (!x[c1] - x[c1]) * r[i][c1] + (!x[c2] - x[c2]) * r[i][c2] + (!x[c3] - x[c3]) * r[i][c3] + (!x[c4] - x[c4]) * r[i][c4];
							if (used[i] + dif > b[i])
							{
								possible = false;
								break;
							}
						}
						if (possible)
						{
							int dif = (!x[c1] - x[c1]) * p[c1] + (!x[c2] - x[c2]) * p[c2] + (!x[c3] - x[c3]) * p[c3] + (!x[c4] - x[c4]) * p[c4];
							if (currentCost + dif > bestImprovementCost)
							{
								bestImprovementCost = currentCost + dif;
								swap1 = c1;
								swap2 = c2;
								swap3 = c3;
								swap4 = c4;
							}
						}
					}
				}
			}
		}
		if (bestImprovementCost > currentCost)
		{
			for (int i = 0; i < m; i++)
			{
				int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1] + (!x[swap2] - x[swap2]) * r[i][swap2] + (!x[swap3] - x[swap3]) * r[i][swap3] + (!x[swap4] - x[swap4]) * r[i][swap4];
				used[i] += constraintDif;
			}
			x[swap1] = !x[swap1];
			x[swap2] = !x[swap2];
			x[swap3] = !x[swap3];
			x[swap4] = !x[swap4];
			currentCost = bestImprovementCost;
		}
		else
		{
			break;
		}
	}

	delete[] used;
}

void printSolution(int n, int* x)
{
	for (int i = 0; i < n; i++)
	{
		std::cout << x[i] << ' ';
	}
}

void initInput(std::ifstream& fin,int n, int m, int* p, int** r, int* b)
{
	for (int i = 0; i < n; i++)
	{
		fin >> p[i];
	}


	for (int i = 0; i < m; i++)
	{
		r[i] = new int[n];
		for (int j = 0; j < n; j++)
		{
			fin >> r[i][j];
		}
	}


	for (int i = 0; i < m; i++)
	{
		fin >> b[i];
	}
}

bool isFeasible(int n, int m, int* x, int** r, int* b)
{
	bool feasible = true;
	for (int i = 0; i < m; i++)
	{
		int u = 0;
		for (int j = 0; j < n; j++)
		{
			u += r[i][j] * x[j];
		}
		feasible &= (b[i] >= u);
	}
	return feasible;
}

int main(int argc, char* argv[])
{
	srand(time(0));
	auto fin = openFile(argc, argv);

	//start init
	int n, m, q, opt;
	int* p, * b, * x;
	int** r;

	fin >> n >> m >> q >> opt;

	p = new int[n];
	r = new int* [m];
	b = new int[m];

	initInput(fin, n, m, p, r, b);

	x = new int[n]; //solution
	double duration = 0;
	//duration = solveConstructionBestWithTime(n, m, x, b, p, r);
	solveConstruction(n, m, x, b, p, r, calculateRateEasy);
	//std::cout << "before improvenemt cost: " << getCost(n, x, p) << '\n';

	if (n <= 300)
	{
		solveImprovement4(n, m, x, b, p, r);
	}
	solveImprovement3(n, m, x, b, p, r);
	solveImprovement2(n, m, x, b, p, r);
	solveImprovement1(n, m, x, b, p, r);


	int s = getCost(n, x, p);

	//std::cout << isFeasible(n,m,x,r,b) << "\n";

	//printSolution(n, x);
    //std::cout << s << "\t" << duration << " milliseconds\n";
	std::cout << s << '\n';
}
