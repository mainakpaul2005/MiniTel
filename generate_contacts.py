#!/usr/bin/env python3
"""
Generate a large contacts CSV using the existing `contacts.csv` as templates.
Writes `contacts_1M.csv` with 1,000,000 contact rows (plus header).
Streaming write so memory usage is small.
"""
import csv
import os

INPUT = "contacts.csv"
OUTPUT = "contacts_1M.csv"
TARGET = 1_000_000


def sanitize_email(name: str, idx: int) -> str:
    # Make a simple lowercase alnum+underscore base from the name
    base = "".join(c.lower() if c.isalnum() else "_" for c in name)
    base = base.strip("_") or "contact"
    return f"{base}.{idx}@example.com"


if not os.path.exists(INPUT):
    raise SystemExit(f"Input file '{INPUT}' not found in working directory.")

with open(INPUT, newline="", encoding="utf-8") as fin, open(OUTPUT, "w", newline="", encoding="utf-8") as fout:
    reader = csv.DictReader(fin)
    fieldnames = reader.fieldnames
    if not fieldnames:
        raise SystemExit("No header found in input CSV")

    writer = csv.DictWriter(fout, fieldnames=fieldnames, quoting=csv.QUOTE_MINIMAL)
    writer.writeheader()

    templates = [row for row in reader]
    if not templates:
        raise SystemExit("No template rows found in contacts.csv")

    n_templates = len(templates)

    # Stream output: create each row by cycling through templates
    for i in range(1, TARGET + 1):
        tpl = templates[(i - 1) % n_templates].copy()
        tpl["id"] = str(i)
        original_name = tpl.get("name", "Contact")
        tpl["name"] = f"{original_name} #{i}"
        # Ensure phone is unique-ish: +91 followed by 10-digit zero-padded index
        tpl["phone"] = f"+91{str(i).zfill(10)}"
        tpl["email"] = sanitize_email(original_name, i)
        # Keep address as-is
        writer.writerow(tpl)

print(f"Written {TARGET} contacts to {OUTPUT}")
