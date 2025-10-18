# MiniTel
A mini telephone directory app, a project for my academics using only C and modules.


Basic System Architechture
┌──────────────────────────┐
│        UI Layer          │ ← handles menus, colors, inputs
├──────────────────────────┤
│   Core Logic Layer       │ ← add, delete, restore, search
├──────────────────────────┤
│   Data Structure Layer   │ ← array + hash map implementation
├──────────────────────────┤
│   Persistence Layer      │ ← CSV read/write + log management
└──────────────────────────┘

Usage
-----
Build with gcc (example):

```powershell
gcc -Wall -Wextra main.c menu.c operations.c directory.c hashmap.c -o minitel
```

Run (ASCII-safe mode):

```powershell
.\minitel
```

Run with 'fancy' Unicode/emoji mode (may garble on some terminals):

```powershell
.\minitel --fancy
```

Notes
-----
- The default UI is ASCII-safe to avoid garbled Unicode on some Windows terminals.
- Use `--fancy` on modern terminals that support UTF-8 and emoji for a richer display.
- If colors look off on Windows, ensure your terminal supports ANSI escapes (Windows 10+ or Windows Terminal recommended).
