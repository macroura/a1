# AI Disclosure — A1: Extracting IPv4 Addresses from Noisy Text

**Student:** Christopher Brush  **Course:** EECS 581, Fall 2026

## 1. General disclosure

- **Tool:** Claude Sonnet 5 (Anthropic), used through the claude.ai chat interface.
- **Date consulted:** September 27, 2026.
- **Language:** C++ (std::string prototype), C++11.
- **Test environment:** Ubuntu (WSL), GCC 13.3.0, GNU Make 4.3.

## 2. Prompts used (exact)

**Prompt 1:** I wrote my own detailed specification for the AI, asked it to explain its design before showing code, and asked for tests. The exact text:

> I'm writing a C++ (C++11) program for a university software engineering course. I need help with one function, and I want to review your work critically, so please explain your design before showing code. Task: read one line of text and extract a single valid IPv4 address, optionally followed by a :port, from anywhere in that line. Signature (must match exactly): bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort); On success outAddress is the 32-bit value and outPort is the port, or -1 if none. On failure outAddress = 0 and outPort = -1. Rules: - Only digits, '.', and ':' can ever be part of an address. All other characters are garbage and act as separators. - A candidate token must match the grammar in full: no partial matches, no trimming, and no searching for a valid piece inside a longer run of those characters. - Grammar: octet.octet.octet.octet with an optional :port. - octet: 1-3 digits, value 0-255, no leading zero unless the value is exactly 0. - port: 1-5 digits, value 0-65535, same leading-zero rule. - If a colon is present, the port must be fully valid, or the whole match (address included) is rejected. - Reject: wrong octet count, empty octet, out-of-range values, disallowed leading zeros, a second colon, a colon not right after the 4th octet, and a stray '.' or ':' directly adjacent to an otherwise valid address (e.g. "192.168.1.1." is invalid). - Example: "192a168.1.1.1" should yield 168.1.1.1, because "192" and "168.1.1.1" are separate tokens. Restrictions: - All digit-to-number conversion must be my own hand-written code. - Not allowed: atoi/strtol/stoi/sscanf and similar, inet_pton/inet_aton/inet_addr, and any regex. - std::isdigit is fine. Also write a small main() that loops with the prompt "Enter a string (or 'END' to quit): " until the user enters END (case-sensitive), then prints "Program terminated.". On success print: Extracted IPv4 address: A.B.C.D (decimal value: N, port: P), where P is the number or the word none. On failure print: Invalid input: no valid IPv4 address found Before the code, tell me: 1. How you will tokenize the line, and what you will do if several tokens are valid. 2. The three or four mistakes a parser like this most commonly gets wrong. Then give the code, then a table-driven test list with expected results, including boundary values, leading zeros, and adjacent-punctuation cases.

This prompt carried the requirements, grammar, restrictions, and output format from my own specification. The one thing it left open was which token to report when several are valid; I asked the AI to state its choice (see section 4.3). Everything after it (prompts 2–4 and my own testing in section 6) was my attempt to review and verify the result independently.

**Prompt 2:** "how to test and what are potential changes i could request as the assignment says"
(the AI answered with a testing plan and a list of possible follow-up requests).

**Prompt 3:** "simply give me some tests and tell me exactly how to run"
(the AI supplied 13 manual test inputs with expected results, plus exact commands).

**Prompt 4:** I pasted my test log (section 6) and asked the AI to fix the log to show what I did.

## 3. Code attribution

| File | Author | Notes |
|---|---|---|
| `ipv4.h`, `ipv4.cpp` (`extractIPv4`) | AI-generated (Claude) | Unmodified. I temporarily disabled one line for a mutation test and restored it. |
| `main.cpp` (input loop, output formatting) | AI-generated (Claude) | Unmodified |
| `tests/test_ipv4.cpp` | 56 cases AI-generated; **2 cases added by me** | Now 58 cases |
| `tests/sample_input.txt`, `Makefile`, `README.md` | AI-generated (Claude) | Unmodified |

**Student-written code:** two table-driven test cases in `tests/test_ipv4.cpp` (see section 6, item 5). The two inputs came from the manual test list the AI suggested. I typed them into the test table and ran them.

## 4. Design decisions and ambiguities in the handout

1. **Tokenization.** A candidate token is a maximal run of `0-9 . :`. Any other character ends a token. This is why `192a168.1.1.1` yields `168.1.1.1` (the `192` is its own failing token), while `192.168.1.1.` fails (the stray `.` is part of the same token, and the token must match the grammar in full).
2. **No partial matches.** Each token is validated as a whole, with no trimming and no search for a valid substring.
3. **Multiple valid tokens on one line.** The handout says "exactly one valid address per line" but not which one. The AI chose the **first valid token, left to right**, skipping invalid tokens before it (`999.1.1.1 8.8.8.8` gives `8.8.8.8`). This is an assumption, not something the handout specifies.
4. **Port rules.** 1–5 digits, 0–65535, no leading zero unless exactly `0`. A colon with a bad or missing port rejects the whole token, address included.
5. **Digit limits.** More than 3 digits (octet) or 5 digits (port) fails immediately, so the accumulator cannot overflow. This limit and the leading-zero check are independent guards.
6. **Failure outputs.** `outAddress` is set to 0 and `outPort` to -1 at the start and only written on success.
7. **Extras in main.** EOF exits cleanly, a trailing `\r` is stripped, and `END` is case-sensitive.
8. **Restrictions.** No `ato*`, `strto*`, `sto*`, `sscanf`, `inet_*`, or regex. Digit accumulation is by hand. Only `std::isdigit` is used from the allowed classification functions.

## 5. AI-side verification (in the AI's environment)

The AI built and ran the code before delivering it: 56/56 automated tests passed, the sample run matched the handout, and it compiled cleanly with `-Wall -Wextra -Wpedantic`. **Caveat:** the same AI wrote the code and these tests, so they can share blind spots. This is why I ran my own checks below.

## 6. Student testing log (my own checks)

Environment: Ubuntu (WSL), GCC 13.3.0, GNU Make 4.3.

1. **Automated suite.** `make test` gave **56/56 passed**.
2. **Demo run.** `make demo` compiled and ran. All sample inputs produced the expected addresses, decimal values, ports, and invalid-input messages.
3. **Interactive tests.** I ran 13 inputs by hand, and all matched the expected table:
   `12:34:56.789 host 10.0.0.1` · `10.0.0.1/24` · `v1.2.3.4` · `1.2.3.4:65535:` · `1.2.3.4-5.6.7.8` · `3.14159` · `(1.2.3.4)` · `1.2.3.4:0000000000000000` · `0.0.0.0:0` · `255.255.255.255:65535` · `1.2.3.4:65536` · `001.2.3.4` · `1.2.3.4.`
4. **Mutation / failure check.** I temporarily commented out this line in `ipv4.cpp`:
   `if (digits > 1 && s[start] == '0') return false;`
   With it disabled, the suite gave **49/56**. Seven leading-zero tests failed, confirming that the tests detect this bug. Each was expected to be rejected, but the mutated code accepted it:

   | Input | Expected | Actual (check disabled) |
   |---|---|---|
   | `192.168.01.1` (handout sample) | Invalid | Accepted as 192.168.1.1 |
   | `01.2.3.4` | Invalid | Accepted as 1.2.3.4 |
   | `1.2.3.04` | Invalid | Accepted as 1.2.3.4 |
   | `000.1.1.1` | Invalid | Accepted as 0.1.1.1 |
   | `1.00.1.1` | Invalid | Accepted as 1.0.1.1 |
   | `1.2.3.4:00` | Invalid | Accepted with port 0 |
   | `1.2.3.4:080` | Invalid | Accepted with port 80 |

   I then restored the line and confirmed the suite was back to all passing. Note that `1.2.3.0004` was not among the failures, because the separate 3-digit limit already rejects it.
5. **Tests added.** I added two table-driven cases to `tests/test_ipv4.cpp`:
   - CIDR suffix noise: `10.0.0.1/24`
   - Timestamp and host prefix: `12:34:56.789 host 10.0.0.1`

   Result after adding them: **58/58 passed**.
6. **Sanitizers.** I ran:
   `g++ -std=c++11 -g -fsanitize=address,undefined tests/test_ipv4.cpp ipv4.cpp -o t && ./t`
   Result: **58/58 passed**, with no AddressSanitizer or UndefinedBehaviorSanitizer errors.

**Observations**
- The implementation correctly rejects leading-zero octets and ports.
- It correctly handles noisy text, CIDR suffixes, parentheses, hyphens, port boundaries, and invalid formats.
- The ZIP was not needed because the project was already extracted. `git` initially failed only because the directory was not yet a Git repository.

**Changes made:** two tests added to `tests/test_ipv4.cpp`. The leading-zero check was temporarily disabled and then restored. **No production-code fix was required.**

**Limits of this review:** the code and most tests came from the same AI, and my manual test inputs came from a list the AI suggested. I did not write an independent second implementation or do differential testing.

## 7. Verification statement

**[STUDENT: check a box only if it is true for you.]**

- [ ] I have read and understand every line of the submitted code.
- [ ] I ran the code and tests and they work as described in section 6.
- **Known limitations:** (a) if a line has several valid tokens, only the first is reported (assumption, section 4.3); (b) IPv6 and other address forms are out of scope; (c) no bugs were found, so no unresolved bugs are known, but see the limits in section 6.
