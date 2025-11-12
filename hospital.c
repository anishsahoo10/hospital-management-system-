#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PATIENT_FILE "patients.dat"
#define SCHEDULE_FILE "schedules.dat"
#define BILL_FILE "bills.dat"

typedef struct {
    int id;
    char name[100];
    int age;
    char gender[10];
    char phone[20];
} Patient;

typedef struct {
    int id;
    char doctor_name[100];
    char specialization[60];
    char day[20];     // e.g., "Monday"
    char slot[30];    // e.g., "10:00-12:00"
} DoctorSchedule;

typedef struct {
    int id;
    int patient_id;
    char patient_name[100];
    char doctor_name[100];
    double amount;
    char date[30]; // e.g., "2025-11-12 17:45"
} Bill;

/* Utility: read a line from stdin and strip newline */
void read_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        size_t ln = strlen(buffer);
        if (ln && buffer[ln-1] == '\n') buffer[ln-1] = '\0';
    }
}

/* Get next ID for a file of structures where first int field is id */
int get_next_id(const char *filename, size_t recsize) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 1; // file not exist -> start from 1
    int max_id = 0;
    void *buf = malloc(recsize);
    while (fread(buf, recsize, 1, f) == 1) {
        int id = *((int *)buf);
        if (id > max_id) max_id = id;
    }
    free(buf);
    fclose(f);
    return max_id + 1;
}

/* -------- Patients -------- */
void add_patient() {
    Patient p;
    p.id = get_next_id(PATIENT_FILE, sizeof(Patient));
    printf("\n--- Add Patient (ID %d) ---\n", p.id);
    printf("Name: ");
    read_line(p.name, sizeof(p.name));
    printf("Age: ");
    char tmp[20]; read_line(tmp, sizeof(tmp)); p.age = atoi(tmp);
    printf("Gender: ");
    read_line(p.gender, sizeof(p.gender));
    printf("Phone: ");
    read_line(p.phone, sizeof(p.phone));

    FILE *f = fopen(PATIENT_FILE, "ab");
    if (!f) { perror("Unable to open patient file"); return; }
    fwrite(&p, sizeof(Patient), 1, f);
    fclose(f);
    printf("Patient added successfully.\n");
}

void list_patients() {
    FILE *f = fopen(PATIENT_FILE, "rb");
    if (!f) { printf("No patients found.\n"); return; }
    Patient p;
    printf("\n--- Patients ---\n");
    printf("%-4s %-25s %-4s %-8s %-12s\n", "ID", "Name", "Age", "Gender", "Phone");
    while (fread(&p, sizeof(Patient), 1, f) == 1) {
        printf("%-4d %-25s %-4d %-8s %-12s\n", p.id, p.name, p.age, p.gender, p.phone);
    }
    fclose(f);
}

/* -------- Doctor Schedules -------- */
void add_schedule() {
    DoctorSchedule d;
    d.id = get_next_id(SCHEDULE_FILE, sizeof(DoctorSchedule));
    printf("\n--- Add Doctor Schedule (ID %d) ---\n", d.id);
    printf("Doctor name: ");
    read_line(d.doctor_name, sizeof(d.doctor_name));
    printf("Specialization: ");
    read_line(d.specialization, sizeof(d.specialization));
    printf("Day (e.g., Monday): ");
    read_line(d.day, sizeof(d.day));
    printf("Slot (e.g., 09:00-11:00): ");
    read_line(d.slot, sizeof(d.slot));

    FILE *f = fopen(SCHEDULE_FILE, "ab");
    if (!f) { perror("Unable to open schedule file"); return; }
    fwrite(&d, sizeof(DoctorSchedule), 1, f);
    fclose(f);
    printf("Schedule added successfully.\n");
}

void list_schedules() {
    FILE *f = fopen(SCHEDULE_FILE, "rb");
    if (!f) { printf("No schedules found.\n"); return; }
    DoctorSchedule d;
    printf("\n--- Doctor Schedules ---\n");
    printf("%-4s %-20s %-15s %-10s %-12s\n", "ID", "Doctor", "Specialization", "Day", "Slot");
    while (fread(&d, sizeof(DoctorSchedule), 1, f) == 1) {
        printf("%-4d %-20s %-15s %-10s %-12s\n", d.id, d.doctor_name, d.specialization, d.day, d.slot);
    }
    fclose(f);
}

/* -------- Bills -------- */
void get_current_datetime(char *buf, int size) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm);
}

/* helper to find patient by id; returns 1 on found and fills patient */
int find_patient_by_id(int id, Patient *out) {
    FILE *f = fopen(PATIENT_FILE, "rb");
    if (!f) return 0;
    Patient p;
    int found = 0;
    while (fread(&p, sizeof(Patient), 1, f) == 1) {
        if (p.id == id) { *out = p; found = 1; break; }
    }
    fclose(f);
    return found;
}

void create_bill() {
    Bill b;
    b.id = get_next_id(BILL_FILE, sizeof(Bill));
    printf("\n--- Create Bill (ID %d) ---\n", b.id);
    printf("Patient ID: ");
    char tmp[20]; read_line(tmp, sizeof(tmp));
    b.patient_id = atoi(tmp);

    Patient p;
    if (!find_patient_by_id(b.patient_id, &p)) {
        printf("Patient with ID %d not found. Cancelled.\n", b.patient_id);
        return;
    }
    strncpy(b.patient_name, p.name, sizeof(b.patient_name)-1);
    b.patient_name[sizeof(b.patient_name)-1] = '\0';

    printf("Doctor name: ");
    read_line(b.doctor_name, sizeof(b.doctor_name));
    printf("Amount: ");
    read_line(tmp, sizeof(tmp));
    b.amount = atof(tmp);

    get_current_datetime(b.date, sizeof(b.date));

    FILE *f = fopen(BILL_FILE, "ab");
    if (!f) { perror("Unable to open bill file"); return; }
    fwrite(&b, sizeof(Bill), 1, f);
    fclose(f);
    printf("Bill created: Patient '%s' | Doctor '%s' | Amount %.2f | Date %s\n",
           b.patient_name, b.doctor_name, b.amount, b.date);
}

void list_bills() {
    FILE *f = fopen(BILL_FILE, "rb");
    if (!f) { printf("No bills found.\n"); return; }
    Bill b;
    printf("\n--- Bills ---\n");
    printf("%-4s %-10s %-20s %-15s %-10s %-20s\n", "ID", "PatID", "Patient", "Doctor", "Amount", "Date");
    while (fread(&b, sizeof(Bill), 1, f) == 1) {
        printf("%-4d %-10d %-20s %-15s %-10.2f %-20s\n",
               b.id, b.patient_id, b.patient_name, b.doctor_name, b.amount, b.date);
    }
    fclose(f);
}

/* -------- Simple menu -------- */
void show_menu() {
    printf("\n================ Hospital Management ================\n");
    printf("1. Add patient\n");
    printf("2. List patients\n");
    printf("3. Add doctor schedule\n");
    printf("4. List schedules\n");
    printf("5. Create bill\n");
    printf("6. List bills\n");
    printf("7. Exit\n");
    printf("Choose an option: ");
}

int main() {
    char choice[10];
    while (1) {
        show_menu();
        read_line(choice, sizeof(choice));
        switch (atoi(choice)) {
            case 1: add_patient(); break;
            case 2: list_patients(); break;
            case 3: add_schedule(); break;
            case 4: list_schedules(); break;
            case 5: create_bill(); break;
            case 6: list_bills(); break;
            case 7: printf("Goodbye\n"); exit(0);
            default: printf("Invalid option. Try again.\n");
        }
    }
    return 0;
}
