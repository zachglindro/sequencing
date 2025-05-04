#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int id;
	int length;
	int weight;
	int deadline;
	bool is_in_S;
} Task;

void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

int max_s_on_time_count = -1;
int *optimal_permutation;

// tasks = task list
// p = task order (e.g. [2,5,1,4,3])
// n = number of tasks
// K = tardy weight limit
void evaluate_schedule(Task tasks[], int p[], int n, int K)
{
	int current_time = 0;
	int total_tardy_weight = 0;
	int s_on_time_count = 0;

	for (int i = 0; i < n; i++) {
		int task_index = p[i];
		Task current_task = tasks[task_index];

		int completion_time = current_time + current_task.length;
		current_time = completion_time;

		if (completion_time > current_task.deadline) {
			// Task is tardy
			total_tardy_weight += current_task.weight;
		} else {
			// Task completed on time
			if (current_task.is_in_S) {
				s_on_time_count++;
			}
		}
	}

	if (total_tardy_weight <= K && s_on_time_count > max_s_on_time_count) {
		max_s_on_time_count = s_on_time_count;
		memcpy(optimal_permutation, p, n * sizeof(int));
	}
}

// tasks = task list
// n = number of tasks
// K = tardy weight limit
void generate_permutations(Task tasks[], int n, int K)
{
	int start, move;
	int *nopts =
		(int *)malloc((n + 2) * sizeof(int)); // array of top of stacks
	int **option = (int **)malloc(
		(n + 2) * sizeof(int *)); // array of stacks of options

	for (int i = 0; i < n + 2; i++) {
		option[i] = (int *)malloc((n + 2) * sizeof(int));
	}

	int *current_perm = (int *)malloc(n * sizeof(int));

	move = start = 0;
	nopts[start] = 1;

	while (nopts[start] > 0) // while dummy stack is not empty
	{
		if (nopts[move] > 0) {
			move++;
			nopts[move] = 0; // initialize new move

			if (move == n + 1) // solution found!
			{
				for (int i = 1; i < move; i++) {
					current_perm[i - 1] =
						option[i][nopts[i]];
				}
				evaluate_schedule(tasks, current_perm, n, K);
			} else {
				for (int candidate = n; candidate >= 1;
				     candidate--) {
					int i;
					for (i = move - 1; i >= 1; i--)
						if (candidate - 1 ==
						    option[i][nopts[i]])
							break;
					if (!(i >= 1))
						option[move][++nopts[move]] =
							candidate - 1;
				}
			}
		} else {
			move--;
			nopts[move]--;
		}
	}

	free(current_perm);
	for (int i = 0; i < n + 2; i++) {
		free(option[i]);
	}
	free(option);
	free(nopts);
}

int main(int argc, char *argv[])
{
	// Check if filename is provided
	if (argc != 2) {
		printf("Usage: %s <path>\n", argv[0]);
		return 1;
	}

	// Read the input file
	FILE *file = fopen(argv[1], "r");

	int n; // Total number of tasks
	int K; // Tardy weight limit

	fscanf(file, "%d %d", &n, &K);

	Task *tasks = (Task *)malloc(n * sizeof(Task));
	for (int i = 0; i < n; i++) {
		fscanf(file, "%d %d %d %d", &tasks[i].length, &tasks[i].weight,
		       &tasks[i].deadline, &tasks[i].is_in_S);
		tasks[i].id = i;
	}

	fclose(file);

	optimal_permutation = (int *)malloc(n * sizeof(int));

	generate_permutations(tasks, n, K);

	if (max_s_on_time_count == -1) {
		printf("No valid schedule found\n");
	} else {
		printf("Solution found. Number of S tasks completed: %d\n",
		       max_s_on_time_count);
		printf("Optimal Schedule: ");
		for (int i = 0; i < n; i++) {
			printf("%d%s", optimal_permutation[i],
			       (i == n - 1) ? "" : " -> ");
		}
		printf("\n");
	}

	free(tasks);
	free(optimal_permutation);
}