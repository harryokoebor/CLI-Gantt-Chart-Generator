// Standard library includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Limits for task names, max tasks, and months displayed
#define TASK_SIZE 25
#define NO_OF_TASKS 10
#define NO_OF_MONTHS 12

// Platform-friendly clear screen command
#ifdef _WIN32
    #define CLEAR_SCREEN "cls"
#else
    #define CLEAR_SCREEN "clear"
#endif

// Month values mapped to numbers (1 = Jan ... 12 = Dec)
typedef enum {
    JAN = 1, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC
} Month;

// Task record: name, timeline, and dependency list
typedef struct {
    char name[TASK_SIZE];
    int startMonth;
    int endMonth;
    int dependencies[NO_OF_TASKS];
    int numDependencies;
} Task;

// ---- Function prototypes ----
static void printAsciiArt(void);
static void welcomeScreen(Task tasks[], int *numOfTasks);
static void loadTestTasks(Task tasks[], int *numOfTasks);
static void editTask(Task tasks[], int numOfTasks);
static void createGantt(Task tasks[], int *numOfTasks);
static void displayGantt(const Task tasks[], int numOfTasks);

static int detectCircularDependency(const Task tasks[], int numOfTasks);
static int isCyclicUtil(const Task tasks[], int current, int visited[], int recursionStack[], int numOfTasks);

static void testCriticalPath(const Task tasks[], int numOfTasks);
static void findCriticalPath(const Task tasks[], int current, int numOfTasks,
                             int currentPath[], int chainLength, int *maxLength,
                             int bestPath[], int visited[]);

// ---- Small helper functions (reduces repeated input code) ----
static void readWord(char *out, int maxLen) {
    // Reads a single token safely (no spaces)
    scanf("%24s", out); // maxLen should match TASK_SIZE - 1 for task names
    out[maxLen - 1] = '\0';
}

static int readIntInRange(const char *prompt, int min, int max) {
    int x;
    while (1) {
        printf("%s", prompt);
        fflush(stdout);

        if (scanf("%d", &x) != 1) {
            // Clear invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Invalid number. Try again.\n");
            continue;
        }

        if (x < min || x > max) {
            printf("Please enter a value between %d and %d.\n", min, max);
            continue;
        }

        return x;
    }
}

// ---- Main ----
int main(void) {
    Task tasks[NO_OF_TASKS];
    int numOfTasks = 0;

    printAsciiArt();
    welcomeScreen(tasks, &numOfTasks);

    return 0;
}

/*
 * Prints the ASCII banner shown when the program starts.
 * No input, no return value.
 */
static void printAsciiArt(void) {
    printf("  ________  ________  ________  ________  ________  ________\n");
    printf(" |  ______||  ______||  ______||  ______||  ______||  ______|\n");
    printf(" | |_____  | |_____  | |_____  | |_____  | |_____  | |_____  \n");
    printf(" |_____  | |_____  | |_____  | |_____  | |_____  | |_____  |\n");
    printf("  ______| | ______| | ______| | ______| | ______| | ______| |\n");
    printf(" |________||________||________||________||________||________|\n");
    puts("");
}

/*
 * Shows the starting menu and keeps the program running until the user quits.
 *
 * Inputs:
 *  - tasks[]: array of Task structs
 *  - numOfTasks: pointer storing how many tasks are currently in use
 */
static void welcomeScreen(Task tasks[], int *numOfTasks) {
    char choice[16];
    int selected = 0;

    printf("Welcome to the Gantt Generator!\n");
    printf("Type \"test\" to load an example or \"create\" to build your own:\n");
    fflush(stdout);

    while (!selected) {
        scanf("%15s", choice);

        if (strcmp(choice, "create") == 0) {
            createGantt(tasks, numOfTasks);
            if (*numOfTasks > 0) displayGantt(tasks, *numOfTasks);
            selected = 1;
        } else if (strcmp(choice, "test") == 0) {
            loadTestTasks(tasks, numOfTasks);
            displayGantt(tasks, *numOfTasks);
            selected = 1;
        } else {
            printf("Invalid input. Type \"test\" or \"create\".\n");
            fflush(stdout);
        }
    }

    while (1) {
        printf("\nOptions: \"create\" | \"edit\" | \"test\" | \"quit\"\n> ");
        fflush(stdout);
        scanf("%15s", choice);

        if (strcmp(choice, "edit") == 0) {
            if (*numOfTasks == 0) {
                printf("No tasks exist yet — create or load test tasks first.\n");
            } else {
                editTask(tasks, *numOfTasks);
                displayGantt(tasks, *numOfTasks);
            }
        } else if (strcmp(choice, "test") == 0) {
            if (*numOfTasks == 0) {
                printf("No tasks exist yet — create or load test tasks first.\n");
            } else {
                testCriticalPath(tasks, *numOfTasks);
            }
        } else if (strcmp(choice, "create") == 0) {
            createGantt(tasks, numOfTasks);
            if (*numOfTasks > 0) displayGantt(tasks, *numOfTasks);
        } else if (strcmp(choice, "quit") == 0) {
            break;
        } else {
            printf("Invalid option. Try again.\n");
        }
    }
}

/*
 * Populates the tasks array with a ready-made demo dataset.
 */
static void loadTestTasks(Task tasks[], int *numOfTasks) {
    *numOfTasks = 10;

    strcpy(tasks[0].name, "Research");
    tasks[0].startMonth = JAN; tasks[0].endMonth = MAR; tasks[0].numDependencies = 0;

    strcpy(tasks[1].name, "Budget_Planning");
    tasks[1].startMonth = FEB; tasks[1].endMonth = MAY; tasks[1].numDependencies = 0;

    strcpy(tasks[2].name, "Interior_design");
    tasks[2].startMonth = MAR; tasks[2].endMonth = JUN;
    tasks[2].numDependencies = 2; tasks[2].dependencies[0] = 0; tasks[2].dependencies[1] = 1;

    strcpy(tasks[3].name, "Site_Analysis");
    tasks[3].startMonth = APR; tasks[3].endMonth = JUL;
    tasks[3].numDependencies = 1; tasks[3].dependencies[0] = 2;

    strcpy(tasks[4].name, "Design_Development");
    tasks[4].startMonth = MAY; tasks[4].endMonth = AUG; tasks[4].numDependencies = 0;

    strcpy(tasks[5].name, "Fixture_Selection");
    tasks[5].startMonth = JUN; tasks[5].endMonth = SEP;
    tasks[5].numDependencies = 1; tasks[5].dependencies[0] = 3;

    strcpy(tasks[6].name, "Permits_Approvals");
    tasks[6].startMonth = JUL; tasks[6].endMonth = OCT;
    tasks[6].numDependencies = 1; tasks[6].dependencies[0] = 5;

    strcpy(tasks[7].name, "Construction_phase");
    tasks[7].startMonth = AUG; tasks[7].endMonth = NOV;
    tasks[7].numDependencies = 1; tasks[7].dependencies[0] = 6;

    strcpy(tasks[8].name, "Interior_Finishing");
    tasks[8].startMonth = SEP; tasks[8].endMonth = DEC; tasks[8].numDependencies = 0;

    strcpy(tasks[9].name, "Final_Inspection");
    tasks[9].startMonth = OCT; tasks[9].endMonth = DEC;
    tasks[9].numDependencies = 2; tasks[9].dependencies[0] = 7; tasks[9].dependencies[1] = 8;
}

/*
 * Allows the user to modify an existing task by searching for its name.
 * If the edit creates a cycle, the change is rolled back.
 */
static void editTask(Task tasks[], int numOfTasks) {
    if (numOfTasks == 0) {
        printf("Error: there are no tasks to edit.\n");
        return;
    }

    char taskToEdit[TASK_SIZE];
    int found = 0;

    printf("Enter the task name to edit (must match exactly): ");
    fflush(stdout);
    scanf("%24s", taskToEdit);

    for (int i = 0; i < numOfTasks; i++) {
        if (strcmp(taskToEdit, tasks[i].name) == 0) {
            found = 1;

            Task backup = tasks[i]; // save current version in case we need to undo

            printf("New task name (use _ for spaces):\n");
            fflush(stdout);
            scanf("%24s", tasks[i].name);

            tasks[i].startMonth = readIntInRange("Start month (1-12):\n", 1, 12);

            // end month must be >= start month
            while (1) {
                tasks[i].endMonth = readIntInRange("End month (1-12):\n", 1, 12);
                if (tasks[i].endMonth < tasks[i].startMonth) {
                    printf("End month cannot be before start month.\n");
                } else break;
            }

            tasks[i].numDependencies = readIntInRange("How many dependencies?\n", 0, numOfTasks);

            for (int j = 0; j < tasks[i].numDependencies; j++) {
                int depNum = readIntInRange("Enter dependent task number (1..N):\n", 1, numOfTasks);
                tasks[i].dependencies[j] = depNum - 1;
            }

            if (detectCircularDependency(tasks, numOfTasks)) {
                printf("Error: a circular dependency was created — reverting changes.\n");
                tasks[i] = backup;
            } else {
                printf("Task updated successfully.\n");
            }

            break;
        }
    }

    if (!found) {
        printf("Task not found — check spelling and underscores.\n");
    }
}

/*
 * Prompts the user to input tasks (name, start/end month, dependency list).
 * If the final set contains a cycle, the creation is cancelled.
 */
static void createGantt(Task tasks[], int *numOfTasks) {
    *numOfTasks = readIntInRange("How many tasks would you like to add? (1-10)\n", 1, 10);

    for (int i = 0; i < *numOfTasks; i++) {
        printf("Task %d name (use _ for spaces):\n", i + 1);
        fflush(stdout);
        scanf("%24s", tasks[i].name);

        tasks[i].startMonth = readIntInRange("Start month (1-12):\n", 1, 12);

        while (1) {
            tasks[i].endMonth = readIntInRange("End month (1-12):\n", 1, 12);
            if (tasks[i].endMonth < tasks[i].startMonth) {
                printf("End month cannot be before start month.\n");
            } else break;
        }

        tasks[i].numDependencies = readIntInRange("How many dependencies?\n", 0, *numOfTasks);

        for (int j = 0; j < tasks[i].numDependencies; j++) {
            int depNum = readIntInRange("Enter dependent task number (1..N):\n", 1, *numOfTasks);
            tasks[i].dependencies[j] = depNum - 1;
        }
    }

    if (detectCircularDependency(tasks, *numOfTasks)) {
        printf("Error: circular dependency found — creation cancelled.\n");
        printf("Tasks involved in the cycle were printed above.\n");
        *numOfTasks = 0;
    } else {
        printf("Tasks created successfully.\n");
    }
}

/*
 * Renders the Gantt chart as a 12-month table in the terminal.
 */
static void displayGantt(const Task tasks[], int numOfTasks) {
    system(CLEAR_SCREEN);

    printf("__________________________________________________________________________________________________________________________________________________________________________________\n");
    printf("                    | January   | February  | March     |  April    |  May      |   June    |   July    |  August   | September |  October  |  November |  December | Dependencies \n");
    printf("----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < numOfTasks; i++) {
        printf(" %-18s |", tasks[i].name);

        for (int m = 1; m <= NO_OF_MONTHS; m++) {
            if (m >= tasks[i].startMonth && m <= tasks[i].endMonth) {
                printf("    XXX    |");
            } else {
                printf("           |");
            }
        }

        if (tasks[i].numDependencies > 0) {
            for (int j = 0; j < tasks[i].numDependencies; j++) {
                printf(" %d", tasks[i].dependencies[j] + 1);
            }
        } else {
            printf("             ");
        }

        printf("\n----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    }
}

/*
 * Checks all tasks for dependency cycles using DFS + a recursion stack.
 * Returns 1 if a cycle exists, otherwise 0.
 */
static int detectCircularDependency(const Task tasks[], int numOfTasks) {
    int visited[NO_OF_TASKS] = {0};
    int stack[NO_OF_TASKS] = {0};

    for (int i = 0; i < numOfTasks; i++) {
        if (!visited[i] && isCyclicUtil(tasks, i, visited, stack, numOfTasks)) {
            return 1;
        }
    }
    return 0;
}

/*
 * DFS cycle detector: if a node is found inside the current recursion stack,
 * a cycle exists. Also prints the edge that shows the cycle.
 */
static int isCyclicUtil(const Task tasks[], int current, int visited[], int recursionStack[], int numOfTasks) {
    (void)numOfTasks; // not strictly needed here, but kept for signature consistency

    visited[current] = 1;
    recursionStack[current] = 1;

    for (int i = 0; i < tasks[current].numDependencies; i++) {
        int dep = tasks[current].dependencies[i];

        if (!visited[dep]) {
            if (isCyclicUtil(tasks, dep, visited, recursionStack, numOfTasks)) {
                printf("Circular dependency detected: %s (%d) -> %s (%d)\n",
                       tasks[current].name, current + 1, tasks[dep].name, dep + 1);
                return 1;
            }
        } else if (recursionStack[dep]) {
            printf("Circular dependency detected: %s (%d) -> %s (%d)\n",
                   tasks[current].name, current + 1, tasks[dep].name, dep + 1);
            return 1;
        }
    }

    recursionStack[current] = 0;
    return 0;
}

/*
 * Explores dependencies recursively to find the longest dependency chain
 * (used here as a critical-path style chain).
 */
static void findCriticalPath(const Task tasks[], int current, int numOfTasks,
                             int currentPath[], int chainLength, int *maxLength,
                             int bestPath[], int visited[]) {

    if (visited[current]) return;

    visited[current] = 1;
    currentPath[chainLength++] = current;

    if (chainLength > *maxLength) {
        *maxLength = chainLength;
        for (int i = 0; i < chainLength; i++) bestPath[i] = currentPath[i];
    }

    for (int i = 0; i < tasks[current].numDependencies; i++) {
        int dep = tasks[current].dependencies[i];
        if (dep >= 0 && dep < numOfTasks) {
            findCriticalPath(tasks, dep, numOfTasks, currentPath, chainLength, maxLength, bestPath, visited);
        }
    }

    visited[current] = 0;
}

/*
 * Lets the user choose a starting task and prints the longest chain from it.
 * Also verifies that the overall graph has no cycles.
 */
static void testCriticalPath(const Task tasks[], int numOfTasks) {
    int start = readIntInRange("Enter starting task number for critical path test:\n", 1, numOfTasks) - 1;

    int currentPath[NO_OF_TASKS] = {0};
    int bestPath[NO_OF_TASKS] = {0};
    int visited[NO_OF_TASKS] = {0};
    int maxLength = 0;

    findCriticalPath(tasks, start, numOfTasks, currentPath, 0, &maxLength, bestPath, visited);

    printf("Critical Path: ");
    for (int i = 0; i < maxLength; i++) {
        printf("%s", tasks[bestPath[i]].name);
        if (i != maxLength - 1) printf(" -> ");
    }
    printf("\n");

    if (detectCircularDependency(tasks, numOfTasks)) {
        printf("!!! Circular Dependency Found !!!\n");
    } else {
        printf("No circular dependencies found. Critical path is valid.\n");
    }
}
