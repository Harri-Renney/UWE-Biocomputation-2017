#include <iostream>
#include <stdlib.h>
#include <time.h>

#define LOG_FILE 1

#define POPULATION_SIZE 1000
#define RULEBASE_SIZE 15
#define INPUT_DATA_SIZE 5
#define OUTPUT_DATA_SIZE 1
#define GENE_SIZE RULEBASE_SIZE * (INPUT_DATA_SIZE + OUTPUT_DATA_SIZE)
#define DATA_SET_SIZE 32
#define NUMBER_OF_GENERATIONS 250

float mutation_rate = 1 / (float)(RULEBASE_SIZE * (INPUT_DATA_SIZE + OUTPUT_DATA_SIZE));
float crossover_rate = 0.07;

struct individual
{
	int gene[GENE_SIZE];
	int fitness = 0;
};

individual population[POPULATION_SIZE];
individual offspring[POPULATION_SIZE];

struct rule
{
	int condition[INPUT_DATA_SIZE];
	int output;
};

struct data
{
	int variables[INPUT_DATA_SIZE];
	int output;
};

float deterministicMutation(int currentGeneration)
{
	int n = (INPUT_DATA_SIZE + OUTPUT_DATA_SIZE) * RULEBASE_SIZE;
	int T = NUMBER_OF_GENERATIONS;
	int t = currentGeneration;
	int k = -1;
	return pow((2 + ((float)(n - 2) / (float)(T - 1)) * t), k);
}

void testIndividual(individual ind)
{
	int newData[6];

	for (int i = 0; i != INPUT_DATA_SIZE; ++i)
	{
		newData[i] = std::cin.get() - 48;
	}
	std::cin.get();

	for (int i = 0; i != GENE_SIZE; i += INPUT_DATA_SIZE + OUTPUT_DATA_SIZE)
	{
		int k = 0;
		for (int j = 0; j != INPUT_DATA_SIZE; ++j)
		{
			if (ind.gene[i + j] == newData[j] || ind.gene[i + j] == 2)
				++k;
		}
		if (k == INPUT_DATA_SIZE)
		{
			std::cout << "Rulebase predicted output: " << ind.gene[i + INPUT_DATA_SIZE] << std::endl;
			break;
		}
	}
}

void printGene(individual ind)
{
	for (int i = 0; i != GENE_SIZE; ++i)
	{
		std::cout << ind.gene[i];
	}
	std::cout << " With fitness value " << ind.fitness << std::endl;
}


void printPopulation(individual p[])
{
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		std::cout << "Gene " << i + 1 << ": ";
		printGene(p[i]);
	}
}

bool evaluateFitness(individual res)
{
	if (res.fitness >= DATA_SET_SIZE)
	{
		std::cout << "The result has been found ";
		printGene(res);
		std::cout << "With rulebase: \n";
		for (int i = 0; i != GENE_SIZE; ++i)
		{
			std::cout << res.gene[i];
			if (((i + 1) % (INPUT_DATA_SIZE + OUTPUT_DATA_SIZE)) == 0)
				std::cout << std::endl;
		}
		std::cout << std::endl;
		return 1;
	}
	return 0;
}

void readDataSet(char* filename, data data_set[])
{
	FILE *fp = fopen(filename, "r");
	char buff[255];
	for (int i = 0; fgets(buff, 255, (FILE*)fp); ++i)
	{
		for (int j = 0; j != INPUT_DATA_SIZE; ++j)
		{
			data_set[i].variables[j] = buff[j] - 48;
		}
		data_set[i].output = buff[INPUT_DATA_SIZE + OUTPUT_DATA_SIZE] - 48;
	}
	fclose(fp);
}

bool matchesRule(data d, rule r)
{
	for (int i = 0; i != INPUT_DATA_SIZE; ++i)
	{
		if (r.condition[i] != 2 && d.variables[i] != r.condition[i])
			return false;
	}
	return true;
}

bool matchesOutput(data d, rule r)
{
	if (d.output != r.output)
		return false;
	return true;
}

void fitness(individual *individual, data data_set[])
{
	//Generating rulebase
	int k = 0;
	rule rulebase[RULEBASE_SIZE];

	for (int i = 0; i != RULEBASE_SIZE; i++)
	{
		for (int j = 0; j != INPUT_DATA_SIZE; ++j)
		{
			rulebase[i].condition[j] = individual->gene[k++];
		}
		rulebase[i].output = individual->gene[k++];
	}

	for (int i = 0; i != DATA_SET_SIZE; ++i)
	{
		for (int j = 0; j != RULEBASE_SIZE; ++j)
		{
			if (matchesRule(data_set[i], rulebase[j]))
			{
				if (matchesOutput(data_set[i], rulebase[j]))
					++individual->fitness;
				break;
			}
		}
	}
}

int calculateFitness(individual p[], individual &mf, data data_set[])
{
	int meanFitness = 0;
	int sumFitness = 0;
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		p[i].fitness = 0;
		fitness(&p[i], data_set);
		sumFitness += p[i].fitness;
		if (p[i].fitness >= mf.fitness)
		{
			mf = p[i];
		}
	}
	return sumFitness / POPULATION_SIZE;
}

void transferPopulation(individual p[], individual o[])
{
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		p[i].fitness = o[i].fitness;
		for (int j = 0; j != GENE_SIZE; ++j)
		{
			p[i].gene[j] = o[i].gene[j];
		}
	}
}

void singlePointCrossover(individual p[], individual o[], float crossoverRate)
{
	for (int i = 0; i != POPULATION_SIZE - 1; ++i)
	{
		if ((rand() % 1000) <= crossoverRate * 1000)
		{
			individual temp;
			individual p1 = p[i];
			individual p2 = p[i + 1];

			int crossoverIndex = rand() % GENE_SIZE;

			for (int j = 0; j != crossoverIndex; ++j)
			{
				o[i].gene[j] = p1.gene[j];
			}
			for (int j = crossoverIndex; j != GENE_SIZE; ++j)
			{
				o[i].gene[j] = p2.gene[j];
			}
			for (int j = 0; j != crossoverIndex; ++j)
			{
				o[i + 1].gene[j] = p2.gene[j];
			}
			for (int j = crossoverIndex; j != GENE_SIZE; ++j)
			{
				o[i + 1].gene[j] = p1.gene[j];
			}
		}
	}
}

void mutatePopulation(individual p[], float mutationRate)
{
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		for (int j = 0; j != GENE_SIZE; ++j)
		{
			if ((rand() % 1000) <= mutationRate * 1000)
			{
				if (p[i].gene[j] == 2)
					p[i].gene[j] = 0;
				else if (p[i].gene[j] == 1)
					p[i].gene[j] = 2;
				else if (p[i].gene[j] == 0)
					p[i].gene[j] = 1;
			}
			if ((j + 1) % (INPUT_DATA_SIZE + OUTPUT_DATA_SIZE) == 0)
			{
				if (p[i].gene[j] == 2)
					p[i].gene[j] = rand() % 2;
			}


		}
	}
}

void generatePopulation(individual p[])
{
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		for (int j = 0; j != GENE_SIZE; ++j)
		{
			p[i].gene[j] = rand() % 3;
		}
	}
}

void tournamentSelection(individual p[], individual o[])
{
	individual p1;
	individual p2;
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		p1 = p[rand() % POPULATION_SIZE];
		p2 = p[rand() % POPULATION_SIZE];

		if (p1.fitness >= p2.fitness)
			o[i] = p1;
		else
			o[i] = p2;
	}
}

void rouletteWheelSelection(individual p[], individual o[])
{
	//Sum population fitness
	int totalFitness = 0;
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		totalFitness += p[i].fitness;
	}
	std::cout << "TotalFitness: " << totalFitness << std::endl;

	//Roulette Wheel Selection
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		int selection_point = rand() % totalFitness;
		int running_total = 0;
		int j = 0;
		while (running_total <= selection_point)
		{
			running_total += p[j].fitness;
			++j;
		}
		o[i] = p[j - 1];
	}
}

int main() {
	individual mostFit;
	int meanFit;
	srand((unsigned)time(NULL));

	data data_set[DATA_SET_SIZE];
	readDataSet("t1.txt", data_set);

	//INITALISE
	generatePopulation(population);
	meanFit = calculateFitness(population, mostFit, data_set);

	//PRINT INITIAL GENERATION
	printPopulation(population);
	std::cout << "Most fit of this population = ";
	printGene(mostFit);
	std::cout << "Mean fitness = " << meanFit << std::endl << std::endl;

	//EVALUATE
	int generation = 0;
	while (!(evaluateFitness(mostFit) || generation == NUMBER_OF_GENERATIONS))
	{
		//Dynamic Mutation Approach
		//mutation_rate = deterministicMutation(generation);

		//SELECT
		//rouletteWheelSelection(population, offspring);
		//transferPopulation(population, offspring);
		tournamentSelection(population, offspring);
		transferPopulation(population, offspring);

		//CROSSOVER/RECOMBINATION
		singlePointCrossover(population, offspring, crossover_rate);
		meanFit = calculateFitness(offspring, mostFit, data_set);
		transferPopulation(population, offspring);

		//MUTATE
		mutatePopulation(population, mutation_rate);
		meanFit = calculateFitness(population, mostFit, data_set);

		//PRINT NEW GENERATION
		//std::cout << "Offspring Population" << std::endl;
		//printPopulation(offspring);
		std::cout << "Most fit of this population = \n";
		printGene(mostFit);
		std::cout << "Mean fitness = " << meanFit << std::endl << std::endl;

		++generation;
		std::cout << "Generation: " << generation << std::endl;

		if (LOG_FILE)
		{
			char snum[20];
			snprintf(snum, sizeof(snum), "performance.csv");
			FILE *fp = fopen(snum, "a");
			_itoa(DATA_SET_SIZE - mostFit.fitness, snum, 10);
			fputs(snum, fp);
			fputc(',', fp);
			_itoa(DATA_SET_SIZE - meanFit, snum, 10);
			fputs(snum, fp);
			fputc('\n', fp);
			fclose(fp);
		}
	}

	std::cout << "Enter test data to see rulebase predicted output." << std::endl;

	while (1)
		testIndividual(mostFit);

	int halt;
	std::cin >> halt;

	return 0;
}