#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define STORAGE_FILE "expenses.csv"
#define MAX_LINE_LEN 512
#define MAX_ARGS 10
#define MAX_CAT_REPORT 256

typedef struct {
    unsigned int id;
    char date[12];
    double amount;
    char category[64];
    char description[128];
} Transaction;

typedef struct {
    char name[64];
    double total;
} CategorySummary;

Transaction *transactions = NULL;
int transaction_count = 0;
int transaction_capacity = 0;
unsigned int next_id = 1;

void strip_newline(char *str) {
    str[strcspn(str, "\r\n")] = 0;
}

void get_current_date(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 12, "%Y-%m-%d", tm_info);
}

int validate_date(const char *date) {
    if (strlen(date) != 10) return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') return 0;
        } else {
            if (!isdigit(date[i])) return 0;
        }
    }
    int year = atoi(date);
    int month = atoi(date + 5);
    int day = atoi(date + 8);
    if (year < 1900 || year > 2100) return 0;
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    return 1;
}

int parse_arguments(char *input, char **argv) {
    int argc = 0;
    char *ptr = input;
    while (*ptr && argc < MAX_ARGS) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0') break;
        if (*ptr == '"') {
            ptr++;
            argv[argc++] = ptr;
            while (*ptr && *ptr != '"') ptr++;
            if (*ptr == '"') {
                *ptr = '\0';
                ptr++;
            }
        } else {
            argv[argc++] = ptr;
            while (*ptr && *ptr != ' ' && *ptr != '\t') ptr++;
            if (*ptr != '\0') {
                *ptr = '\0';
                ptr++;
            }
        }
    }
    return argc;
}

void ensure_capacity() {
    if (transaction_count >= transaction_capacity) {
        transaction_capacity = transaction_capacity == 0 ? 16 : transaction_capacity * 2;
        Transaction *temp = realloc(transactions, transaction_capacity * sizeof(Transaction));
        if (!temp) {
            fprintf(stderr, "Fatal Error: Memory allocation failed.\n");
            exit(1);
        }
        transactions = temp;
    }
}

void load_data() {
    FILE *file = fopen(STORAGE_FILE, "r");
    if (!file) return;
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        strip_newline(line);
        if (strlen(line) == 0) continue;
        ensure_capacity();
        Transaction *t = &transactions[transaction_count];
        char *tokens[5];
        int count = 0;
        char *ptr = line;
        char *start = line;
        while (*ptr) {
            if (*ptr == ',') {
                *ptr = '\0';
                tokens[count++] = start;
                start = ptr + 1;
            }
            if (count >= 4) break;
            ptr++;
        }
        tokens[count++] = start;
        if (count < 5) continue;
        t->id = (unsigned int)strtoul(tokens[0], NULL, 10);
        strncpy(t->date, tokens[1], sizeof(t->date) - 1);
        t->amount = strtod(tokens[2], NULL);
        strncpy(t->category, tokens[3], sizeof(t->category) - 1);
        strncpy(t->description, tokens[4], sizeof(t->description) - 1);
        if (t->id >= next_id) {
            next_id = t->id + 1;
        }
        transaction_count++;
    }
    fclose(file);
}

void save_data() {
    FILE *file = fopen(STORAGE_FILE, "w");
    if (!file) {
        fprintf(stderr, "Error: Unable to open storage file for writing.\n");
        return;
    }
    for (int i = 0; i < transaction_count; i++) {
        fprintf(file, "%u,%s,%.2f,%s,%s\n",
                transactions[i].id,
                transactions[i].date,
                transactions[i].amount,
                transactions[i].category,
                transactions[i].description);
    }
    fclose(file);
}

void handle_add(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        printf("Syntax Error: Invalid arguments.\n");
        printf("Usage: add <amount> <\"category\"> <\"description\"> [YYYY-MM-DD]\n");
        return;
    }
    ensure_capacity();
    Transaction *t = &transactions[transaction_count];
    t->id = next_id++;
    t->amount = strtod(argv[0], NULL);
    if (t->amount <= 0) {
        printf("Validation Error: Amount must be a positive number.\n");
        return;
    }
    strncpy(t->category, argv[1], sizeof(t->category) - 1);
    strncpy(t->description, argv[2], sizeof(t->description) - 1);
    if (argc == 4) {
        if (!validate_date(argv[3])) {
            printf("Validation Error: Invalid date format. Use YYYY-MM-DD.\n");
            return;
        }
        strncpy(t->date, argv[3], sizeof(t->date) - 1);
    } else {
        get_current_date(t->date);
    }
    transaction_count++;
    save_data();
    printf("Success: Transaction added successfully with ID: %u\n", t->id);
}

void handle_list(int argc, char **argv) {
    if (transaction_count == 0) {
        printf("Info: No transaction records found.\n");
        return;
    }
    char *filter_cat = (argc >= 1) ? argv[0] : NULL;
    printf("\n=========================================================================================\n");
    printf("%-6s | %-12s | %-18s | %-20s | %-25s\n", "ID", "Date", "Amount", "Category", "Description");
    printf("=========================================================================================\n");
    int match_count = 0;
    for (int i = 0; i < transaction_count; i++) {
        if (filter_cat && strcasecmp(transactions[i].category, filter_cat) != 0) {
            continue;
        }
        printf("%-6u | %-12s | $%-17.2f | %-20s | %-25s\n",
               transactions[i].id,
               transactions[i].date,
               transactions[i].amount,
               transactions[i].category,
               transactions[i].description);
        match_count++;
    }
    printf("=========================================================================================\n");
    printf("Total records displayed: %d\n\n", match_count);
}

void handle_delete(int argc, char **argv) {
    if (argc != 1) {
        printf("Syntax Error: Usage: delete <transaction_id>\n");
        return;
    }
    unsigned int target_id = (unsigned int)strtoul(argv[0], NULL, 10);
    int target_index = -1;
    for (int i = 0; i < transaction_count; i++) {
        if (transactions[i].id == target_id) {
            target_index = i;
            break;
        }
    }
    if (target_index == -1) {
        printf("Error: Transaction ID %u not found.\n", target_id);
        return;
    }
    for (int i = target_index; i < transaction_count - 1; i++) {
        transactions[i] = transactions[i + 1];
    }
    transaction_count--;
    save_data();
    printf("Success: Transaction %u has been deleted.\n", target_id);
}

void handle_report(int argc, char **argv) {
    if (argc != 1) {
        printf("Syntax Error: Usage: report <YYYY-MM>\n");
        return;
    }
    char *target_month = argv[0];
    if (strlen(target_month) != 7 || target_month[4] != '-') {
        printf("Validation Error: Invalid month format. Use YYYY-MM (e.g., 2026-07).\n");
        return;
    }
    CategorySummary summaries[MAX_CAT_REPORT];
    int summary_count = 0;
    double grand_total = 0;
    double max_expense = 0;
    int monthly_trans_count = 0;
    for (int i = 0; i < transaction_count; i++) {
        if (strncmp(transactions[i].date, target_month, 7) == 0) {
            grand_total += transactions[i].amount;
            monthly_trans_count++;
            if (transactions[i].amount > max_expense) {
                max_expense = transactions[i].amount;
            }
            int found = 0;
            for (int j = 0; j < summary_count; j++) {
                if (strcasecmp(summaries[j].name, transactions[i].category) == 0) {
                    summaries[j].total += transactions[i].amount;
                    found = 1;
                    break;
                }
            }
            if (!found && summary_count < MAX_CAT_REPORT) {
                strncpy(summaries[summary_count].name, transactions[i].category, sizeof(summaries[summary_count].name) - 1);
                summaries[summary_count].total = transactions[i].amount;
                summary_count++;
            }
        }
    }
    if (monthly_trans_count == 0) {
        printf("Info: No financial records found for the period %s.\n", target_month);
        return;
    }
    printf("\n=============================================\n");
    printf(" FINANCIAL REPORT PERIOD: %s\n", target_month);
    printf("=============================================\n");
    printf("%-25s | %-15s\n", "Category Breakdown", "Total Expenses");
    printf("---------------------------------------------\n");
    for (int i = 0; i < summary_count; i++) {
        printf("%-25s | $%-14.2f\n", summaries[i].name, summaries[i].total);
    }
    printf("=============================================\n");
    printf("%-25s | $%-14.2f\n", "GRAND TOTAL", grand_total);
    printf("%-25s | $%-14.2f\n", "AVERAGE PER TRANSACTION", grand_total / monthly_trans_count);
    printf("%-25s | $%-14.2f\n", "HIGHEST SINGLE EXPENSE", max_expense);
    printf("=============================================\n\n");
}

void handle_help() {
    printf("\nAvailable Command Interface Utilities:\n");
    printf("  add <amount> <\"cat\"> <\"desc\"> [date]   Add transaction ([date]: YYYY-MM-DD optional)\n");
    printf("  list [\"category\"]                     Display records (optional category filter)\n");
    printf("  delete <id>                           Remove a single record permanently by ID\n");
    printf("  report <YYYY-MM>                      Generate aggregate metrics and breakdown\n");
    printf("  help                                  Show this command parameter schema\n");
    printf("  exit                                  Commit storage alterations and close\n\n");
}

int main() {
    char input[MAX_LINE_LEN];
    FILE *storage_init = fopen(STORAGE_FILE, "a+");
    if (!storage_init) {
        fprintf(stderr, "Fatal: Runtime storage engine failed initialization.\n");
        return 1;
    }
    fclose(storage_init);
    load_data();
    printf("Just A Simple Project Expanse Trackers :D Pls Dont Judges\n");
    printf("Hmm... You Can Just Type 'help'\n\n");
    while (1) {
        printf("expense-tracker>$ ");
        if (!fgets(input, sizeof(input), stdin)) break;
        strip_newline(input);
        if (strlen(input) == 0) continue;
        char *argv[MAX_ARGS];
        int argc = parse_arguments(input, argv);
        if (argc == 0) continue;
        char *cmd = argv[0];
        char **cmd_args = &argv[1];
        int cmd_argc = argc - 1;
        if (strcmp(cmd, "exit") == 0) {
            break;
        } else if (strcmp(cmd, "add") == 0) {
            handle_add(cmd_argc, cmd_args);
        } else if (strcmp(cmd, "list") == 0) {
            handle_list(cmd_argc, cmd_args);
        } else if (strcmp(cmd, "delete") == 0) {
            handle_delete(cmd_argc, cmd_args);
        } else if (strcmp(cmd, "report") == 0) {
            handle_report(cmd_argc, cmd_args);
        } else if (strcmp(cmd, "help") == 0) {
            handle_help();
        } else {
            printf("Is This CMD Error :D : I don't understand this statement '%s'.\n", cmd);
        }
    }
    free(transactions);
    return 0;
}
