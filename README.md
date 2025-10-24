Simple Telephone Directory (MiniTel)

This is a small command-line telephone directory written in C. It stores contacts in CSV format in `contacts.csv`.

Features
- Add contact (name, phone, email, address)
- List all contacts
- Search by name or phone (simple substring match)
- Delete contact by id

Limitations
- Update: fields are now quoted when needed. Commas and double-quotes in fields are supported: quotes are doubled inside quoted fields.

Build (Windows PowerShell)

If you have GCC (MinGW) installed, run:

powershell
    gcc -o MiniTel.exe main.c


Run

powershell
    .\MiniTel.exe


Usage
- Choose 1 to add a contact. The program will append to `contacts.csv`.
- Choose 2 to list all contacts.
- Choose 3 to search by name or phone.
- Choose 4 to delete by id (integer).
- Choose 0 to exit.

Notes
- The program creates `contacts.csv` in the current working directory.
- If you want CSV with commas in fields, you'll need a more advanced CSV writer.
