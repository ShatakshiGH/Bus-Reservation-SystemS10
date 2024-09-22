#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 100

typedef struct {
    char username[MAX_LEN];
    char password[MAX_LEN];
} User;

typedef struct {
    char bus_number[MAX_LEN];
    char origin[MAX_LEN];
    char destination[MAX_LEN];
    int total_seats;
    int available_seats;
    int fare;
} Bus;

typedef struct {
    char booking_id[MAX_LEN];
    char username[MAX_LEN];
    char bus_number[MAX_LEN];
    char name[MAX_LEN];
    int age;
} Booking;

void register_user() {
    FILE *file = fopen("users.txt", "a");
    if (!file) {
        perror("Failed to open users file");
        return;
    }

    User user;
    printf("Enter username: ");
    scanf("%49s", user.username);
    printf("Enter password: ");
    scanf("%49s", user.password);

    if (fprintf(file, "%s %s\n", user.username, user.password) < 0) {
        perror("Failed to write to users file");
    }

    fflush(file);
    fclose(file);
    printf("Registration successful.\n");
}

int login_user(char *logged_in_username) {
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        perror("Failed to open users file");
        return 0;
    }

    User user;
    char username[MAX_LEN], password[MAX_LEN];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    while (fscanf(file, "%s %s", user.username, user.password) != EOF) {
        if (strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
            fclose(file);
            strcpy(logged_in_username, username);
            printf("Login successful.\n");
            return 1;
        }
    }

    fclose(file);
    printf("Invalid username or password.\n");
    return 0;
}

void load_buses(Bus buses[], int *bus_count) {
    FILE *file = fopen("buses.txt", "r");
    if (!file) {
        perror("Failed to open buses file");
        return;
    }

    *bus_count = 0;
    while (fscanf(file, "%s %s %s %d %d %d", buses[*bus_count].bus_number, buses[*bus_count].origin,
                  buses[*bus_count].destination, &buses[*bus_count].total_seats, 
                  &buses[*bus_count].available_seats, &buses[*bus_count].fare) != EOF) {
        (*bus_count)++;
    }

    fclose(file);
}

void save_buses(Bus buses[], int bus_count) {
    FILE *file = fopen("buses.txt", "w");
    if (!file) {
        perror("Failed to open buses file");
        return;
    }

    for (int i = 0; i < bus_count; i++) {
        fprintf(file, "%s %s %s %d %d %d\n", buses[i].bus_number, buses[i].origin, buses[i].destination,
                buses[i].total_seats, buses[i].available_seats, buses[i].fare);
    }

    fclose(file);
}

void book_ticket(char *username) {
    Bus buses[MAX_LEN];
    int bus_count;
    load_buses(buses, &bus_count);

    char origin[MAX_LEN], destination[MAX_LEN];
    printf("Enter origin: ");
    scanf("%s", origin);
    printf("Enter destination: ");
    scanf("%s", destination);

    int found = 0;
    printf("\nAvailable buses for the route %s to %s:\n", origin, destination);
    for (int i = 0; i < bus_count; i++) {
        if (strcmp(buses[i].origin, origin) == 0 && strcmp(buses[i].destination, destination) == 0) {
            printf("Bus Number: %s, Total Seats: %d, Available Seats: %d, Fare: %d\n", 
                   buses[i].bus_number, buses[i].total_seats, buses[i].available_seats, buses[i].fare);
            found = 1;
        }
    }

    if (!found) {
        printf("No buses available for the entered route.\n");
        return;
    }

    char option;
    printf("Do you want to book a ticket? (Y/N): ");
    scanf(" %c", &option);

    if (option == 'N' || option == 'n') {
        return;
    }

    char bus_number[MAX_LEN];
    printf("Enter bus number: ");
    scanf("%s", bus_number);

    int i;
    for (i = 0; i < bus_count; i++) {
        if (strcmp(buses[i].bus_number, bus_number) == 0) {
            break;
        }
    }

    if (i == bus_count) {
        printf("Invalid bus number.\n");
        return;
    }

    if (buses[i].available_seats == 0) {
        printf("No available seats.\n");
        return;
    }

    Booking booking;
    strcpy(booking.username, username);
    strcpy(booking.bus_number, bus_number);

    printf("Enter your name: ");
    scanf("%s", booking.name);
    printf("Enter your age: ");
    scanf("%d", &booking.age);

    snprintf(booking.booking_id, MAX_LEN, "B%d%d", rand() % 10000, i);  // Generate unique booking ID

    FILE *file = fopen("bookings.txt", "a");
    if (!file) {
        perror("Failed to open bookings file");
        return;
    }

    fprintf(file, "%s %s %s %s %d\n", booking.booking_id, booking.username, booking.bus_number, booking.name, booking.age);
    fclose(file);

    buses[i].available_seats--;
    save_buses(buses, bus_count);

    printf("Ticket booked successfully. Your booking ID is: %s\n", booking.booking_id);
}

void cancel_ticket(char *username) {
    char booking_id[MAX_LEN];
    printf("Enter your booking ID: ");
    scanf("%s", booking_id);

    Bus buses[MAX_LEN];
    int bus_count;
    load_buses(buses, &bus_count);

    FILE *file = fopen("bookings.txt", "r");
    if (!file) {
        perror("Failed to open bookings file");
        return;
    }

    FILE *temp_file = fopen("temp.txt", "w");
    if (!temp_file) {
        perror("Failed to open temp file");
        fclose(file);
        return;
    }

    Booking booking;
    int found = 0;
    while (fscanf(file, "%s %s %s %s %d", booking.booking_id, booking.username, booking.bus_number, booking.name, &booking.age) != EOF) {
        if (strcmp(booking.booking_id, booking_id) == 0 && strcmp(booking.username, username) == 0) {
            found = 1;
        } else {
            fprintf(temp_file, "%s %s %s %s %d\n", booking.booking_id, booking.username, booking.bus_number, booking.name, booking.age);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        remove("bookings.txt");
        rename("temp.txt", "bookings.txt");

        for (int i = 0; i < bus_count; i++) {
            if (strcmp(buses[i].bus_number, booking.bus_number) == 0) {
                buses[i].available_seats++;
                break;
            }
        }

        save_buses(buses, bus_count);
        printf("Ticket cancelled successfully.\n");
    } else {
        remove("temp.txt");
        printf("No booking found with the provided details.\n");
    }
}

void check_bus_status() {
    Bus buses[MAX_LEN];
    int bus_count;
    load_buses(buses, &bus_count);

    char bus_number[MAX_LEN];
    printf("Enter bus number: ");
    scanf("%s", bus_number);

    int found = 0;
    for (int i = 0; i < bus_count; i++) {
        if (strcmp(buses[i].bus_number, bus_number) == 0) {
            printf("Bus Number: %s, Origin: %s, Destination: %s, Total Seats: %d, Available Seats: %d, Fare: %d\n", 
                   buses[i].bus_number, buses[i].origin, buses[i].destination, buses[i].total_seats, buses[i].available_seats, buses[i].fare);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("No bus found with the entered number.\n");
    }
}

int main() {
    char logged_in_username[MAX_LEN] = "";
    int is_logged_in = 0;

    while (1) {
        if (!is_logged_in) {
            int choice;
            printf("\n--- Bus Reservation System ---\n");
            printf("1. Register\n");
            printf("2. Login\n");
            printf("3. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    register_user();
                    break;
                case 2:
                    if (login_user(logged_in_username)) {
                        is_logged_in = 1;
                    }
                    break;
                case 3:
                    printf("Exiting the system.\n");
                    exit(0);
                    break;
                default:
                    printf("Invalid choice, try again.\n");
            }
        } else {
            int choice;
            printf("\n--- Welcome, %s ---\n", logged_in_username);
            printf("1. Book Ticket\n");
            printf("2. Cancel Ticket\n");
            printf("3. Check Bus Status\n");
            printf("4. Logout\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    book_ticket(logged_in_username);
                    break;
                case 2:
                    cancel_ticket(logged_in_username);
                    break;
                case 3:
                    check_bus_status();
                    break;
                case 4:
                    printf("Logging out.\n");
                    is_logged_in = 0;
                    break;
                default:
                    printf("Invalid choice, try again.\n");
            }
        }
    }

    return 0;
}
