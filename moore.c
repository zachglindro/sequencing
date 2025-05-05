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
    // printf("Schedule: ");
    // for (int i = 0; i < n; i++) {
    //     printf("%d%s", schedule[i], (i == n - 1) ? "" : " -> ");
    // }
    // printf("\n");
    
    // Print detailed information about the schedule
    int current_time = 0;
    int total_tardy_weight = 0;
    int s_on_time_count = 0;
    
    // printf("\nTask execution details:\n");
    // printf("%-5s %-8s %-10s %-10s %-10s %-10s %-10s\n", 
    //        "ID", "Length", "Weight", "Deadline", "Start", "Finish", "Status");
    
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
        
        // printf("%-5d %-8d %-10d %-10d %-10d %-10d %-10s\n", 
        //        current_task.id, current_task.length, current_task.weight, 
        //        current_task.deadline, start_time, completion_time, status);
    }
    
    // printf("\nSummary:\n");
    // printf("Total tardy weight: %d\n", total_tardy_weight);
    // printf("Number of S tasks completed on time: %d\n", s_on_time_count);
}

// Forward declaration of merge function
void merge(Task** arr, int left, int mid, int right);

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

/**
 * Improved Moore's Algorithm with Extensions
 * This algorithm aims to:
 * 1. Maximize the number of on-time S tasks
 * 2. Keep the total weight of tardy jobs under K
 */
int* improved_moores_algorithm(Task tasks[], int n, int K, int *max_s_on_time) {
    // Create array of pointers for easier sorting and manipulation
    Task** task_ptrs = (Task**)malloc(n * sizeof(Task*));
    for (int i = 0; i < n; i++) {
        task_ptrs[i] = &tasks[i];
    }
    
    // Sort tasks by deadline (EDD - Earliest Due Date)
    merge_sort_by_deadline(task_ptrs, 0, n-1);
    
    // Initialize best solution tracking
    int* best_schedule = (int*)malloc(n * sizeof(int));
    int best_s_on_time = -1;
    int best_tardy_weight = K + 1; // Initialize as invalid
    
    // Try different initial schedules
    // Strategy 1: Standard EDD (Earliest Due Date)
    int* current_schedule = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        current_schedule[i] = task_ptrs[i]->id;
    }
    
    // Apply classic Moore's algorithm with our extensions
    // Start with EDD order
    int* edd_schedule = (int*)malloc(n * sizeof(int));
    memcpy(edd_schedule, current_schedule, n * sizeof(int));
    
    // Simulate execution
    bool* in_schedule = (bool*)calloc(n, sizeof(bool));
    for (int i = 0; i < n; i++) {
        in_schedule[i] = true;
    }
    
    // Try to find a schedule that completes tasks on time
    int current_time = 0;
    for (int i = 0; i < n; i++) {
        int task_id = current_schedule[i];
        Task* task = &tasks[task_id];
        
        current_time += task->length;
        
        // If we're late for this task
        if (current_time > task->deadline) {
            // Find the longest task in our current schedule
            int max_length_idx = i;
            int max_length = task->length;
            
            for (int j = 0; j < i; j++) {
                int prev_id = current_schedule[j];
                if (tasks[prev_id].length > max_length) {
                    max_length = tasks[prev_id].length;
                    max_length_idx = j;
                }
            }
            
            // Remove the longest task (make it tardy)
            current_time -= tasks[current_schedule[max_length_idx]].length;
            in_schedule[max_length_idx] = false;
        }
    }
    
    // Build the actual schedule with on-time tasks first
    int idx = 0;
    for (int i = 0; i < n; i++) {
        if (in_schedule[i]) {
            edd_schedule[idx++] = current_schedule[i];
        }
    }
    
    // Then add tardy tasks
    for (int i = 0; i < n; i++) {
        if (!in_schedule[i]) {
            edd_schedule[idx++] = current_schedule[i];
        }
    }
    
    // Evaluate the EDD-based schedule
    int s_on_time = 0;
    int total_tardy_weight = 0;
    current_time = 0;
    
    for (int i = 0; i < n; i++) {
        int task_id = edd_schedule[i];
        Task* task = &tasks[task_id];
        
        int completion_time = current_time + task->length;
        current_time = completion_time;
        
        if (completion_time > task->deadline) {
            total_tardy_weight += task->weight;
        } else if (task->is_in_S) {
            s_on_time++;
        }
    }
    
    if (total_tardy_weight <= K && s_on_time > best_s_on_time) {
        best_s_on_time = s_on_time;
        best_tardy_weight = total_tardy_weight;
        memcpy(best_schedule, edd_schedule, n * sizeof(int));
    }
    
    // Strategy 2: Prioritize S tasks
    // Sort by S status first (S tasks come first), then by deadline
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if ((!task_ptrs[i]->is_in_S && task_ptrs[j]->is_in_S) ||
                (task_ptrs[i]->is_in_S == task_ptrs[j]->is_in_S && 
                 task_ptrs[i]->deadline > task_ptrs[j]->deadline)) {
                Task* temp = task_ptrs[i];
                task_ptrs[i] = task_ptrs[j];
                task_ptrs[j] = temp;
            }
        }
    }
    
    // Create schedule based on S-priority
    for (int i = 0; i < n; i++) {
        current_schedule[i] = task_ptrs[i]->id;
    }
    
    // Apply Moore's algorithm with S-priority
    int* s_priority_schedule = (int*)malloc(n * sizeof(int));
    memcpy(s_priority_schedule, current_schedule, n * sizeof(int));
    
    // Reset in_schedule array
    for (int i = 0; i < n; i++) {
        in_schedule[i] = true;
    }
    
    // Try to find a schedule that completes tasks on time
    current_time = 0;
    for (int i = 0; i < n; i++) {
        int task_id = current_schedule[i];
        Task* task = &tasks[task_id];
        
        current_time += task->length;
        
        // If we're late for this task
        if (current_time > task->deadline) {
            // In this case, we prioritize removing non-S tasks or the longest S task
            int to_remove_idx = -1;
            int max_length = -1;
            
            // First try to find a non-S task to remove
            for (int j = 0; j <= i; j++) {
                int prev_id = current_schedule[j];
                if (in_schedule[j] && !tasks[prev_id].is_in_S) {
                    if (tasks[prev_id].length > max_length) {
                        max_length = tasks[prev_id].length;
                        to_remove_idx = j;
                    }
                }
            }
            
            // If no non-S task found, remove the longest S task
            if (to_remove_idx == -1) {
                for (int j = 0; j <= i; j++) {
                    int prev_id = current_schedule[j];
                    if (in_schedule[j] && tasks[prev_id].length > max_length) {
                        max_length = tasks[prev_id].length;
                        to_remove_idx = j;
                    }
                }
            }
            
            // Remove the selected task
            if (to_remove_idx != -1) {
                current_time -= tasks[current_schedule[to_remove_idx]].length;
                in_schedule[to_remove_idx] = false;
            }
        }
    }
    
    // Build the actual schedule with on-time tasks first
    idx = 0;
    for (int i = 0; i < n; i++) {
        if (in_schedule[i]) {
            s_priority_schedule[idx++] = current_schedule[i];
        }
    }
    
    // Then add tardy tasks, prioritizing low weights
    // First sort remaining tasks by weight
    int tardy_count = n - idx;
    Task** tardy_tasks = (Task**)malloc(tardy_count * sizeof(Task*));
    int tardy_idx = 0;
    
    for (int i = 0; i < n; i++) {
        if (!in_schedule[i]) {
            tardy_tasks[tardy_idx++] = &tasks[current_schedule[i]];
        }
    }
    
    // Sort tardy tasks by weight (ascending)
    for (int i = 0; i < tardy_count; i++) {
        for (int j = i + 1; j < tardy_count; j++) {
            if (tardy_tasks[i]->weight > tardy_tasks[j]->weight) {
                Task* temp = tardy_tasks[i];
                tardy_tasks[i] = tardy_tasks[j];
                tardy_tasks[j] = temp;
            }
        }
    }
    
    // Add tardy tasks to schedule
    for (int i = 0; i < tardy_count; i++) {
        s_priority_schedule[idx++] = tardy_tasks[i]->id;
    }
    
    // Evaluate the S-priority schedule
    s_on_time = 0;
    total_tardy_weight = 0;
    current_time = 0;
    
    for (int i = 0; i < n; i++) {
        int task_id = s_priority_schedule[i];
        Task* task = &tasks[task_id];
        
        int completion_time = current_time + task->length;
        current_time = completion_time;
        
        if (completion_time > task->deadline) {
            total_tardy_weight += task->weight;
        } else if (task->is_in_S) {
            s_on_time++;
        }
    }
    
    if (total_tardy_weight <= K && 
        (s_on_time > best_s_on_time || 
         (s_on_time == best_s_on_time && total_tardy_weight < best_tardy_weight))) {
        best_s_on_time = s_on_time;
        best_tardy_weight = total_tardy_weight;
        memcpy(best_schedule, s_priority_schedule, n * sizeof(int));
    }
    
    // Strategy 3: Weight-based ordering for K constraint
    // Sort by weight/length ratio (WSPT - Weighted Shortest Processing Time)
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double ratio_i = (double)task_ptrs[i]->weight / task_ptrs[i]->length;
            double ratio_j = (double)task_ptrs[j]->weight / task_ptrs[j]->length;
            if (ratio_i < ratio_j) {
                Task* temp = task_ptrs[i];
                task_ptrs[i] = task_ptrs[j];
                task_ptrs[j] = temp;
            }
        }
    }
    
    // Create schedule based on WSPT
    for (int i = 0; i < n; i++) {
        current_schedule[i] = task_ptrs[i]->id;
    }
    
    // Apply Moore's algorithm with WSPT
    int* wspt_schedule = (int*)malloc(n * sizeof(int));
    memcpy(wspt_schedule, current_schedule, n * sizeof(int));
    
    // Reset in_schedule array
    for (int i = 0; i < n; i++) {
        in_schedule[i] = true;
    }
    
    // Try to find a schedule that keeps total tardy weight under K
    current_time = 0;
    for (int i = 0; i < n; i++) {
        int task_id = current_schedule[i];
        Task* task = &tasks[task_id];
        
        current_time += task->length;
        
        // If we're late for this task
        if (current_time > task->deadline) {
            // Consider the weight when deciding what to remove
            int to_remove_idx = i;
            int min_weight_loss = task->weight;
            
            for (int j = 0; j < i; j++) {
                int prev_id = current_schedule[j];
                if (in_schedule[j] && tasks[prev_id].weight < min_weight_loss) {
                    min_weight_loss = tasks[prev_id].weight;
                    to_remove_idx = j;
                }
            }
            
            // Remove the selected task
            current_time -= tasks[current_schedule[to_remove_idx]].length;
            in_schedule[to_remove_idx] = false;
        }
    }
    
    // Build the actual schedule with on-time tasks first
    idx = 0;
    for (int i = 0; i < n; i++) {
        if (in_schedule[i]) {
            wspt_schedule[idx++] = current_schedule[i];
        }
    }
    
    // Then add tardy tasks
    for (int i = 0; i < n; i++) {
        if (!in_schedule[i]) {
            wspt_schedule[idx++] = current_schedule[i];
        }
    }
    
    // Evaluate the WSPT schedule
    s_on_time = 0;
    total_tardy_weight = 0;
    current_time = 0;
    
    for (int i = 0; i < n; i++) {
        int task_id = wspt_schedule[i];
        Task* task = &tasks[task_id];
        
        int completion_time = current_time + task->length;
        current_time = completion_time;
        
        if (completion_time > task->deadline) {
            total_tardy_weight += task->weight;
        } else if (task->is_in_S) {
            s_on_time++;
        }
    }
    
    if (total_tardy_weight <= K && 
        (s_on_time > best_s_on_time || 
         (s_on_time == best_s_on_time && total_tardy_weight < best_tardy_weight))) {
        best_s_on_time = s_on_time;
        best_tardy_weight = total_tardy_weight;
        memcpy(best_schedule, wspt_schedule, n * sizeof(int));
    }
    
    // Free allocated memory
    free(task_ptrs);
    free(current_schedule);
    free(edd_schedule);
    free(s_priority_schedule);
    free(wspt_schedule);
    free(in_schedule);
    free(tardy_tasks);
    
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
    int *optimal_schedule = improved_moores_algorithm(tasks, n, K, &max_s_on_time);
    
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
    
    // printf("\nExecution time: %f seconds\n", cpu_time_used);
    
    free(tasks);
    free(optimal_schedule);
    
    return 0;
}