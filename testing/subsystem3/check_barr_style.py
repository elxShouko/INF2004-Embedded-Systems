"""Check the repeatable subset of the Member 3 Barr-C review.

This is a guard against regression, not a complete C parser or certification.
Public compatibility exceptions and manual rules live in docs/buddy3/BARR_COMPLIANCE.md.
"""

from pathlib import Path
import re
import sys


def check(root):
    errors = []
    count = 0
    longest_function = 0
    definition = re.compile(
        r"^(?:static\s+)?(?:EXPORT\s+|LOCAL\s+)?\w+\s+"
        r"(?P<name>\w+)\s*\([^;{}]*\)\s*\n\{", re.M
    )
    for path in sorted(root.rglob("*")):
        if path.suffix not in {".c", ".h"}:
            continue
        count += 1
        data = path.read_bytes()
        text = data.decode("utf-8")
        if b"\r" in data or b"\t" in data:
            errors.append(f"{path}: CR or tab character (3.5/3.6)")
        for number, line in enumerate(text.splitlines(), 1):
            if len(line) > 80:
                errors.append(f"{path}:{number}: exceeds 80 columns (1.2.a)")
        if not text.startswith("/** @file"):
            errors.append(f"{path}: missing module documentation (2.2.h)")
        if not text.endswith("/*** End of file. ***/\n\n"):
            errors.append(f"{path}: missing end-of-file comment/blank line (3.3.c)")
        if re.search(r"\bwhile\s*\(\s*(?:1|true)\s*\)", text):
            errors.append(f"{path}: nonstandard infinite loop (8.4.c)")
        if re.search(r"\b(?:u?int\d+_t|unsigned int)\s+[ij]\b", text):
            errors.append(f"{path}: short loop variable (7.1.e)")
        if path.suffix == ".h":
            if "#ifndef " not in text or "#define " not in text:
                errors.append(f"{path}: missing include guard (4.2.b)")
            continue
        paired = path.with_suffix(".h")
        if not paired.is_file() or f'#include "{paired.name}"' not in text:
            errors.append(f"{path}: missing matching module header (4.2.a/4.3.c)")
        for match in definition.finditer(text):
            end = text.index("\n}", match.end())
            length = text[match.start():end].count("\n") + 2
            longest_function = max(longest_function, length)
            if length > 100:
                errors.append(f"{path}: {match['name']} exceeds 100 lines (6.2.a)")
            prefix = text[:match.start()].rstrip()
            if not prefix.endswith("*/") or "@brief" not in prefix[prefix.rfind("/**"):]:
                errors.append(f"{path}: {match['name']} lacks Doxygen brief (2.2.h)")
    print(f"Checked {count} Member 3 C/header files; longest function: {longest_function} lines.")
    for error in errors:
        print(error)
    print("PASS: checked rules" if not errors else "FAIL: checked rules")
    print("Full conformance remains conditional on manual review and exception approval.")
    return bool(errors)


if __name__ == "__main__":
    repository = Path(__file__).resolve().parents[2]
    sys.exit(check(repository / "app_program/subsystem3"))
