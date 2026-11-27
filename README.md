# Railway Reservation Management System (C)

A console-based railway reservation project written in **C** as a **Data Structures & Algorithms (DSA)** mini project.

**Main DSA concepts used:** Arrays, Searching, Sorting

---

## Features / Modules (all in one file: `railway.c`)

| Module | Name | What it does |
|:------:|------|--------------|
| 1 | Train Details Module | Stores train number, name, source, destination, departure time, fare and seat availability in arrays. Add and display trains. |
| 2 | Ticket Booking Module | Books tickets, assigns a seat number and a unique PNR, reduces available seats, and puts passengers on a **waiting list** when the train is full. |
| 3 | Cancellation and Search Module | Cancels tickets, frees the seat, and **promotes the first waiting passenger**. Searches trains by number or route, and passengers by PNR. |
| 4 | Sorting and Reports Module | Sorts trains by number, departure time or fare. Shows a train-wise booking report and an all-tickets report. |

---

## DSA Concepts Mapping

| Concept | Where it is used |
|---------|------------------|
| **Arrays** | `trains[]` (array of Train), `tickets[]` (array of Ticket), `seatMap[]` (seat array inside each train) |
| **Linear Search** | Search train by number (`findTrainIndex`) and by route (`searchTrainByRoute`) |
| **Binary Search** | Search passenger by PNR (`findTicketIndex`). Tickets are stored in increasing PNR order, so the array is always sorted. |
| **Bubble Sort** | `sortTrains()` sorts the trains array by train number, departure time or fare |

---

## How to Compile and Run

### Using GCC (Linux / macOS / MinGW on Windows)
```bash
gcc railway.c -o railway
./railway          # on Windows: railway.exe
```

### Using Code::Blocks / Dev-C++ / Turbo C++
Open `railway.c`, then Build and Run.

---

## Sample Menu

```
==================== MAIN MENU ====================
 1. Train Details Module
 2. Ticket Booking Module
 3. Cancellation and Search Module
 4. Sorting and Reports Module
 0. Exit
```

The program starts with **5 sample trains** pre-loaded (with few seats), so you can test booking, the waiting list and cancellation immediately.

### Quick test scenario
1. Go to **Module 2** and book 5 passengers on train `12423` (it has only 4 seats). The 5th passenger goes to the **waiting list**.
2. Go to **Module 3** and cancel one confirmed ticket by PNR. The waiting passenger is automatically **confirmed**.
3. Go to **Module 3** and search by PNR, train number, or route (e.g. `Delhi`).
4. Go to **Module 4** and sort by fare, then view the booking report.

---

## Rules Implemented

- PNR numbers start from **1001** and increase by 1 for every booking.
- Seat numbers are assigned to the **first free seat** in the train.
- Waiting list limit is **5 passengers per train**.
- Cancelling a confirmed ticket refunds **80%** of the fare. Cancelling a waiting-list ticket refunds **100%**.
- Limits: 50 trains, 500 tickets, up to 100 seats per train.
- Input is validated (numbers, time in `HH:MM` 24-hour format).

---

## Project Structure

```
railway_reservation/
 |-- railway.c    # complete source code (all 4 modules)
 |-- README.md    # this file
```

## Possible Improvements
- Save and load data using file handling (binary or text files)
- Multiple passengers in one booking
- Use faster sorting (Merge Sort / Quick Sort) for large data
- Date-wise train schedules and class-wise (Sleeper / AC) fares
