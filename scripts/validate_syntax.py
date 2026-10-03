#!/usr/bin/env python3
"""
Syntax, LaTeX, and Mermaid Validator for The DSA Handbook.
Scans all Markdown documentation files for:
1. Balanced code blocks
2. Display math $$ delimiters and LaTeX brace matching
3. Inline math $ delimiters and LaTeX brace matching
4. Local relative Markdown link integrity
5. Mermaid diagram quote balance and syntax hazards
"""

import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

def check_file(file_path):
    rel_path = file_path.relative_to(REPO_ROOT)
    file_dir = file_path.parent
    with open(file_path, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()

    lines = content.splitlines(keepends=True)
    errors = []

    # 1. Check code blocks & mermaid
    in_code = False
    code_lang = ""
    code_start = 0
    mermaid_lines = []
    mermaid_start = 0

    for idx, line in enumerate(lines):
        line_num = idx + 1
        stripped = line.strip()
        if stripped.startswith("```"):
            if not in_code:
                in_code = True
                code_lang = stripped[3:].strip()
                code_start = line_num
                if code_lang.lower() == "mermaid":
                    mermaid_lines = []
                    mermaid_start = line_num
            else:
                in_code = False
                if code_lang.lower() == "mermaid":
                    m_errs = validate_mermaid(mermaid_lines, mermaid_start, rel_path)
                    errors.extend(m_errs)
                code_lang = ""
            continue
        if in_code and code_lang.lower() == "mermaid":
            mermaid_lines.append((line_num, line))

    if in_code:
        errors.append(("CODE_BLOCK_UNCLOSED", f"{rel_path}:{code_start}", f"Code block not closed (started on line {code_start})"))

    # 2. Check LaTeX block $$ balance
    text_without_code = re.sub(r"```.*?```", "", content, flags=re.DOTALL)
    double_dollars = re.findall(r"(?<!\\)\$\$", text_without_code)
    if len(double_dollars) % 2 != 0:
        errors.append(("LATEX_BLOCK_UNBALANCED", f"{rel_path}", f"Odd number of $$ delimiters ({len(double_dollars)})"))

    # 3. Check LaTeX braces inside display math $$ ... $$
    display_math_blocks = re.findall(r"(?<!\\)\$\$(.*?)(?<!\\)\$\$", text_without_code, flags=re.DOTALL)
    for block in display_math_blocks:
        open_b = block.count("{") - block.count(r"\{")
        close_b = block.count("}") - block.count(r"\}")
        if open_b != close_b:
            errors.append(("LATEX_BRACES_MISMATCH", f"{rel_path}", f"Display math braces mismatch ({open_b} open vs {close_b} close)"))

    # 4. Check inline LaTeX $ ... $ on non-code lines
    in_code_block = False
    in_display_math = False
    for idx, line in enumerate(lines):
        line_num = idx + 1
        stripped = line.strip()
        if stripped.startswith("```"):
            in_code_block = not in_code_block
            continue
        if in_code_block:
            continue

        dd_count = len(re.findall(r"(?<!\\)\$\$", line))
        if dd_count % 2 != 0:
            in_display_math = not in_display_math
            continue
        if in_display_math:
            continue

        clean_line = re.sub(r"`[^`\n]*`", "", line)
        clean_line = re.sub(r"(?<!\\)\$\$.*?(?<!\\)\$\$", "", clean_line)
        single_dollars = re.findall(r"(?<!\\)\$", clean_line)
        if len(single_dollars) % 2 != 0:
            errors.append(("LATEX_INLINE_UNBALANCED", f"{rel_path}:{line_num}", f"Odd unescaped $ ({len(single_dollars)}) on line: {stripped[:60]}"))
        else:
            inline_math = re.findall(r"(?<!\\)\$(.*?)(?<!\\)\$", clean_line)
            for im in inline_math:
                ob = im.count("{") - im.count(r"\{")
                cb = im.count("}") - im.count(r"\}")
                if ob != cb:
                    errors.append(("LATEX_INLINE_BRACES", f"{rel_path}:{line_num}", f"Inline math brace mismatch in '${im}$'"))

    # 5. Check local relative links
    link_matches = re.finditer(r'\[([^\]]*)\]\(([^)]+)\)', text_without_code)
    for lm in link_matches:
        url = lm.group(2).strip()
        if url.startswith(("http://", "https://", "mailto:", "#", "ftp://")):
            continue
        clean_url = url.split("#")[0].split("?")[0]
        if not clean_url:
            continue
        
        target_path = (file_dir / clean_url).resolve()
        if not target_path.exists():
            errors.append(("BROKEN_LINK", f"{rel_path}", f"Link '{url}' -> target '{target_path.relative_to(REPO_ROOT) if REPO_ROOT in target_path.parents else target_path}' does not exist"))

    return errors

def validate_mermaid(lines, start_line, rel_path):
    issues = []
    for l_num, l_text in lines:
        s = l_text.strip()
        if not s or s.startswith("%%"):
            continue
        
        if s.count('"') % 2 != 0:
            issues.append(("MERMAID_UNBALANCED_QUOTES", f"{rel_path}:{l_num}", f"Unbalanced quotes in line: {s}"))
            
        if s.startswith("participant ") and " as " in s:
            alias = s.split(" as ", 1)[1].strip()
            if not alias.startswith('"') and ('(' in alias or ')' in alias):
                issues.append(("MERMAID_UNQUOTED_PARTICIPANT", f"{rel_path}:{l_num}", f"Unquoted parens in participant alias: {s}"))

    return issues

def main():
    md_files = sorted(list(REPO_ROOT.glob("docs/**/*.md")) + list(REPO_ROOT.glob("*.md")))
    all_errors = []
    for f in md_files:
        errs = check_file(f)
        all_errors.extend(errs)

    print(f"Total markdown files scanned: {len(md_files)}")
    print(f"Total syntax issues found: {len(all_errors)}\n")

    if all_errors:
        by_type = {}
        for cat, loc, msg in all_errors:
            by_type.setdefault(cat, []).append((loc, msg))

        for cat, items in by_type.items():
            print(f"=== {cat} ({len(items)}) ===")
            for loc, msg in items[:15]:
                print(f"  {loc}: {msg}")
            if len(items) > 15:
                print(f"  ... and {len(items)-15} more")
            print()
        sys.exit(1)
    else:
        print("All Markdown, LaTeX, and Mermaid syntax checks passed successfully!")
        sys.exit(0)

if __name__ == "__main__":
    main()
