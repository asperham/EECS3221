/**
 *      Colin LeDonne   
 *      EECS 3221-Z
 *
 *      I was able to implement most of the required things. However the output file 
 *      always has a string of numbers at the end which I was unable to fix. I was also unable to 
 *      implement the burst time correctly so the outputs are incorrect. 
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define MAX_QUANTUM 2

// Structure to store process information
struct Process {
    int id;
    int cpu_time;
    int io_time;
    int arrival_time;
};

// Function to compare two processes based on their arrival time
int compare_processes(const void *a, const void *b) {
    struct Process *p1 = (struct Process *)a;
    struct Process *p2 = (struct Process *)b;
    return p1->arrival_time - p2->arrival_time;
}

// Function to simulate the scheduling algorithms
void simulate_scheduling(struct Process processes[], int n, int schedule_algorithm, FILE* output_file_name) {
    int current_time = 0;
    int finish_time = 0;


    // Sort the processes in the order of their arrival time
    qsort(processes, n, sizeof(struct Process), compare_processes);

    // Execute the scheduling algorithm based on the input value
    switch (schedule_algorithm) {
        // First-Come-First-Served (FCFS)
        case 0:

 {
            // Execute each process in the order of their arrival time
            for (int i = 0; i < n; i++) {
                current_time = finish_time;
                current_time += processes[i].cpu_time;
                finish_time = current_time + processes[i].io_time;
               fprintf(output_file_name,"%d %d:running\n", processes[i].id, current_time);
               fprintf(output_file_name,"%d %d:blocked\n", processes[i].id, finish_time);
            }
            break;
        }
        // Round Robin (RR)
        case 1:
        {
            int quantum = 0;
            int i = 0;

            // Execute each process for a fixed time quantum
            while (1) {
                if (i == n) {
                    i = 0;
                }
                if (processes[i].cpu_time == 0) {
                    i++;
                    continue;
                }
                quantum = (processes[i].cpu_time > MAX_QUANTUM) ? MAX_QUANTUM : processes[i].cpu_time;
                current_time += quantum;
                processes[i].cpu_time -= quantum;
                fprintf(output_file_name,"%d %d:running\n", processes[i].id, current_time);
				if (processes[i].cpu_time == 0) {
                    finish_time = current_time + processes[i].io_time;
                   fprintf(output_file_name,"%d %d:blocked\n", processes[i].id, finish_time);
                }
                i++;
                int all_processes_completed = 1;
                for (int j = 0; j < n; j++) {
                    if (processes[j].cpu_time > 0) {
                        all_processes_completed = 0;
                        break;
                    }
                }
                if (all_processes_completed) {
                    break;
                }
                                 break;
            }
        }
                case 2:
                // Shortest Job First (SJF) 
        {
            int i = 0;

            // Execute the process with the shortest CPU time
            while (1) {
			int shortest_job = -1;
                for (int j = 0; j < n; j++) {
                    if (processes[j].cpu_time > 0 && (shortest_job == -1 || processes[j].cpu_time < processes[shortest_job].cpu_time)) {
                        shortest_job = j;
                    }
                }
                if (shortest_job == -1) {
                    break;
                }
                current_time += processes[shortest_job].cpu_time;
                processes[shortest_job].cpu_time = 0;
                fprintf(output_file_name,"%d %d:running\n", processes[shortest_job].id, current_time);
                finish_time = current_time + processes[shortest_job].io_time;
                fprintf(output_file_name,"%d %d:blocked\n", processes[shortest_job].id, finish_time);
            }
            break;
        }
        default:
        {
            printf("Error: Invalid scheduling algorithm\n");
            exit(1);
        }
    }
    // Calculate the CPU utilization
    int total_cpu_time = 0;
    for (int i = 0; i < n; i++) {
        total_cpu_time += processes[i].cpu_time;
    }
    int cpu_utilization = 1.0 * total_cpu_time / finish_time;
    fprintf(output_file_name, "\nFinishing time: %d\n", finish_time);
    fprintf(output_file_name, "CPU utilization: %d\n", cpu_utilization);

    // Calculate the turnaround time for each process
    for (int i = 0; i < n; i++) {
        int turnaround_time = finish_time - processes[i].arrival_time;
        fprintf(output_file_name, "Turnaround process %d: %d\n", processes[i].id, turnaround_time);
    }
}
int main(int argc, char *argv[]) {

    int schedule_algorithm = atoi(argv[2]);

    // Read the input file
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error: Unable to open input file\n");
        exit(1);
    }
    int n;
    fscanf(fp, "%d", &n);
    struct Process processes[MAX_PROCESSES];
    for (int i = 0; i < n; i++) {
        fscanf(fp, "%d %d %d %d", &processes[i].id, &processes[i].cpu_time, &processes[i].io_time, &processes[i].arrival_time);
    }

    //Creating output file
    char output_file_name[100];
    sprintf(output_file_name, "%s-%d.txt", argv[1], schedule_algorithm);
    FILE *output_file = fopen(output_file_name, "w");

    fclose(fp);

    simulate_scheduling(processes, n, schedule_algorithm, output_file);
    fclose(output_file);
    return 0;
}

