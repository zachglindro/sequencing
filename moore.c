#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int id;
    int length;
    int weight;
    int deadline;
    bool is_in_S;
} Task;

void print_schedule(Task tasks[], int schedule[], int n) {
    printf("Schedule: ");
    for (int i = 0; i < n; i++) {
        printf("%d%s", schedule[i], (i == n - 1) ? "" : " -> ");
    }
    printf("\n");
    
    // Print detailed information about the schedule
    int current_time = 0;
    int total_tardy_weight = 0;
    int s_on_time_count = 0;
    
    printf("\nTask execution details:\n");
    printf("%-5s %-8s %-10s %-10s %-10s %-10s %-10s\n", 
           "ID", "Length", "Weight", "Deadline", "Start", "Finish", "Status");
    
    for (int i = 0; i < n; i++) {
        int task_index = schedule[i];
        Task current_task = tasks[task_index];
        
        int start_time = current_time;
        int completion_time = current_time + current_task.length;
        current_time = completion_time;
        
        const char* status;
        if (completion_time <= current_task.deadline) {
            status = current_task.is_in_S ? "On-time (S)" : "On-time";
            if (current_task.is_in_S) {
                s_on_time_count++;
            }
        } else {
            status = "Tardy";
            total_tardy_weight += current_task.weight;
        }
        
        printf("%-5d %-8d %-10d %-10d %-10d %-10d %-10s\n", 
               current_task.id, current_task.length, current_task.weight, 
               current_task.deadline, start_time, completion_time, status);
    }
    
    printf("\nSummary:\n");
    printf("Total tardy weight: %d\n", total_tardy_weight);
    printf("Number of S tasks completed on time: %d\n", s_on_time_count);
}

/**
 * Moore's Algorithm with Extensions
 * The original Moore's algorithm aims to minimize the number of tardy jobs.
 * This implementation adds:
 * 1. A weight constraint (total weight of tardy jobs must not exceed K)
 * 2. Prioritization of S tasks (trying to maximize on-time S tasks)
 */
int* moores_algorithm(Task tasks[], int n, int K, int *max_s_on_time) {
    // Step 1: Sort tasks by deadline (EDD - Earliest Due Date)
    Task** sorted_tasks = (Task**)malloc(n * sizeof(Task*));
    for (int i = 0; i < n; i++) {
        sorted_tasks[i] = &tasks[i];
    }
    
    // Merge sort by deadline - more efficient than bubble sort with O(n log n) complexity
    void merge_sort_by_deadline(Task** arr, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            
            // Sort first and second halves
            merge_sort_by_deadline(arr, left, mid);
            merge_sort_by_deadline(arr, mid + 1, right);
            
            // Merge the sorted halves
            merge(arr, left, mid, right);
        }
    }
    
    void merge(Task** arr, int left, int mid, int right) {
        int i, j, k;
        int n1 = mid - left + 1;
        int n2 = right - mid;
        
        // Create temporary arrays
        Task** L = (Task**)malloc(n1 * sizeof(Task*));
        Task** R = (Task**)malloc(n2 * sizeof(Task*));
        
        // Copy data to temporary arrays
        for (i = 0; i < n1; i++)
            L[i] = arr[left + i];
        for (j = 0; j < n2; j++)
            R[j] = arr[mid + 1 + j];
        
        // Merge the temporary arrays back into arr[left..right]
        i = 0;
        j = 0;
        k = left;
        while (i < n1 && j < n2) {
            if (L[i]->deadline <= R[j]->deadline) {
                arr[k] = L[i];
                i++;
            } else {
                arr[k] = R[j];
                j++;
            }
            k++;
        }
        
        // Copy the remaining elements of L[], if there are any
        while (i < n1) {
            arr[k] = L[i];
            i++;
            k++;
        }
        
        // Copy the remaining elements of R[], if there are any
        while (j < n2) {
            arr[k] = R[j];
            j++;
            k++;
        }
        
        // Free temporary arrays
        free(L);
        free(R);
    }
    
    // Call merge sort
    merge_sort_by_deadline(sorted_tasks, 0, n-1);
    
    // Initial schedule based on EDD
    int* schedule = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        schedule[i] = sorted_tasks[i]->id;
    }
    
    // Best schedule found so far
    int* best_schedule = (int*)malloc(n * sizeof(int));
    int best_s_on_time = -1;
    int best_tardy_weight = K + 1; // Initialize as invalid
    
    // Step 2: Apply Moore's algorithm with weight constraint
    bool found_improvement = true;
    while (found_improvement) {
        found_improvement = false;
        
        // Evaluate current schedule
        int current_time = 0;
        int total_tardy_weight = 0;
        int s_on_time_count = 0;
        int* tardy_tasks = (int*)calloc(n, sizeof(int)); // Mark which tasks are tardy
        int tardy_count = 0;
        
        for (int i = 0; i < n; i++) {
            int task_id = schedule[i];
            Task* task = &tasks[task_id];
            
            int completion_time = current_time + task->length;
            current_time = completion_time;
            
            if (completion_time > task->deadline) {
                // Task is tardy
                total_tardy_weight += task->weight;
                tardy_tasks[i] = 1;
                tardy_count++;
            } else if (task->is_in_S) {
                // Task is on time and is an S task
                s_on_time_count++;
            }
        }
        
        // Check if current schedule is valid and better than previous best
        if (total_tardy_weight <= K && 
           (s_on_time_count > best_s_on_time || 
           (s_on_time_count == best_s_on_time && total_tardy_weight < best_tardy_weight))) {
            memcpy(best_schedule, schedule, n * sizeof(int));
            best_s_on_time = s_on_time_count;
            best_tardy_weight = total_tardy_weight;
        }
        
        // If total weight exceeds K, look for improvements
        if (total_tardy_weight > K && tardy_count > 0) {
            // Find the task with maximum weight among tardy tasks
            int max_weight_idx = -1;
            int max_weight = -1;
            
            for (int i = 0; i < n; i++) {
                if (tardy_tasks[i]) {
                    int task_id = schedule[i];
                    if (tasks[task_id].weight > max_weight) {
                        max_weight = tasks[task_id].weight;
                        max_weight_idx = i;
                    }
                }
            }
            
            // Move the heaviest tardy task to the end
            if (max_weight_idx != -1) {
                int heavy_task = schedule[max_weight_idx];
                for (int i = max_weight_idx; i < n - 1; i++) {
                    schedule[i] = schedule[i + 1];
                }
                schedule[n - 1] = heavy_task;
                found_improvement = true;
            }
        } else {
            // Try swapping tasks to increase S on-time tasks
            // This is an extension to Moore's algorithm
            bool swapped = false;
            
            // Look for a non-S on-time task that can be swapped with an S tardy task
            for (int i = 0; i < n && !swapped; i++) {
                int task_i_id = schedule[i];
                Task* task_i = &tasks[task_i_id];
                
                // If task_i is on-time but not an S task
                if (!tardy_tasks[i] && !task_i->is_in_S) {
                    for (int j = 0; j < n && !swapped; j++) {
                        int task_j_id = schedule[j];
                        Task* task_j = &tasks[task_j_id];
                        
                        // If task_j is tardy and is an S task
                        if (tardy_tasks[j] && task_j->is_in_S) {
                            // Try swapping and see if it improves the solution
                            int temp = schedule[i];
                            schedule[i] = schedule[j];
                            schedule[j] = temp;
                            
                            // Re-evaluate the schedule after swap
                            int new_time = 0;
                            int new_tardy_weight = 0;
                            int new_s_on_time = 0;
                            
                            for (int k = 0; k < n; k++) {
                                int task_id = schedule[k];
                                Task* task = &tasks[task_id];
                                
                                int completion_time = new_time + task->length;
                                new_time = completion_time;
                                
                                if (completion_time > task->deadline) {
                                    new_tardy_weight += task->weight;
                                } else if (task->is_in_S) {
                                    new_s_on_time++;
                                }
                            }
                            
                            // Check if swap is beneficial
                            if (new_tardy_weight <= K && 
                                (new_s_on_time > best_s_on_time || 
                                (new_s_on_time == best_s_on_time && new_tardy_weight < best_tardy_weight))) {
                                memcpy(best_schedule, schedule, n * sizeof(int));
                                best_s_on_time = new_s_on_time;
                                best_tardy_weight = new_tardy_weight;
                                swapped = true;
                                found_improvement = true;
                            } else {
                                // Swap back
                                temp = schedule[i];
                                schedule[i] = schedule[j];
                                schedule[j] = temp;
                            }
                        }
                    }
                }
            }
        }
        
        free(tardy_tasks);
        
        // If no improvement found after trying all options, break
        if (!found_improvement) {
            break;
        }
    }
    
    // Free allocated memory
    free(sorted_tasks);
    free(schedule);
    
    // Set output parameter
    *max_s_on_time = best_s_on_time;
    
    return best_schedule;
}

int main(int argc, char *argv[]) {
    // Check if filename is provided
    if (argc != 2) {
        printf("Usage: %s <path>\n", argv[0]);
        return 1;
    }
    
    // Read the input file
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", argv[1]);
        return 1;
    }
    
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
    
    // Record time for performance analysis
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    
    int max_s_on_time = -1;
    int *optimal_schedule = moores_algorithm(tasks, n, K, &max_s_on_time);
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    if (max_s_on_time == -1) {
        printf("No valid schedule found\n");
    } else {
        printf("Solution found. Number of S tasks completed: %d\n", max_s_on_time);
        printf("Optimal Schedule: ");
        for (int i = 0; i < n; i++) {
            printf("%d%s", optimal_schedule[i], (i == n - 1) ? "" : " -> ");
        }
        printf("\n");
        
        // For printing detailed schedule information
        print_schedule(tasks, optimal_schedule, n);
    }
    
    printf("\nExecution time: %f seconds\n", cpu_time_used);
    
    free(tasks);
    free(optimal_schedule);
    
    return 0;
}
