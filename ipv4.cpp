// ipv4.cpp - extract one IPv4 address (with optional :port) from noisy text.
//
// Approach
//   1. Split the line into "candidate tokens": maximal runs of the only
//      characters that can ever belong to an address: digits, '.', ':'.
//      Every other character is garbage and acts as a separator.
//   2. Validate each whole token against the grammar
//          octet '.' octet '.' octet '.' octet [ ':' port ]
//      The token must match completely. There is no trimming and no searching
//      for a valid piece inside a longer token, so "192.168.1.1." and
//      "1.2.3.4.5" are rejected instead of being "fixed".
//   3. The first token that validates is the result.
//
// All digit accumulation is done by hand. No strto*/ato*/sscanf/inet_*/regex.

#include "ipv4.h"

#include <cctype>
#include <cstddef>

namespace {

bool isDigit(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) != 0;
}

bool isTokenChar(char c) {
    return isDigit(c) || c == '.' || c == ':';
}

// Reads 1..maxDigits digits starting at pos (never past end) as a decimal
// number with no leading zero unless the value is exactly "0", and checks
// value <= maxValue. On success stores the value in out and advances pos.
bool parseNumber(const std::string& s, std::size_t& pos, std::size_t end,
                 int maxDigits, long maxValue, long& out) {
    std::size_t start = pos;
    long value = 0;
    int digits = 0;

    while (pos < end && isDigit(s[pos])) {
        if (++digits > maxDigits) return false;  // too many digits
        value = value * 10 + (s[pos] - '0');     // <= 5 digits: cannot overflow
        ++pos;
    }

    if (digits == 0) return false;                        // empty field
    if (digits > 1 && s[start] == '0') return false;      // leading zero
    if (value > maxValue) return false;                   // out of range

    out = value;
    return true;
}

// Validates s[begin, end) as a complete address[:port] token.
bool validateToken(const std::string& s, std::size_t begin, std::size_t end,
                   unsigned long& address, int& port) {
    std::size_t pos = begin;
    unsigned long addr = 0;

    for (int i = 0; i < 4; ++i) {
        long octet = 0;
        if (!parseNumber(s, pos, end, 3, 255, octet)) return false;
        addr = (addr << 8) | static_cast<unsigned long>(octet);

        if (i < 3) {
            if (pos >= end || s[pos] != '.') return false;  // need exactly 3 dots
            ++pos;
        }
    }

    int parsedPort = -1;
    if (pos < end) {
        // Anything after the 4th octet must be ':' + a fully valid port.
        if (s[pos] != ':') return false;                    // e.g. trailing '.'
        ++pos;
        long p = 0;
        if (!parseNumber(s, pos, end, 5, 65535, p)) return false;
        parsedPort = static_cast<int>(p);
        if (pos != end) return false;                       // e.g. second ':' or '.'
    }

    address = addr;
    port = parsedPort;
    return true;
}

}  // namespace

bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    outAddress = 0;
    outPort = -1;

    std::size_t i = 0;
    while (i < str.size()) {
        if (!isTokenChar(str[i])) {  // garbage: skip
            ++i;
            continue;
        }

        std::size_t j = i;
        while (j < str.size() && isTokenChar(str[j])) ++j;  // token = [i, j)

        unsigned long addr = 0;
        int port = -1;
        if (validateToken(str, i, j, addr, port)) {
            outAddress = addr;
            outPort = port;
            return true;
        }
        i = j;  // token failed as a whole; move on to the next token
    }
    return false;
}
