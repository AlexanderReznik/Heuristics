// Multidim Knapsack Construction Heuristics.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <time.h>
#include <chrono>

void tabuSearch(int, int, int*, int*, int*, int**, double , double , int );

#pragma region Common

void copyArray(int size, int* source, int* destination)
{
	for (int i = 0; i < size; i++)
	{
		destination[i] = source[i];
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

int getCost(int n, int* x, int* p)
{
	int s = 0;
	for (int i = 0; i < n; i++)
	{
		s += x[i] * p[i];
	}

	return s;
}

void initUsed(int n, int m, int* x, int* used, int** r)
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

void printSolution(int n, int* x)
{
	for (int i = 0; i < n; i++)
	{
		std::cout << x[i] << ' ';
	}
}

void initInput(std::ifstream& fin, int n, int m, int* p, int** r, int* b)
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

int* chooseBestSolution(std::vector<int*> solutions, int n, int* p)
{
	int bestIndex = -1;
	int bestCost = -1;
	for (int i = 0; i < solutions.size(); i++)
	{
		int cost = getCost(n, solutions[i], p);
		//std::cout << cost << ' ';
		if (cost > bestCost)
		{
			bestIndex = i;
			bestCost = cost;
		}
	}
	return solutions[bestIndex];
}

double solveWithTime(int n, int m, int* x, int* b, int* p, int** r, void solveImprovement(int, int, int*, int*, int*, int**))
{
	auto start = std::chrono::high_resolution_clock::now();

	solveImprovement(n, m, x, b, p, r);

	auto stop = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();

	return microseconds;
}

#pragma endregion


#pragma region Construction

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



int* chooseBestConstruction(int n, int m, int* b, int* p, int** r)
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
	int* solution = chooseBestConstruction(n, m, b, p, r);
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

#pragma endregion


#pragma region Improvement

void solveImprovementFlip1(int n, int m, int* x, int* b, int* p, int** r, bool mock)
{
	//1-flip

	int* used = new int[m]; //amount of used space in every dimension
	initUsed(n, m, x, used, r);
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

void findImprovement2(int n, int m, int* x, int** r, int* used, int* b, int* p, int& currentCost, int& bestImprovementCost, int& swap1, int& swap2)
{
	for (int c1 = 0; c1 < n; c1++)
	{
		for (int c2 = c1 + 1; c2 < n; c2++)
		{
			if (x[c1] + x[c2] == 1)
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
	}
}

void returnToBest2(int* x, int* p, int& swap1, int& swap2)
{
	// if there was at least one swap and previous solution is better than current (all solutions in search are feasible)
	// than go back to previous
	if (swap1 != -1 && p[swap1] * x[swap1] + p[swap2] * x[swap2] < p[swap1] * !x[swap1] + p[swap2] * !x[swap2])
	{
		x[swap1] = !x[swap1];
		x[swap2] = !x[swap2];
	}
}

bool makeMove2(int n, int m, int* x, int** r, int* used, int& currentCost, int& bestImprovementCost, int& previousCost, int& swap1, int& swap2, bool jagged)
{
	if (bestImprovementCost <= (jagged ? previousCost : currentCost))
	{
		return false;
	}

	for (int i = 0; i < m; i++)
	{
		int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1] + (!x[swap2] - x[swap2]) * r[i][swap2];
		used[i] += constraintDif;
	}
	x[swap1] = !x[swap1];
	x[swap2] = !x[swap2];
	if (jagged)
	{
		previousCost = currentCost;
	}
	currentCost = bestImprovementCost;

	return true;
}

void solveImprovementFlip2(int n, int m, int* x, int* b, int* p, int** r, bool jagged)
{
	//2-flip

	int* used = new int[m]; //amount of used space in every dimension
	initUsed(n, m, x, used, r);
	int currentCost = getCost(n, x, p);
	int bestImprovementCost, previousCost = currentCost;
	int swap1, swap2;

	while (true)
	{
		bestImprovementCost = 0;
		swap1 = -1;
		swap2 = -1;
		findImprovement2(n, m, x, r, used, b, p, currentCost, bestImprovementCost, swap1, swap2);
		bool moved = makeMove2(n, m, x, r, used, currentCost, bestImprovementCost, previousCost, swap1, swap2, jagged);
		if (!moved)
		{
			break;
		}
	}

	if (jagged)
	{
		returnToBest2(x, p, swap1, swap2);
	}

	delete[] used;
}


void findImprovement3(int n, int m, int* x, int** r, int* used, int* b, int* p, int& currentCost, int& bestImprovementCost, int& swap1, int& swap2, int& swap3)
{
	for (int c1 = 0; c1 < n; c1++)
	{
		for (int c2 = c1 + 1; c2 < n; c2++)
		{
			for (int c3 = c2 + 1; c3 < n; c3++)
			{
				int sum = x[c1] + x[c2] + x[c3];
				if (sum > 0 && sum < 3)
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
	}
}

void returnToBest3(int* x, int* p, int& swap1, int& swap2, int& swap3)
{
	// if there was at least one swap and previous solution is better than current (all solutions in search are feasible)
	// than go back to previous
	if (swap1 != -1 && p[swap1] * x[swap1] + p[swap2] * x[swap2] + p[swap3] * x[swap3] < p[swap1] * !x[swap1] + p[swap2] * !x[swap2] + p[swap3] * !x[swap3])
	{
		x[swap1] = !x[swap1];
		x[swap2] = !x[swap2];
		x[swap3] = !x[swap3];
	}
}

bool makeMove3(int n, int m, int* x, int** r, int* used, int& currentCost, int& bestImprovementCost, int& previousCost, int& swap1, int& swap2, int& swap3, bool jagged)
{
	if (bestImprovementCost <= (jagged ? previousCost : currentCost))
	{
		return false;
	}

	for (int i = 0; i < m; i++)
	{
		int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1] + (!x[swap2] - x[swap2]) * r[i][swap2] + (!x[swap3] - x[swap3]) * r[i][swap3];
		used[i] += constraintDif;
	}
	x[swap1] = !x[swap1];
	x[swap2] = !x[swap2];
	x[swap3] = !x[swap3];
	if (jagged)
	{
		previousCost = currentCost;
	}
	currentCost = bestImprovementCost;

	return true;
}

void solveImprovementFlip3(int n, int m, int* x, int* b, int* p, int** r, bool jagged)
{
	//3-flip

	int* used = new int[m]; //amount of used space in every dimension
	initUsed(n, m, x, used, r);
	int currentCost = getCost(n, x, p);
	int bestImprovementCost, previousCost = currentCost;
	int swap1, swap2, swap3;

	while (true)
	{
		bestImprovementCost = 0;
		swap1 = -1;
		swap2 = -1;
		swap3 = -1;
		findImprovement3(n, m, x, r, used, b, p, currentCost, bestImprovementCost, swap1, swap2, swap3);
		bool moved = makeMove3(n, m, x, r, used, currentCost, bestImprovementCost, previousCost, swap1, swap2, swap3, jagged);
		if (!moved)
		{
			break;
		}
	}

	if (jagged)
	{
		returnToBest3(x, p, swap1, swap2, swap3);
	}

	delete[] used;
}


void findImprovement4(int n, int m, int* x, int** r, int* used, int* b, int* p, int& currentCost, int& bestImprovementCost, int& swap1, int& swap2, int& swap3, int& swap4)
{
	for (int c1 = 0; c1 < n; c1++)
	{
		for (int c2 = c1 + 1; c2 < n; c2++)
		{
			for (int c3 = c2 + 1; c3 < n; c3++)
			{
				for (int c4 = c3 + 1; c4 < n; c4++)
				{
					int sum = x[c1] + x[c2] + x[c3] + x[c4];
					if (sum > 0 && sum < 4)
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
	}
}

void returnToBest4(int* x, int* p, int& swap1, int& swap2, int& swap3, int& swap4)
{
	// if there was at least one swap and previous solution is better than current (all solutions in search are feasible)
	// than go back to previous
	if (swap1 != -1 && p[swap1] * x[swap1] + p[swap2] * x[swap2] + p[swap3] * x[swap3] + p[swap4] * x[swap4] < p[swap1] * !x[swap1] + p[swap2] * !x[swap2] + p[swap3] * !x[swap3] + p[swap4] * !x[swap4])
	{
		x[swap1] = !x[swap1];
		x[swap2] = !x[swap2];
		x[swap3] = !x[swap3];
		x[swap4] = !x[swap4];
	}
}

bool makeMove4(int n, int m, int* x, int** r, int* used, int& currentCost, int& bestImprovementCost, int& previousCost, int& swap1, int& swap2, int& swap3, int& swap4, bool jagged)
{
	if (bestImprovementCost <= (jagged ? previousCost : currentCost))
	{
		return false;
	}

	for (int i = 0; i < m; i++)
	{
		int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1] + (!x[swap2] - x[swap2]) * r[i][swap2] + (!x[swap3] - x[swap3]) * r[i][swap3] + (!x[swap4] - x[swap4]) * r[i][swap4];
		used[i] += constraintDif;
	}
	x[swap1] = !x[swap1];
	x[swap2] = !x[swap2];
	x[swap3] = !x[swap3];
	x[swap4] = !x[swap4];
	if (jagged)
	{
		previousCost = currentCost;
	}
	currentCost = bestImprovementCost;

	return true;
}

void solveImprovementFlip4(int n, int m, int* x, int* b, int* p, int** r, bool jagged)
{
	//4-flip

	int* used = new int[m]; //amount of used space in every dimension
	initUsed(n, m, x, used, r);
	int currentCost = getCost(n, x, p);
	int bestImprovementCost, previousCost = currentCost;
	int swap1, swap2, swap3, swap4;

	while (true)
	{
		bestImprovementCost = 0;
		swap1 = -1;
		swap2 = -1;
		swap3 = -1;
		swap4 = -1;
		findImprovement4(n, m, x, r, used, b, p, currentCost, bestImprovementCost, swap1, swap2, swap3, swap4);
		bool moved = makeMove4(n, m, x, r, used, currentCost, bestImprovementCost, previousCost, swap1, swap2, swap3, swap4, jagged);
		if (!moved)
		{
			break;
		}
	}

	if (jagged)
	{
		returnToBest4(x, p, swap1, swap2, swap3, swap4);
	}

	delete[] used;
}


void solveImprovementFlip24(int n, int m, int* x, int* b, int* p, int** r, bool jagged)
{
	int* used = new int[m]; //amount of used space in every dimension
	initUsed(n, m, x, used, r);
	int bestImprovementCost, currentCost;
	int swap1, swap2, swap3, swap4;

	while (true)
	{
		solveImprovementFlip2(n, m, x, b, p, r, jagged);

		//4-flip
		initUsed(n, m, x, used, r);
		currentCost = getCost(n, x, p);
		bestImprovementCost = 0;
		swap1 = -1;
		swap2 = -1;
		swap3 = -1;
		swap4 = -1;
		findImprovement4(n, m, x, r, used, b, p, currentCost, bestImprovementCost, swap1, swap2, swap3, swap4);
		bool moved = makeMove4(n, m, x, r, used, currentCost, bestImprovementCost, currentCost, swap1, swap2, swap3, swap4, false);
		if (!moved)
		{
			break;
		}
	}

	delete[] used;
}

void solveImprovementFlip23(int n, int m, int* x, int* b, int* p, int** r, bool jagged)
{
	int* used = new int[m]; //amount of used space in every dimension
	initUsed(n, m, x, used, r);
	int bestImprovementCost, currentCost;
	int swap1, swap2, swap3;

	while (true)
	{
		solveImprovementFlip2(n, m, x, b, p, r, jagged);

		//4-flip
		initUsed(n, m, x, used, r);
		currentCost = getCost(n, x, p);
		bestImprovementCost = 0;
		swap1 = -1;
		swap2 = -1;
		swap3 = -1;
		findImprovement3(n, m, x, r, used, b, p, currentCost, bestImprovementCost, swap1, swap2, swap3);
		bool moved = makeMove3(n, m, x, r, used, currentCost, bestImprovementCost, currentCost, swap1, swap2, swap3, false);
		if (!moved)
		{
			break;
		}
	}

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

double solveImprovementWithTime(int n, int m, int* x, int* b, int* p, int** r, bool jagged, void solveImprovement(int, int, int*, int*, int*, int**, bool))
{
	// returns execution time in milliseconds

	auto start = std::chrono::high_resolution_clock::now();

	solveImprovement(n, m, x, b, p, r, jagged);

	auto stop = std::chrono::high_resolution_clock::now();

	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
	return microseconds * 1.0 / 1000;
}


void solveBestImprovement(int n, int m, int* x, int* b, int* p, int** r)
{
	std::vector<int*> solutions;

	int* solution1 = new int[n];
	solveConstruction(n, m, solution1, b, p, r, calculateRateEasy);
	if (n <= 200)
	{
		solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip4);
	}
	solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip3);
	solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip2);
	solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip1);
	solutions.push_back(solution1);

	int* solution2 = new int[n];
	solveConstruction(n, m, solution2, b, p, r, calculateRate);
	if (n <= 270)
	{
		solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip4);
	}
	solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip3);
	solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip2);
	solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip1);
	solutions.push_back(solution2);

	int* solution3 = new int[n];
	solveConstruction(n, m, solution3, b, p, r, calculateRateNorm);
	if (n <= 200)
	{
		solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip4);
	}
	solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip3);
	solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip2);
	solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip1);
	solutions.push_back(solution3);

	int* bestSolution = chooseBestSolution(solutions, n, p);
	copyArray(n, bestSolution, x);

	delete[] solution1;
	delete[] solution2;
	delete[] solution3;

	//std::cout << isFeasible(n,m,x,r,b) << "\n";

	//printSolution(n, x);
}

void solveTestImprovement(int n, int m, int* x, int* b, int* p, int** r)
{
	std::vector<int*> solutions;

	int* solution1 = new int[n];
	solveConstruction(n, m, solution1, b, p, r, calculateRateEasy);
	if (n <= 200)
	{
		solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip24);
	}
	else
	{
		solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip23);
	}
	solutions.push_back(solution1);

	int* solution2 = new int[n];
	solveConstruction(n, m, solution2, b, p, r, calculateRate);
	if (n <= 270)
	{
		solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip24);
	}
	else
	{
		solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip23);
	}
	solutions.push_back(solution2);

	int* solution3 = new int[n];
	solveConstruction(n, m, solution3, b, p, r, calculateRateNorm);
	if (n <= 200)
	{
		solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip24);
	}
	else
	{
		solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip23);
	}
	solutions.push_back(solution3);

	int* bestSolution = chooseBestSolution(solutions, n, p);
	copyArray(n, bestSolution, x);

	delete[] solution1;
	delete[] solution2;
	delete[] solution3;

	//std::cout << isFeasible(n,m,x,r,b) << "\n";

	//printSolution(n, x);
}

void solveQuickImprovement(int n, int m, int* x, int* b, int* p, int** r)
{
	std::vector<int*> solutions;

	int* solution1 = new int[n];
	solveConstruction(n, m, solution1, b, p, r, calculateRateEasy);
	if (n <= 200)
	{
		solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip3);
	}
	solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip2);
	solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip1);
	solutions.push_back(solution1);

	int* solution2 = new int[n];
	solveConstruction(n, m, solution2, b, p, r, calculateRate);
	if (n <= 200)
	{
		solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip3);
	}
	solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip2);
	solveImprovementWithTime(n, m, solution2, b, p, r, true, solveImprovementFlip1);
	solutions.push_back(solution2);

	int* solution3 = new int[n];
	solveConstruction(n, m, solution3, b, p, r, calculateRateNorm);
	if (n <= 200)
	{
		solveImprovementWithTime(n, m, solution1, b, p, r, true, solveImprovementFlip3);
	}
	solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip2);
	solveImprovementWithTime(n, m, solution3, b, p, r, true, solveImprovementFlip1);
	solutions.push_back(solution3);

	int* bestSolution = chooseBestSolution(solutions, n, p);
	copyArray(n, bestSolution, x);

	delete[] solution1;
	delete[] solution2;
	delete[] solution3;

	//std::cout << isFeasible(n,m,x,r,b) << "\n";

	//printSolution(n, x);
}

void improve(int n, int m, int* x, int* b, int* p, int** r, int limitFor4)
{
	if (n <= limitFor4)
	{
		solveImprovementFlip4(n, m, x, b, p, r, true);
	}
	solveImprovementFlip3(n, m, x, b, p, r, true);
	solveImprovementFlip2(n, m, x, b, p, r, true);
	solveImprovementFlip1(n, m, x, b, p, r, true);
}

#pragma endregion


#pragma region Tabu

double evaluate(int objective, int infeasibility, double alpha)
{
	double value = objective - alpha * infeasibility;
	return value;
}

int getTabuTenure(int n, double beta)
{
	//return round(beta * sqrt(n));
	int tenure = (5 + 9 * rand() / RAND_MAX);
	return tenure;
}



void findTabuImprovement2(int n, int m, int* x, int** r, int* used, int* b, int* p, int* tabu, int iteration, int currentObjective, double& bestImprovementCost, int& swap1, int& swap2, double alpha)
{
	for (int c1 = 0; c1 < n; c1++)
	{
		for (int c2 = c1 + 1; c2 < n; c2++)
		{
			if (x[c1] + x[c2] == 1)
			{
				int infeasibility = 0;
				for (int i = 0; i < m; i++)
				{
					int dif = (!x[c1] - x[c1]) * r[i][c1] + (!x[c2] - x[c2]) * r[i][c2];
					if (used[i] + dif > b[i])
					{
						infeasibility += used[i] + dif - b[i];
					}
				}

				if (tabu[c1] < iteration && tabu[c2] < iteration)
				{
					int dif = (!x[c1] - x[c1]) * p[c1] + (!x[c2] - x[c2]) * p[c2];
					double cost = evaluate(currentObjective + dif, infeasibility, alpha);
					if (cost > bestImprovementCost)
					{
						bestImprovementCost = cost;
						swap1 = c1;
						swap2 = c2;
					}
				}
			}
		}
	}
}

bool makeTabuMove2(int n, int m, int* x, int** r, int* used, int* b, int* p, int* tabu, int iteration, int& currentObjective, double& currentCost, double& bestImprovementCost, int& swap1, int& swap2, double beta)
{
	if (bestImprovementCost > -10e9)
	{
		bool feasible = true;
		for (int i = 0; i < m; i++)
		{
			int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1] + (!x[swap2] - x[swap2]) * r[i][swap2];
			used[i] += constraintDif;
			if (used[i] > b[i])
			{
				feasible = false;
			}
		}

		int objDif = (!x[swap1] - x[swap1]) * p[swap1] + (!x[swap2] - x[swap2]) * p[swap2];
		currentObjective += objDif;

		x[swap1] = !x[swap1];
		x[swap2] = !x[swap2];

		tabu[swap1] = iteration + getTabuTenure(n, beta);
		tabu[swap2] = iteration + getTabuTenure(n, beta);

		currentCost = bestImprovementCost;

		return feasible;
	}

	return false;
}

void tabuSearch2(int n, int m, int* x, int* b, int* p, int** r, double alpha = 0.5, double beta = 1, int seconds = 15)
{
	int* currentSolution = new int[n];
	int* bestSolution = new int[n];
	int* used = new int[m];

	copyArray(n, x, currentSolution);
	copyArray(n, x, bestSolution);
	initUsed(n, m, currentSolution, used, r);

	int currentObjective = getCost(n, currentSolution, p);
	double bestCost = evaluate(currentObjective, 0, alpha);
	double currentCost = bestCost;
	int iteration = 0;

	int swap1 = -1;
	int swap2 = -1;
	double bestImprovementCost;

	int* tabu = new int[n];
	for (int i = 0; i < n; i++)
	{
		tabu[i] = -1;
	}

	int total = 0;
	int maxIteration = 10e9 / n / n / sqrt(n);
	auto start = std::chrono::high_resolution_clock::now();
	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < seconds)
	{
		iteration++;
		bestImprovementCost = -10e9;
		findTabuImprovement2(n, m, currentSolution, r, used, b, p, tabu, iteration, currentObjective, bestImprovementCost, swap1, swap2, alpha);
		bool feasible = makeTabuMove2(n, m, currentSolution, r, used, b, p, tabu, iteration, currentObjective, currentCost, bestImprovementCost, swap1, swap2, beta);

		if (feasible)
		{
			total++;
		}

		if (currentCost > bestCost && feasible)
		{
			bestCost = currentCost;
			copyArray(n, currentSolution, bestSolution);
			//std::cout <<"temp:" <<isFeasible(n, m, x, r, b) << ' ' << getCost(n, bestSolution, p) << '\n';
		}
	}
	//std::cout << total << '/' << maxIteration << '=' << 100.0*total / maxIteration << '\n';
	copyArray(n, bestSolution, x);
}


void findTabuImprovement(int n, int m, int* x, int** r, int* used, int* b, int* p, int* tabu, int iteration, int currentObjective, double& bestImprovementCost, double& bestFeasibleCost, int& swap1, double alpha)
{
	for (int c1 = 0; c1 < n; c1++)
	{
		int infeasibility = 0;
		for (int i = 0; i < m; i++)
		{
			int dif = (!x[c1] - x[c1]) * r[i][c1];
			if (used[i] + dif > b[i])
			{
				infeasibility += used[i] + dif - b[i];
			}
		}

		int dif = (!x[c1] - x[c1]) * p[c1];
		double cost = evaluate(currentObjective + dif, infeasibility, alpha);

		if (cost > bestImprovementCost && (tabu[c1] < iteration || (cost > bestFeasibleCost && infeasibility == 0)))
		{
			bestImprovementCost = cost;
			swap1 = c1;
		}
	}
}

bool makeTabuMove(int n, int m, int* x, int** r, int* used, int* b, int* p, int* tabu, int iteration, int& currentObjective, double& currentCost, double& bestImprovementCost, int& swap1, double beta)
{
	if (bestImprovementCost == -10e9)
	{
		return false;
	}

	bool feasible = true;
	for (int i = 0; i < m; i++)
	{
		int constraintDif = (!x[swap1] - x[swap1]) * r[i][swap1];
		used[i] += constraintDif;
		if (used[i] > b[i])
		{
			feasible = false;
		}
	}

	int objDif = (!x[swap1] - x[swap1]) * p[swap1];
	currentObjective += objDif;

	x[swap1] = !x[swap1];

	tabu[swap1] = iteration + getTabuTenure(n, beta);

	currentCost = bestImprovementCost;

	return feasible;
}

void tabuSearch(int n, int m, int* x, int* b, int* p, int** r, double alpha = 1, double beta = 1, int seconds = 19)
{
	int* currentSolution = new int[n];
	int* bestFeasibleSolution = new int[n];
	int* used = new int[m];

	copyArray(n, x, currentSolution);
	copyArray(n, x, bestFeasibleSolution);
	initUsed(n, m, currentSolution, used, r);

	int currentObjective = getCost(n, currentSolution, p);
	double bestFeasibleCost = evaluate(currentObjective, 0, alpha);
	double currentCost = bestFeasibleCost;
	int iteration = 0;

	int swap1 = -1;
	double bestImprovementCost;

	int* tabu = new int[n];
	for (int i = 0; i < n; i++)
	{
		tabu[i] = -1;
	}

	int changes = 0;
	bool previous = true;

	auto start = std::chrono::high_resolution_clock::now();

	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < seconds)
	//while (iteration < 33000)
	{
		iteration++;
		bestImprovementCost = -10e9;
		findTabuImprovement(n, m, currentSolution, r, used, b, p, tabu, iteration, currentObjective, bestImprovementCost, bestFeasibleCost, swap1, alpha);
		bool feasible = makeTabuMove(n, m, currentSolution, r, used, b, p, tabu, iteration, currentObjective, currentCost, bestImprovementCost, swap1, beta);

		if (feasible != previous)
		{
			changes++;
		}

		previous = feasible;

		if (feasible && currentCost > bestFeasibleCost)
		{
			bestFeasibleCost = currentCost;
			copyArray(n, currentSolution, bestFeasibleSolution);
		}

	}
	copyArray(n, bestFeasibleSolution, x);
	//std::cout << changes << '/' << iteration << ':' << changes * 100.0 / iteration << '\n';
}



void metaheuristicTest(int n, int m, int* x, int* b, int* p, int** r)
{
	std::vector<int*> solutions;

	int* solution1 = new int[n];
	solveConstruction(n, m, solution1, b, p, r, calculateRateEasy);
	tabuSearch(n, m, solution1, b, p, r);
	//improve(n, m, solution1, b, p, r, 100);
	solutions.push_back(solution1);

	int* solution2 = new int[n];
	solveConstruction(n, m, solution2, b, p, r, calculateRate);
	tabuSearch(n, m, solution2, b, p, r);
	//improve(n, m, solution2, b, p, r, 250);
	solutions.push_back(solution2);

	int* solution3 = new int[n];
	solveConstruction(n, m, solution3, b, p, r, calculateRateNorm);
	tabuSearch(n, m, solution3, b, p, r);
	//improve(n, m, solution1, b, p, r, 100);
	solutions.push_back(solution3);

	int* bestSolution = chooseBestSolution(solutions, n, p);
	copyArray(n, bestSolution, x);

	delete[] solution1;
	delete[] solution2;
	delete[] solution3;
}

#pragma endregion


#pragma region GRASP

double findMin(short* indexes, int k, double* rate, double& min)
{
	min = rate[indexes[0]];
	short minIndex = 0;

	for (int i = 1; i < k; i++)
	{
		if (rate[indexes[i]] < min)
		{
			min = rate[indexes[i]];
			minIndex = i;
		}
	}

	return minIndex;
}

void createReducedCandList(int n, double* rate, short* assigned, int candNumber, short* reducedCandidateList)
{
	int i = 0;
	int k = 0;
	short minIndex;
	double min;

	for (; i < n && k < candNumber; i++)
	{
		if (!assigned[i])
		{
			reducedCandidateList[k] = i;
			k++;
		}
	}
	
	minIndex = findMin(reducedCandidateList, candNumber, rate, min);

	for (; i < n; i++)
	{
		if (!assigned[i] && rate[i] > min)
		{
			reducedCandidateList[minIndex] = i;
			minIndex = findMin(reducedCandidateList, candNumber, rate, min);
		}
	}
}

short chooseRandomVariable(short* reducedCandList, int candNumber)
{
	return reducedCandList[rand() % candNumber];
}

void GRASP(int n, int m, int* x, int* b, int* p, int** r, double alpha, void calculateRate(int, int, int*, int**, int*, int*, short*, double*))
{
	short* assigned = new short[n]; //1 if we already assigned value for a variable
	int* used = new int[m]; //amount of used space in every dimension
	double* rate = new double[n];//rate used to choose variable
	int nassigned = 0; //number of assigned variables
	initSolution(n, m, assigned, used);

	int candNumber;
	short* reducedCandList = new short[n];

	//start algo
	while (nassigned < n)
	{
		calculateRate(n, m, p, r, b, used, assigned, rate);

		candNumber = ceil(alpha * (n - nassigned));
		createReducedCandList(n, rate, assigned, candNumber, reducedCandList);

		int j = chooseRandomVariable(reducedCandList, candNumber);
		bool canTake = checkPossibility(j, m, used, r, b);
		assignValue(j, canTake, m, nassigned, assigned, x, used, r);
		//std::cout << getCost(n, x, p) << '\n';
		//printSolution(n, x);
	}

	delete[] assigned;
	delete[] used;
	delete[] rate;
}

void testGrasp(int n, int m, int* x, int* b, int* p, int** r, void calculateRate(int, int, int*, int**, int*, int*, short*, double*), double alpha, int seconds)
{
	auto start = std::chrono::high_resolution_clock::now();

	int64_t sum = 0;
	int count = 0;
	int best = -1;

	int* solution = new int[n];

	//for (int i = 0; i < 10; i++)
	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < seconds)
	{
		count++;
		GRASP(n, m, solution, b, p, r, alpha, calculateRate);
		int cost = getCost(n, solution, p);
		sum += cost;
		if (cost > best)
		{
			best = cost;
			copyArray(n, solution, x);
		}
	}

	std::cout << alpha << ' ' << sum * 1.0 / count << ' ' << best << '\n';

}

void metaheuristic(int n, int m, int* x, int* b, int* p, int** r)
{
	auto start = std::chrono::high_resolution_clock::now();
	std::vector<int*> solutions;

	int* solution1 = new int[n];
	solveConstruction(n, m, solution1, b, p, r, calculateRateEasy);
	improve(n, m, solution1, b, p, r, 100);
	solutions.push_back(solution1);

	int* solution2 = new int[n];
	solveConstruction(n, m, solution2, b, p, r, calculateRate);
	improve(n, m, solution2, b, p, r, 250);
	solutions.push_back(solution2);

	int* solution3 = new int[n];
	solveConstruction(n, m, solution3, b, p, r, calculateRateNorm);
	improve(n, m, solution3, b, p, r, 100);
	solutions.push_back(solution3);


	int* solution = new int[n];
	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 50)
	{
		int best = 0/*getCost(n, x, p)*/;
		int* bestSolution = new int[n];

		auto graspStart = std::chrono::high_resolution_clock::now();
		while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - graspStart).count() < 1)
		{
			GRASP(n, m, solution, b, p, r, 0.07, calculateRate);


			int cost = getCost(n, solution, p);
			if (cost > best)
			{
				//std::cout << "Opa" << ' ';
				best = cost;
				copyArray(n, solution, bestSolution);
			}
		}

		improve(n, m, bestSolution, b, p, r, 250);
		solutions.push_back(bestSolution);
	}

	int* bestSolution = chooseBestSolution(solutions, n, p);
	copyArray(n, bestSolution, x);

	for (auto sol : solutions)
	{
		delete[] sol;
	}

}

#pragma endregion


int main(int argc, char* argv[])
{
	srand(14);
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
	x = new int[n];
	
	//GRASP(n, m, x, b, p, r, 0.1, calculateRateEasy);


	//testGrasp(n, m, x, b, p, r, calculateRateNorm, 0.07, 20);
	
	//metaheuristic(n, m, x, b, p, r);

	//double duration = solveBestImprovementWithTime(n, m, x, b, p, r, solveTestImprovement);

	double duration = solveWithTime(n, m, x, b, p, r, metaheuristic);

	//solveConstruction(n, m, x, b, p, r, calculateRate);
	//tabuSearch(n, m, x, b, p, r, 1.5, 1, 30);

	//printSolution(n, x);
	//std::cout << isFeasible(n, m, x, r, b) << ' ';
	std::cout << getCost(n, x, p) << ' ';
	std::cout << duration * 1.0 / 1000000;
	std::cout << '\n';
    //std::cout <<isFeasible(n, m, x, r, b) << ' ' << getCost(n, x, p) << '\t' << duration * 1.0 /1000000 << " seconds\n";
}
