#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <random>

#define LOG_FILE 1

#define POPULATION_SIZE 1000
#define RULEBASE_SIZE 10
#define INPUT_DATA_SIZE 6
#define OUTPUT_DATA_SIZE 1
#define GENE_SIZE RULEBASE_SIZE * (INPUT_DATA_SIZE*2 + OUTPUT_DATA_SIZE)
#define DATA_SET_SIZE 2000
#define NUMBER_OF_GENERATIONS 250
#define K_FOLD 5
#define TRAINING_DATA_SIZE (DATA_SET_SIZE / K_FOLD) * (K_FOLD-1)
#define VALIDATION_DATA_SIZE (DATA_SET_SIZE / K_FOLD)

float mutation_rate = 1 / (float)(RULEBASE_SIZE * (INPUT_DATA_SIZE + OUTPUT_DATA_SIZE));
float crossover_rate = 0.01;
float creep_range = 0.08;
float bound_range = 0.1;

struct individual
{
	float gene[GENE_SIZE];
	int fitness = 0;
	int validationScore = 0;
};

individual population[POPULATION_SIZE];
individual offspring[POPULATION_SIZE];

struct rule
{
	float condition[INPUT_DATA_SIZE * 2];
	int output;
};

struct data
{
	float variables[INPUT_DATA_SIZE];
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

//Generate random float between lower bound a and upper b.
float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

bool isOutput(int index)
{
	if ((index + 1) % (INPUT_DATA_SIZE*2 + OUTPUT_DATA_SIZE) == 0)
		return true;
	return false;
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
			std::cout << "Rulebase predicted output: " << ind.gene[i + INPUT_DATA_SIZE] << '\n';
			break;
		}
	}
}

void printGene(individual ind)
{
	for (int i = 0; i != GENE_SIZE; ++i)
	{
		if (isOutput(i))
			std::cout << "Output: " << ind.gene[i] << '\n';
		else
			std::cout << ind.gene[i] << '\n';
	}
	std::cout << " With fitness value " << ind.fitness << '\n';
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
		int c;
		std::cin >> c;
		/*for (int i = 0; i != GENE_SIZE; ++i)
		{
		std::cout << res.gene[i];
		if (((i + 1) % (INPUT_DATA_SIZE + OUTPUT_DATA_SIZE)) == 0)
		std::cout << '\n';
		}
		std::cout << '\n';*/
		return 1;
	}
	return 0;
}

//Read data file of floating points and binary output.
void readDataSet(char* filename, data data_set[])
{
	FILE *fp = fopen(filename, "r");
	char buff[99999];
	for (int i = 0; fgets(buff, 255, (FILE*)fp); ++i)
	{
		for (int j = 0; j != INPUT_DATA_SIZE; ++j)
		{
			int c = j * 9;
			++c; ++c;
			float sum = 0.0;
			int counter = 10;
			while (buff[c] != ' ')
			{
				sum += (buff[c] - 48) / (float)counter;
				counter = counter * 10;
				c++;
			}
			data_set[i].variables[j] = sum;
		}
		data_set[i].output = buff[54] - 48;
	}
	fclose(fp);
}

bool matchesRule(data d, rule r)
{
	int k = 0;
	for (int i = 0; i != INPUT_DATA_SIZE; ++i)
	{
		if (r.condition[k] != 2 && r.condition[k + 1] != 2)
		{
			if (r.condition[k] < r.condition[k + 1])
			{
				if (d.variables[i] > r.condition[k + 1] || d.variables[i] < r.condition[k])
					return false;
			}
			else if (r.condition[k] > r.condition[k + 1])
			{
				if (d.variables[i] > r.condition[k] || d.variables[i] < r.condition[k + 1])
					return false;
			}
		}
		k += 2;
	}
	return true;
}

bool matchesOutput(data d, rule r)
{
	if (d.output != r.output)
		return false;
	return true;
}

void fitness(individual *individual, data data_set[], int currentK)
{
	int k = 0;
	rule rulebase[RULEBASE_SIZE];

	for (int i = 0; i != RULEBASE_SIZE; i++)
	{
		for (int j = 0; j != INPUT_DATA_SIZE*2; ++j)
		{
			rulebase[i].condition[j] = individual->gene[k++];
		}
		rulebase[i].output = individual->gene[k++];
	}

	int kfoldBegin = (VALIDATION_DATA_SIZE)* currentK;
	int kfoldEnd = (VALIDATION_DATA_SIZE) * (currentK + 1);
	for (int i = 0; i != DATA_SET_SIZE; ++i)
	{
		for (int j = 0; j != RULEBASE_SIZE; ++j)
		{
			if (matchesRule(data_set[i], rulebase[j]))
			{
				if (matchesOutput(data_set[i], rulebase[j]))
					if (LOG_FILE && (i >= kfoldBegin && i < kfoldEnd))
						++individual->validationScore;
					else
						++individual->fitness;
				break;
			}
		}
	}
}

void calculateFitness(individual p[], individual &mostf, individual &meanf, data data_set[], int currentK)
{
	int meanFitness = 0;
	int sumFitness = 0;
	int sumValidation = 0;
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		p[i].fitness = 0;
		p[i].validationScore = 0;
		fitness(&p[i], data_set, currentK);
		//printGene(p[i]);
		sumFitness += p[i].fitness;
		sumValidation += p[i].validationScore;
		if (p[i].fitness >= mostf.fitness)
		{
			mostf = p[i];
		}
	}
	meanf.fitness = sumFitness / POPULATION_SIZE;
	meanf.validationScore = sumValidation / POPULATION_SIZE;
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
		if ((rand() % 100) < crossoverRate * 100)
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

void intermediateCrossover(individual p[], individual o[], float crossoverRate)
{
	for (int i = 0; i != POPULATION_SIZE - 1; ++i)
	{
		if ((rand() % 1000) < crossoverRate * 1000)
		{
			individual temp;
			individual p1 = p[i];
			individual p2 = p[i + 1];

			int crossoverIndex = rand() % GENE_SIZE;

			for (int j = crossoverIndex; j != GENE_SIZE; ++j)
			{
				 o[i].gene[j] = (p1.gene[j] + p2.gene[j]) / 2;
				 o[i + 1].gene[j] = (p1.gene[j] + p2.gene[j]) / 2;
			}
		}
	}
}

void gaussianMutation(individual p[], float mutationRate)
{
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		for (int j = 0; j != GENE_SIZE; ++j)
		{
			if (isOutput(j))
			{
				if ((rand() % 10000) < mutationRate * 10000)
					p[i].gene[j] = rand() % 2;
			}
			else if ((rand() % 10000) < mutationRate * 10000)
			{
				//Non-Uniform Creep Mutation (Normal distribution creep)
				std::random_device rd;
				std::mt19937 gen(rd());
				std::normal_distribution<> d(0, 0.15);
				float te = d(gen);
				p[i].gene[j] += d(gen);

				//Random value between bounds creep
				/*int creepDirection = rand() % 100;
				if (creepDirection >= 50)
				{
				p[i].gene[j] += RandomFloat(0.0, creep_range);
				}
				else
				{
				p[i].gene[j] -= RandomFloat(0.0, creep_range);
				}*/
				//If the creep takes out of bounds. Just get new random value in range.
				//if (p[i].gene[j] < 0 || p[i].gene[j] > 1.0)
				//{
				//	p[i].gene[j] = RandomFloat(0.0, 1.0);
				//}
				if (p[i].gene[j] < 0)
				{
					p[i].gene[j] = 0;
				}
				else if (p[i].gene[j] > 1)
				{
					p[i].gene[j] = 1;
				}
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
			if (isOutput(j))
			{
				if ((rand() % 10000) < mutationRate * 10000)
					p[i].gene[j] = rand() % 2;
			}
			else if ((rand() % 10000) < mutationRate * 10000)
			{
				/*if ((rand() % 100) < 10)
					p[i].gene[j] = 2.0;
				else
				{*/
					//Uniform creep mutation?
					p[i].gene[j] = RandomFloat(0.0, 1.0);
				//}
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
			if (isOutput(j))
			{
				p[i].gene[j] = rand() % 2;
			}
			else
			{
				p[i].gene[j] = RandomFloat(0.0, 1.0);
			}
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
	int totalFitness = 0;
	for (int i = 0; i != POPULATION_SIZE; ++i)
	{
		totalFitness += p[i].fitness;
	}

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
	individual meanFit;
	srand((unsigned)time(NULL));

	data data_set[DATA_SET_SIZE];
	readDataSet("t3.txt", data_set);

	for (int i = 0; i != K_FOLD; ++i)
	{

		//INITALISE
		generatePopulation(population);
		calculateFitness(population, mostFit, meanFit, data_set, i);

		//PRINT INITIAL GENERATION
		printPopulation(population);
		std::cout << "Most fit of this population = ";
		printGene(mostFit);
		std::cout << "Mean fitness = " << meanFit.fitness << '\n' << '\n';

		//EVALUATE
		int generation = 0;
		while (!(evaluateFitness(mostFit) || generation == NUMBER_OF_GENERATIONS))
		{
			//Dynamic Mutation Approach
			//mutation_rate = deterministicMutation(generation);

			individual mostFit;
			individual meanFit;
			//SELECT
			//rouletteWheelSelection(population, offspring);
			//transferPopulation(population, offspring);
			tournamentSelection(population, offspring);
			transferPopulation(population, offspring);

			//CROSSOVER/RECOMBINATION
			//singlePointCrossover(population, offspring, crossover_rate);
			intermediateCrossover(population, offspring, crossover_rate);
			calculateFitness(offspring, mostFit, meanFit, data_set, i);
			transferPopulation(population, offspring);

			//MUTATE
			gaussianMutation(population, mutation_rate);
			//mutatePopulation(population, mutation_rate);
			calculateFitness(population, mostFit, meanFit, data_set, i);

			//PRINT NEW GENERATION
			//std::cout << "Offspring Population" << '\n';
			//printPopulation(offspring);
			if ((generation % 100) == 0)
			{
				std::cout << "Most fit of this population = ";
				printGene(mostFit);
				std::cout << "Mean fitness = " << meanFit.fitness << '\n' << '\n';

				std::cout << "Generation: " << generation << '\n';
			}
			++generation;

			if (LOG_FILE)
			{
				char snum[11];
				snprintf(snum, sizeof(snum), "kfold%d.csv", i);
				FILE *fp = fopen(snum, "a");
				_itoa(TRAINING_DATA_SIZE - mostFit.fitness, snum, 10);
				fputs(snum, fp);
				fputc(',', fp);
				_itoa(VALIDATION_DATA_SIZE - mostFit.validationScore, snum, 10);
				fputs(snum, fp);
				fputc(',', fp);
				_itoa(TRAINING_DATA_SIZE - meanFit.fitness, snum, 10);
				fputs(snum, fp);
				fputc(',', fp);
				_itoa(VALIDATION_DATA_SIZE - meanFit.validationScore, snum, 10);
				fputs(snum, fp);
				fputc('\n', fp);
				fclose(fp);
			}
		}
	}

	std::cout << "Enter test data to see rulebase predicted output." << '\n';

	while (1)
		testIndividual(mostFit);

	int halt;
	std::cin >> halt;

	return 0;
}