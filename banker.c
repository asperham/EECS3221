// Colin Le Donne 215037955
// EECS3221-Z
// March/20/23
// Description: This code was implemented with a tasks strucutre that hold the information about tasks 
//				The ResouceManager structure is the Manager that holds and releases resources
//				I then have a funciton to read inputs and allocate the data to the correct data structures
//				This then goes through the two algorithms and is then formated and printed out to stdout
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TASKS 100
#define MAX_RESOURCES 100



//Tasks
typedef struct Task {
    unsigned int id;
    unsigned int time_taken;
    unsigned int waiting_time;
    unsigned int initial_claims[MAX_RESOURCES];
    unsigned int allocated[MAX_RESOURCES];
	unsigned int remaining[MAX_RESOURCES];
    bool terminated;
    bool aborted;
} Task;
//Resource Manager structure
typedef struct ResourceManager {
    unsigned int total_resources[MAX_RESOURCES];
    unsigned int available_resources[MAX_RESOURCES];
    Task tasks[MAX_TASKS];
    unsigned int num_tasks;
    unsigned int num_resources;
} ResourceManager;


//reads the input file and allocates data to according data structures
void read_input_file(const char *filename, ResourceManager *rm) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        exit(1);
    }

    fscanf(file, "%u %u", &rm->num_tasks, &rm->num_resources);

    for (unsigned int i = 0; i < rm->num_resources; ++i) {
        fscanf(file, "%u", &rm->total_resources[i]);
        rm->available_resources[i] = rm->total_resources[i];
    }
	for (unsigned int i = 0; i < rm->num_tasks; ++i) {
		for (unsigned int j = 0; j < rm->num_resources; ++j) {
			rm->tasks[i].remaining[j] = rm->tasks[i].allocated[j];
		}
}
    for (unsigned int i = 0; i < rm->num_tasks; ++i) {
        rm->tasks[i].id = i + 1;
        rm->tasks[i].time_taken = 0;
        rm->tasks[i].waiting_time = 0;
        rm->tasks[i].terminated = false;
        rm->tasks[i].aborted = false;
        for (unsigned int j = 0; j < rm->num_resources; ++j) {
            rm->tasks[i].initial_claims[j] = 0;
            rm->tasks[i].allocated[j] = 0;
        }
    }

    char action[10];
    unsigned int task_number, delay, resource_type, value;

	while (fscanf(file, "%s %u %u %u %u", action, &task_number, &delay, &resource_type, &value) != EOF) {
        Task *task = &rm->tasks[task_number - 1];
        if (strcmp(action, "initiate") == 0) {
            task->initial_claims[resource_type - 1] = value;
            task->remaining[resource_type - 1] = value;
        } else if (strcmp(action, "request") == 0) {
            task->waiting_time += delay;
            task->allocated[resource_type - 1] += value;
            task->remaining[resource_type - 1] -= value;
        } else if (strcmp(action, "release") == 0) {
            task->waiting_time += delay;
            task->allocated[resource_type - 1] -= value;
            task->remaining[resource_type - 1] += value;
        } else if (strcmp(action, "terminate") == 0) {
            task->waiting_time += delay;
            task->terminated = true;
        }
    }

    fclose(file);
}
//Checks to see if the reuqest can be satisfied
bool can_satisfy_request(ResourceManager *rm, Task *task, unsigned int resource_type, unsigned int request) {
    return rm->available_resources[resource_type] >= request && task->remaining[resource_type] >= request;
}
//Optomistic manager
void optimistic_manager(ResourceManager *rm) {
    unsigned int cycle = 0;
    bool all_terminated;

    do {
        all_terminated = true;
        for (unsigned int i = 0; i < rm->num_tasks; ++i) {
            Task *task = &rm->tasks[i];
            if (!task->terminated && !task->aborted) {
                all_terminated = false;
                bool request_satisfied = true;
                for (unsigned int j = 0; j < rm->num_resources; ++j) {
                    if (can_satisfy_request(rm, task, j, task->remaining[j])) {
                        rm->available_resources[j] -= task->remaining[j];
                        task->allocated[j] += task->remaining[j];
                        task->remaining[j] = 0;
                    } else {
                        request_satisfied = false;
                        break;
                    }
                }

                if (request_satisfied) {
                    task->time_taken = cycle;
                    task->terminated = true;
                    for (unsigned int j = 0; j < rm->num_resources; ++j) {
                        rm->available_resources[j] += task->allocated[j];
                    }
                } else {
                    task->waiting_time++;
                }
            }
        }
        cycle++;
    } while (!all_terminated);
}
bool is_safe_state(ResourceManager *rm, Task *task, unsigned int resource_type, unsigned int request_amount) {
    // Check if the request can be granted
    if (request_amount > rm->available_resources[resource_type]) {
        return false;
    }

    // Temporarily grant the request
    rm->available_resources[resource_type] -= request_amount;
    task->allocated[resource_type] += request_amount;
    task->remaining[resource_type] -= request_amount;

    // Check if the system is in a safe state
    bool safe_state = true;
    for (unsigned int i = 0; i < rm->num_tasks && safe_state; ++i) {
        Task *curr_task = &rm->tasks[i];
        if (curr_task->terminated || curr_task->aborted) {
            continue;
        }

        for (unsigned int j = 0; j < rm->num_resources; ++j) {
            if (curr_task->remaining[j] > rm->available_resources[j]) {
                safe_state = false;
                break;
            }
        }
    }

    // Revert the temporary request grant
    rm->available_resources[resource_type] += request_amount;
    task->allocated[resource_type] -= request_amount;
    task->remaining[resource_type] += request_amount;

    return safe_state;
}
//Bankers algorithm
void bankers_algorithm(ResourceManager *rm) {
    unsigned int cycle = 0;
    bool all_terminated;

    do {
        all_terminated = true;
        for (unsigned int i = 0; i < rm->num_tasks; ++i) {
            Task *task = &rm->tasks[i];
            if (!task->terminated && !task->aborted) {
                all_terminated = false;
                bool request_satisfied = true;
                for (unsigned int j = 0; j < rm->num_resources; ++j) {
                    if (is_safe_state(rm, task, j, task->remaining[j])) {
                        rm->available_resources[j] -= task->remaining[j];
                        task->allocated[j] += task->remaining[j];
                        task->remaining[j] = 0;
                    } else {
                        request_satisfied = false;
                        break;
                    }
                }

                if (request_satisfied) {
                    task->time_taken = cycle;
                    task->terminated = true;
                    for (unsigned int j = 0; j < rm->num_resources; ++j) {
                        rm->available_resources[j] += task->allocated[j];
                    }
                } else {
                    task->waiting_time++;
                }
            }
        }
        cycle++;
    } while (!all_terminated);
}
//Printing the results to stdout
void print_results(ResourceManager *rm, const char *algorithm_name) {
    printf("%s\n", algorithm_name);
    unsigned int total_time = 0;
    unsigned int total_waiting_time = 0;

    for (unsigned int i = 0; i < rm->num_tasks; ++i) {
        Task *task = &rm->tasks[i];
        if (task->aborted) {
            printf("Task %u aborted\n", task->id);
        } else {
            unsigned int percentage = (task->time_taken == 0) ? 0 : task->waiting_time * 100 / task->time_taken;
            printf("Task %u %u %u %u%%\n", task->id, task->time_taken, task->waiting_time, percentage);
            total_time += task->time_taken;
            total_waiting_time += task->waiting_time;
        }
    }

    unsigned int overall_percentage = (total_time == 0) ? 0 : total_waiting_time * 100 / total_time;
    printf("total %u %u %u%%\n", total_time, total_waiting_time, overall_percentage);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    ResourceManager rm;
    read_input_file(argv[1], &rm);
    optimistic_manager(&rm);
    print_results(&rm, "FIFO");

    read_input_file(argv[1], &rm);
    bankers_algorithm(&rm);
    print_results(&rm, "BANKER'S");

    return 0;
}