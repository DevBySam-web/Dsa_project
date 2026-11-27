/*
 * ============================================================================
 *  RAILWAY RESERVATION MANAGEMENT SYSTEM
 *  Main DSA Concepts : Arrays, Searching, Sorting
 * ============================================================================
 *  Module 1 : Train Details Module
 *             -> store train number, name, source, destination, timing,
 *                fare and seat availability in arrays
 *  Module 2 : Ticket Booking Module
 *             -> book tickets, assign seat / PNR, reduce available seats,
 *                handle waiting list
 *  Module 3 : Cancellation and Search Module
 *             -> cancel tickets, update seats (promote waiting list),
 *                search train by number / route, search passenger by PNR
 *  Module 4 : Sorting and Reports Module
 *             -> sort trains by number, departure time or fare,
 *                display booking reports
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ----------------------------- Constants -------------------------------- */
#define MAX_TRAINS       50
#define MAX_TICKETS      500
#define MAX_SEATS        100     /* upper limit of seats in one train        */
#define MAX_WAITING      5       /* waiting list limit per train             */
#define NAME_LEN         50
#define CITY_LEN         30
#define INPUT_LEN        100

#define ST_CANCELLED     0
#define ST_CONFIRMED     1
#define ST_WAITING       2

/* ----------------------------- Data types ------------------------------- */
typedef struct {
    int   number;                    /* train number                         */
    char  name[NAME_LEN];            /* train name                           */
    char  source[CITY_LEN];          /* source station                       */
    char  destination[CITY_LEN];     /* destination station                  */
    int   depMinutes;                /* departure time in minutes (0-1439)   */
    float fare;                      /* fare per passenger                   */
    int   totalSeats;                /* total seats                          */
    int   availSeats;                /* seats still available                */
    int   seatMap[MAX_SEATS];        /* seatMap[i] = PNR holding seat i+1    */
} Train;                             /* (0 means the seat is free)           */

typedef struct {
    int   pnr;                       /* unique PNR (always increasing)       */
    char  passenger[NAME_LEN];
    int   age;
    int   trainNumber;
    int   seatNo;                    /* 0 when on waiting list / cancelled   */
    float fare;
    int   status;                    /* ST_CANCELLED / CONFIRMED / WAITING   */
} Ticket;

/* ----------------------------- Global arrays ---------------------------- */
Train  trains[MAX_TRAINS];
int    trainCount = 0;

Ticket tickets[MAX_TICKETS];         /* kept sorted by PNR (appended in order)*/
int    ticketCount = 0;
int    nextPNR = 1001;

/* ======================== Utility / helper functions ===================== */
void readLine(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
}

int readInt(const char *prompt, int min, int max)
{
    char buf[INPUT_LEN];
    char *end;
    long v;
    while (1) {
        readLine(prompt, buf, sizeof(buf));
        v = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && v >= min && v <= max)
            return (int)v;
        printf("  Invalid input! Enter a number between %d and %d.\n", min, max);
    }
}

float readFloat(const char *prompt, float min, float max)
{
    char buf[INPUT_LEN];
    char *end;
    float v;
    while (1) {
        readLine(prompt, buf, sizeof(buf));
        v = strtof(buf, &end);
        if (end != buf && *end == '\0' && v >= min && v <= max)
            return v;
        printf("  Invalid input! Enter a value between %.0f and %.0f.\n", min, max);
    }
}

int readTime(const char *prompt)
{
    char buf[INPUT_LEN];
    int h, m;
    while (1) {
        readLine(prompt, buf, sizeof(buf));
        if (sscanf(buf, "%d:%d", &h, &m) == 2 &&
            h >= 0 && h <= 23 && m >= 0 && m <= 59)
            return h * 60 + m;
        printf("  Invalid time! Use 24-hour format HH:MM (e.g. 18:45).\n");
    }
}

void formatTime(int minutes, char *out)
{
    sprintf(out, "%02d:%02d", minutes / 60, minutes % 60);
}

/* case-insensitive substring search: is 'needle' inside 'hay'? */
int containsIgnoreCase(const char *hay, const char *needle)
{
    size_t i, j, hl = strlen(hay), nl = strlen(needle);
    if (nl == 0) return 1;
    for (i = 0; i + nl <= hl; i++) {
        for (j = 0; j < nl; j++)
            if (tolower((unsigned char)hay[i + j]) !=
                tolower((unsigned char)needle[j]))
                break;
        if (j == nl) return 1;
    }
    return 0;
}

void pressEnter(void)
{
    char buf[INPUT_LEN];
    readLine("\nPress ENTER to continue...", buf, sizeof(buf));
}

const char *statusText(int s)
{
    if (s == ST_CONFIRMED) return "CONFIRMED";
    if (s == ST_WAITING)   return "WAITING";
    return "CANCELLED";
}

/* ======================================================================== */
/*  MODULE 1 : TRAIN DETAILS MODULE                                         */
/* ======================================================================== */

/* Linear search of the trains array by train number. Returns index or -1. */
int findTrainIndex(int number)
{
    int i;
    for (i = 0; i < trainCount; i++)
        if (trains[i].number == number)
            return i;
    return -1;
}

void addTrainData(int number, const char *name, const char *src,
                  const char *dst, int dep, float fare, int seats)
{
    Train *t = &trains[trainCount];
    memset(t, 0, sizeof(Train));
    t->number     = number;
    strncpy(t->name, name, NAME_LEN - 1);
    strncpy(t->source, src, CITY_LEN - 1);
    strncpy(t->destination, dst, CITY_LEN - 1);
    t->depMinutes = dep;
    t->fare       = fare;
    t->totalSeats = seats;
    t->availSeats = seats;
    trainCount++;
}

void addTrain(void)
{
    int number, dep, seats;
    float fare;
    char name[NAME_LEN], src[CITY_LEN], dst[CITY_LEN];

    if (trainCount >= MAX_TRAINS) {
        printf("\n  Train list is full!\n");
        return;
    }
    printf("\n--- Add New Train ---\n");
    number = readInt("Train number (1-99999): ", 1, 99999);
    if (findTrainIndex(number) != -1) {
        printf("  A train with number %d already exists!\n", number);
        return;
    }
    readLine("Train name            : ", name, sizeof(name));
    readLine("Source station        : ", src, sizeof(src));
    readLine("Destination station   : ", dst, sizeof(dst));
    dep   = readTime("Departure time (HH:MM): ");
    fare  = readFloat("Fare per passenger (Rs): ", 1, 100000);
    seats = readInt("Total seats (1-100)    : ", 1, MAX_SEATS);

    addTrainData(number, name, src, dst, dep, fare, seats);
    printf("\n  Train %d (%s) added successfully!\n", number, name);
}

void printTrainHeader(void)
{
    printf("\n%-7s %-24s %-12s %-12s %-6s %-9s %-6s %-5s\n",
           "TrainNo", "Train Name", "From", "To", "Dep", "Fare(Rs)",
           "Avail", "WL");
    printf("------------------------------------------------------------"
           "-------------------------\n");
}

int waitingCount(int trainNumber)
{
    int i, c = 0;
    for (i = 0; i < ticketCount; i++)
        if (tickets[i].trainNumber == trainNumber &&
            tickets[i].status == ST_WAITING)
            c++;
    return c;
}

void printTrainRow(const Train *t)
{
    char tm[8];
    formatTime(t->depMinutes, tm);
    printf("%-7d %-24s %-12s %-12s %-6s %-9.2f %-6d %-5d\n",
           t->number, t->name, t->source, t->destination, tm, t->fare,
           t->availSeats, waitingCount(t->number));
}

void displayTrains(void)
{
    int i;
    if (trainCount == 0) {
        printf("\n  No trains available.\n");
        return;
    }
    printTrainHeader();
    for (i = 0; i < trainCount; i++)
        printTrainRow(&trains[i]);
}

void loadSampleData(void)
{
    addTrainData(12956, "Jaipur-Mumbai SF Exp",  "Jaipur",  "Mumbai",  1115, 1450.00f, 6);
    addTrainData(12015, "Ajmer Shatabdi",        "Delhi",   "Ajmer",    365, 1100.00f, 5);
    addTrainData(12423, "Rajdhani Express",      "Delhi",   "Guwahati", 1000, 2350.50f, 4);
    addTrainData(12009, "Shatabdi Express",      "Mumbai",  "Ahmedabad", 380, 980.75f, 5);
    addTrainData(12951, "Mumbai Rajdhani",       "Mumbai",  "Delhi",   1005, 2100.00f, 6);
}

void module1_TrainDetails(void)
{
    int ch;
    do {
        printf("\n========== MODULE 1 : TRAIN DETAILS ==========\n");
        printf(" 1. Add new train\n");
        printf(" 2. Display all trains\n");
        printf(" 0. Back to main menu\n");
        ch = readInt("Enter choice: ", 0, 2);
        switch (ch) {
            case 1: addTrain();     break;
            case 2: displayTrains(); break;
        }
        if (ch != 0) pressEnter();
    } while (ch != 0);
}

/* ======================================================================== */
/*  MODULE 2 : TICKET BOOKING MODULE                                        */
/* ======================================================================== */

/* Find first free seat in the train's seat array. Returns seat no or 0. */
int assignSeat(Train *t, int pnr)
{
    int i;
    for (i = 0; i < t->totalSeats; i++) {
        if (t->seatMap[i] == 0) {
            t->seatMap[i] = pnr;
            t->availSeats--;             /* reduce available seats */
            return i + 1;
        }
    }
    return 0;
}

void printTicket(const Ticket *tk)
{
    int ti = findTrainIndex(tk->trainNumber);
    char tm[8] = "--:--";
    printf("\n  +------------------ E-TICKET ------------------+\n");
    printf("   PNR        : %d\n", tk->pnr);
    printf("   Passenger  : %s (Age %d)\n", tk->passenger, tk->age);
    if (ti != -1) {
        formatTime(trains[ti].depMinutes, tm);
        printf("   Train      : %d - %s\n", trains[ti].number, trains[ti].name);
        printf("   Route      : %s -> %s\n", trains[ti].source, trains[ti].destination);
        printf("   Departure  : %s\n", tm);
    }
    if (tk->status == ST_CONFIRMED)
        printf("   Seat No    : %d\n", tk->seatNo);
    else if (tk->status == ST_WAITING)
        printf("   Seat No    : Not assigned (Waiting list)\n");
    printf("   Fare       : Rs %.2f\n", tk->fare);
    printf("   Status     : %s\n", statusText(tk->status));
    printf("  +----------------------------------------------+\n");
}

void bookTicket(void)
{
    int tn, ti, age, seat;
    char name[NAME_LEN];
    Ticket *tk;

    if (trainCount == 0) {
        printf("\n  No trains available. Add trains first (Module 1).\n");
        return;
    }
    if (ticketCount >= MAX_TICKETS) {
        printf("\n  Ticket database is full!\n");
        return;
    }

    displayTrains();
    printf("\n--- Book Ticket ---\n");
    tn = readInt("Enter train number: ", 1, 99999);
    ti = findTrainIndex(tn);
    if (ti == -1) {
        printf("  Train %d not found!\n", tn);
        return;
    }

    Train *t = &trains[ti];
    if (t->availSeats == 0 && waitingCount(tn) >= MAX_WAITING) {
        printf("  Train is full and the waiting list is also full (%d). "
               "Cannot book.\n", MAX_WAITING);
        return;
    }

    readLine("Passenger name: ", name, sizeof(name));
    if (strlen(name) == 0) {
        printf("  Name cannot be empty!\n");
        return;
    }
    age = readInt("Passenger age : ", 1, 120);

    tk = &tickets[ticketCount];
    memset(tk, 0, sizeof(Ticket));
    tk->pnr = nextPNR++;
    strncpy(tk->passenger, name, NAME_LEN - 1);
    tk->age = age;
    tk->trainNumber = tn;
    tk->fare = t->fare;

    if (t->availSeats > 0) {
        seat = assignSeat(t, tk->pnr);
        tk->seatNo = seat;
        tk->status = ST_CONFIRMED;
        printf("\n  Ticket CONFIRMED!\n");
    } else {
        tk->seatNo = 0;
        tk->status = ST_WAITING;
        printf("\n  Train is full. You are added to the WAITING LIST "
               "(position %d).\n", waitingCount(tn) + 1);
    }
    ticketCount++;
    printTicket(tk);
}

void module2_TicketBooking(void)
{
    int ch;
    do {
        printf("\n========== MODULE 2 : TICKET BOOKING ==========\n");
        printf(" 1. Book a ticket\n");
        printf(" 2. Show seat availability of all trains\n");
        printf(" 0. Back to main menu\n");
        ch = readInt("Enter choice: ", 0, 2);
        switch (ch) {
            case 1: bookTicket();    break;
            case 2: displayTrains(); break;
        }
        if (ch != 0) pressEnter();
    } while (ch != 0);
}

/* ======================================================================== */
/*  MODULE 3 : CANCELLATION AND SEARCH MODULE                               */
/* ======================================================================== */

/* Binary search on tickets[] (sorted by PNR). Returns index or -1. */
int findTicketIndex(int pnr)
{
    int low = 0, high = ticketCount - 1, mid;
    while (low <= high) {
        mid = (low + high) / 2;
        if (tickets[mid].pnr == pnr)
            return mid;
        else if (tickets[mid].pnr < pnr)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

/* After a confirmed seat is freed, give it to the first waiting passenger */
void promoteWaiting(Train *t)
{
    int i;
    for (i = 0; i < ticketCount; i++) {
        if (tickets[i].trainNumber == t->number &&
            tickets[i].status == ST_WAITING) {
            tickets[i].seatNo = assignSeat(t, tickets[i].pnr);
            tickets[i].status = ST_CONFIRMED;
            printf("\n  Waiting-list passenger '%s' (PNR %d) is now CONFIRMED "
                   "with seat %d.\n", tickets[i].passenger, tickets[i].pnr,
                   tickets[i].seatNo);
            return;
        }
    }
}

void cancelTicket(void)
{
    int pnr, idx, ti;
    float refund;
    Ticket *tk;

    printf("\n--- Cancel Ticket ---\n");
    pnr = readInt("Enter PNR: ", 1, 999999);
    idx = findTicketIndex(pnr);
    if (idx == -1) {
        printf("  PNR %d not found!\n", pnr);
        return;
    }
    tk = &tickets[idx];
    if (tk->status == ST_CANCELLED) {
        printf("  Ticket with PNR %d is already cancelled.\n", pnr);
        return;
    }
    ti = findTrainIndex(tk->trainNumber);

    if (tk->status == ST_CONFIRMED) {
        refund = tk->fare * 0.80f;               /* 20% cancellation charge */
        if (ti != -1) {
            trains[ti].seatMap[tk->seatNo - 1] = 0;   /* free the seat      */
            trains[ti].availSeats++;                  /* update seats       */
        }
        tk->status = ST_CANCELLED;
        tk->seatNo = 0;
        printf("\n  Ticket PNR %d cancelled. Refund: Rs %.2f (80%% of fare).\n",
               pnr, refund);
        if (ti != -1) promoteWaiting(&trains[ti]);
    } else {                                      /* waiting-list ticket    */
        tk->status = ST_CANCELLED;
        printf("\n  Waiting-list ticket PNR %d cancelled. Full refund: "
               "Rs %.2f.\n", pnr, tk->fare);
    }
}

void searchTrainByNumber(void)
{
    int tn, ti;
    printf("\n--- Search Train by Number ---\n");
    tn = readInt("Enter train number: ", 1, 99999);
    ti = findTrainIndex(tn);                      /* linear search */
    if (ti == -1) {
        printf("  Train %d not found.\n", tn);
        return;
    }
    printTrainHeader();
    printTrainRow(&trains[ti]);
}

void searchTrainByRoute(void)
{
    char src[CITY_LEN], dst[CITY_LEN];
    int i, found = 0;
    printf("\n--- Search Train by Route ---\n");
    readLine("Source (blank = any)     : ", src, sizeof(src));
    readLine("Destination (blank = any): ", dst, sizeof(dst));
    for (i = 0; i < trainCount; i++) {            /* linear search */
        if (containsIgnoreCase(trains[i].source, src) &&
            containsIgnoreCase(trains[i].destination, dst)) {
            if (!found) printTrainHeader();
            printTrainRow(&trains[i]);
            found = 1;
        }
    }
    if (!found) printf("  No train found for this route.\n");
}

void searchPassengerByPNR(void)
{
    int pnr, idx;
    printf("\n--- Search Passenger by PNR ---\n");
    pnr = readInt("Enter PNR: ", 1, 999999);
    idx = findTicketIndex(pnr);                   /* binary search */
    if (idx == -1)
        printf("  PNR %d not found.\n", pnr);
    else
        printTicket(&tickets[idx]);
}

void module3_CancelSearch(void)
{
    int ch;
    do {
        printf("\n===== MODULE 3 : CANCELLATION AND SEARCH =====\n");
        printf(" 1. Cancel ticket\n");
        printf(" 2. Search train by number\n");
        printf(" 3. Search train by route\n");
        printf(" 4. Search passenger by PNR\n");
        printf(" 0. Back to main menu\n");
        ch = readInt("Enter choice: ", 0, 4);
        switch (ch) {
            case 1: cancelTicket();        break;
            case 2: searchTrainByNumber(); break;
            case 3: searchTrainByRoute();  break;
            case 4: searchPassengerByPNR(); break;
        }
        if (ch != 0) pressEnter();
    } while (ch != 0);
}

/* ======================================================================== */
/*  MODULE 4 : SORTING AND REPORTS MODULE                                   */
/* ======================================================================== */

/* returns >0 if train a must come after train b for the chosen key */
int compareTrains(const Train *a, const Train *b, int key)
{
    if (key == 1) return a->number - b->number;
    if (key == 2) return a->depMinutes - b->depMinutes;
    if (a->fare > b->fare) return 1;
    if (a->fare < b->fare) return -1;
    return 0;
}

/* Bubble sort on the trains array (with early-exit optimisation) */
void sortTrains(int key)
{
    int i, j, swapped;
    Train temp;
    for (i = 0; i < trainCount - 1; i++) {
        swapped = 0;
        for (j = 0; j < trainCount - i - 1; j++) {
            if (compareTrains(&trains[j], &trains[j + 1], key) > 0) {
                temp = trains[j];
                trains[j] = trains[j + 1];
                trains[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

void sortMenu(void)
{
    int key;
    if (trainCount == 0) {
        printf("\n  No trains to sort.\n");
        return;
    }
    printf("\n--- Sort Trains ---\n");
    printf(" 1. By train number\n 2. By departure time\n 3. By fare\n");
    key = readInt("Sort by: ", 1, 3);
    sortTrains(key);
    printf("\n  Trains sorted by %s:",
           key == 1 ? "train number" : key == 2 ? "departure time" : "fare");
    displayTrains();
}

void bookingReport(void)
{
    int i, j, conf, wait, canc;
    float revenue, grandRevenue = 0;
    int totC = 0, totW = 0, totX = 0;

    if (trainCount == 0) {
        printf("\n  No trains available.\n");
        return;
    }
    printf("\n================= BOOKING REPORT =================\n");
    printf("%-7s %-24s %-5s %-5s %-5s %-6s %-10s\n",
           "TrainNo", "Train Name", "Conf", "Wait", "Canc", "Free", "Revenue");
    printf("---------------------------------------------------------------"
           "--\n");
    for (i = 0; i < trainCount; i++) {
        conf = wait = canc = 0;
        revenue = 0;
        for (j = 0; j < ticketCount; j++) {
            if (tickets[j].trainNumber != trains[i].number) continue;
            if (tickets[j].status == ST_CONFIRMED) {
                conf++;
                revenue += tickets[j].fare;
            } else if (tickets[j].status == ST_WAITING) {
                wait++;
            } else {
                canc++;
            }
        }
        printf("%-7d %-24s %-5d %-5d %-5d %-6d %-10.2f\n",
               trains[i].number, trains[i].name, conf, wait, canc,
               trains[i].availSeats, revenue);
        totC += conf; totW += wait; totX += canc;
        grandRevenue += revenue;
    }
    printf("---------------------------------------------------------------"
           "--\n");
    printf("%-32s %-5d %-5d %-5d %-6s %-10.2f\n",
           "TOTAL", totC, totW, totX, "", grandRevenue);
}

void allTicketsReport(void)
{
    int i;
    if (ticketCount == 0) {
        printf("\n  No tickets booked yet.\n");
        return;
    }
    printf("\n%-6s %-20s %-4s %-8s %-5s %-10s %-10s\n",
           "PNR", "Passenger", "Age", "TrainNo", "Seat", "Fare", "Status");
    printf("------------------------------------------------------------"
           "------\n");
    for (i = 0; i < ticketCount; i++) {
        printf("%-6d %-20s %-4d %-8d ", tickets[i].pnr, tickets[i].passenger,
               tickets[i].age, tickets[i].trainNumber);
        if (tickets[i].seatNo > 0) printf("%-5d ", tickets[i].seatNo);
        else                       printf("%-5s ", "-");
        printf("%-10.2f %-10s\n", tickets[i].fare, statusText(tickets[i].status));
    }
}

void module4_SortReports(void)
{
    int ch;
    do {
        printf("\n======== MODULE 4 : SORTING AND REPORTS ========\n");
        printf(" 1. Sort trains (number / departure time / fare)\n");
        printf(" 2. Booking report (train-wise summary)\n");
        printf(" 3. All tickets report\n");
        printf(" 0. Back to main menu\n");
        ch = readInt("Enter choice: ", 0, 3);
        switch (ch) {
            case 1: sortMenu();         break;
            case 2: bookingReport();    break;
            case 3: allTicketsReport(); break;
        }
        if (ch != 0) pressEnter();
    } while (ch != 0);
}

/* ======================================================================== */
/*  MAIN                                                                    */
/* ======================================================================== */
int main(void)
{
    int ch;

    loadSampleData();   /* a few trains so the program can be tried at once */

    printf("\n****************************************************\n");
    printf("*     RAILWAY RESERVATION MANAGEMENT SYSTEM        *\n");
    printf("*     DSA Project : Arrays, Searching, Sorting     *\n");
    printf("****************************************************\n");

    do {
        printf("\n==================== MAIN MENU ====================\n");
        printf(" 1. Train Details Module\n");
        printf(" 2. Ticket Booking Module\n");
        printf(" 3. Cancellation and Search Module\n");
        printf(" 4. Sorting and Reports Module\n");
        printf(" 0. Exit\n");
        ch = readInt("Enter your choice: ", 0, 4);
        switch (ch) {
            case 1: module1_TrainDetails();  break;
            case 2: module2_TicketBooking(); break;
            case 3: module3_CancelSearch();  break;
            case 4: module4_SortReports();   break;
            case 0: printf("\nThank you for using the system. Goodbye!\n"); break;
        }
    } while (ch != 0);

    return 0;
}
