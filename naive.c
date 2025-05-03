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

int max_s_tasks_completed_on_time = -1;
int *optimal_permutation;

// tasks = task list
// p = task order (e.g. [2,5,1,4,3])
// n = number of tasks
// K = tardy weight limit
void evaluate_schedule(Task tasks[], int p[], int n, int K)
{
	int current_time = 0;
	int total_tardy_weight = 0;
	int s_tasks_completed_on_time = 0;

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
				s_tasks_completed_on_time++;
			}
		}
	}

	if (total_tardy_weight <= K &&
	    s_tasks_completed_on_time > max_s_tasks_completed_on_time) {
		max_s_tasks_completed_on_time = s_tasks_completed_on_time;
		memcpy(optimal_permutation, p, n * sizeof(int));
	}
}

// tasks = task list
// p = task order (e.g. [2,5,1,4,3])
// n = number of tasks
// K = tardy weight limit
void generate_permutations(Task tasks[], int p[], int k, int n, int K)
{
	if (k == n) {
		evaluate_schedule(tasks, p, n, K);
	}

	for (int i = k; i < n; i++) {
		swap(&p[k], &p[i]);
		generate_permutations(tasks, p, k + 1, n, K);
		swap(&p[k], &p[i]);
	}
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
	int *permutation = (int *)malloc(n * sizeof(int));
	for (int i = 0; i < n; i++) {
		permutation[i] = i;
	}

	generate_permutations(tasks, permutation, 0, n, K);

	if (max_s_tasks_completed_on_time == -1) {
		printf("No valid schedule found\n");
	} else {
		printf("Solution found. Number of S tasks completed: %d\n",
		       max_s_tasks_completed_on_time);
		printf("Optimal Schedule: ");
		for (int i = 0; i < n; i++) {
			printf("%d%s", optimal_permutation[i],
			       (i == n - 1) ? "" : " -> ");
		}
		printf("\n");
	}

	free(tasks);
	free(permutation);
	free(optimal_permutation);
}