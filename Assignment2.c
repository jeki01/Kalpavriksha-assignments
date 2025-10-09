#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"

typedef struct {
    unsigned int id;       // id cannot be negative
    char name[50];
    unsigned short age;    // age cannot be negative
} User;

// Function prototypes
void addUser();
void displayUsers();
void updateUser();
void deleteUser();
unsigned int getUnsignedInt(const char *prompt);

int main() {
    int choice;

    while (1) {
        printf("\n--- User Management System ---\n");
        printf("1. Add User\n");
        printf("2. Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");

        choice = getUnsignedInt("Enter your choice: ");

        switch (choice) {
            case 1: addUser(); break;
            case 2: displayUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: exit(0);
            default: printf("Invalid choice!\n");
        }
    }
    return 0;
}

// Safe input function for unsigned integers
unsigned int getUnsignedInt(const char *prompt) {
    char buffer[20];
    unsigned int num;
    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Error reading input. Try again.\n");
            continue;
        }
        if (sscanf(buffer, "%u", &num) != 1) {
            printf("Invalid input. Please enter a valid number.\n");
            continue;
        }
        return num;
    }
}

// Add user
void addUser() {
    FILE *fp = fopen(FILE_NAME, "a");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    User u;
    u.id = getUnsignedInt("Enter ID: ");

    printf("Enter Name: ");
    fgets(u.name, sizeof(u.name), stdin);
    u.name[strcspn(u.name, "\n")] = 0;

    u.age = (unsigned short)getUnsignedInt("Enter Age: ");

    fprintf(fp, "%u %s %hu\n", u.id, u.name, u.age);
    fclose(fp);
    printf("User added successfully!\n");
}

// Display users
void displayUsers() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("Error opening file.\n"); // Updated message
        return;
    }

    User u;
    printf("\n--- User Records ---\n");
    while (fscanf(fp, "%u %49s %hu", &u.id, u.name, &u.age) == 3) {
        printf("ID: %u | Name: %s | Age: %hu\n", u.id, u.name, u.age);
    }

    fclose(fp);
}

// Update user by ID (in-memory approach, no temp file)
void updateUser() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    // read all users into array
    #define MAX_USERS 1000
    User users[MAX_USERS];
    int count = 0;
    while (count < MAX_USERS && fscanf(fp, "%u %49s %hu", &users[count].id, users[count].name, &users[count].age) == 3) {
        count++;
    }
    fclose(fp);

    if (count == 0) {
        printf("No records to update.\n");
        return;
    }

    unsigned int id = getUnsignedInt("Enter ID to update: ");
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (users[i].id == id) {
            found = 1;
            printf("Enter new Name: ");
            fgets(users[i].name, sizeof(users[i].name), stdin);
            users[i].name[strcspn(users[i].name, "\n")] = 0;
            users[i].age = (unsigned short)getUnsignedInt("Enter new Age: ");
            break;
        }
    }

    // write back all users to the original file
    fp = fopen(FILE_NAME, "w");
    if (fp == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%u %s %hu\n", users[i].id, users[i].name, users[i].age);
    }
    fclose(fp);

    if (found)
        printf("User updated successfully!\n");
    else
        printf("User with ID %u not found.\n", id);
}


// Delete user by ID 
void deleteUser() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    // read all users into array
    #define MAX_USERS 1000
    static User users[MAX_USERS];
    int count = 0;
    while (count < MAX_USERS && fscanf(fp, "%u %49s %hu", &users[count].id, users[count].name, &users[count].age) == 3) {
        count++;
    }
    fclose(fp);

    if (count == 0) {
        printf("No records to delete.\n");
        return;
    }

    unsigned int id = getUnsignedInt("Enter ID to delete: ");
    int found = 0;

    // remove the user by shifting array
    for (int i = 0; i < count; i++) {
        if (users[i].id == id) {
            found = 1;
            for (int j = i; j < count - 1; j++) {
                users[j] = users[j + 1];
            }
            count--; // one less record
            break;
        }
    }

    // write back remaining users
    fp = fopen(FILE_NAME, "w");
    if (fp == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%u %s %hu\n", users[i].id, users[i].name, users[i].age);
    }
    fclose(fp);

    if (found)
        printf("User deleted successfully!\n");
    else
        printf("User with ID %u not found.\n", id);
}


