// Table-driven tests for extractIPv4. Build/run with `make test`.
#include "../ipv4.h"

#include <iostream>
#include <string>

struct Case {
    const char* name;
    const char* input;
    bool ok;
    unsigned long a, b, c, d;  // expected octets (ignored when !ok)
    int port;                  // expected port (-1 = none)
};

static const Case cases[] = {
    // ---- handout sample run ----
    {"sample: plain",            "connecting to 192.168.1.1 now",   true, 192,168,1,1,   -1},
    {"sample: port + suffix",    "server=10.0.0.255:8080end",       true, 10,0,0,255,    8080},
    {"sample: 192a168.1.1.1",    "192a168.1.1.1",                   true, 168,1,1,1,     -1},
    {"sample: trailing period",  "192.168.1.1.",                    false,0,0,0,0,       -1},
    {"sample: in sentence",      "Connection from 192.168.1.1 refused", true, 192,168,1,1, -1},
    {"sample: leading zero",     "192.168.01.1",                    false,0,0,0,0,       -1},
    {"sample: port too big",     "1.2.3.4:99999",                   false,0,0,0,0,       -1},
    {"sample: 3 octets",         "12.34.56",                        false,0,0,0,0,       -1},
    {"sample: no number",        "no number here",                  false,0,0,0,0,       -1},

    // ---- boundaries ----
    {"all zeros",                "0.0.0.0",                         true, 0,0,0,0,       -1},
    {"all 255",                  "255.255.255.255",                 true, 255,255,255,255,-1},
    {"high bit set (>2^31)",     "200.1.1.1",                       true, 200,1,1,1,     -1},
    {"octet 256",                "256.1.1.1",                       false,0,0,0,0,       -1},
    {"octet 256 last",           "1.1.1.256",                       false,0,0,0,0,       -1},
    {"octet 1000 (4 digits)",    "1.2.3.1000",                      false,0,0,0,0,       -1},
    {"octet 999",                "999.1.1.1",                       false,0,0,0,0,       -1},

    // ---- leading zeros ----
    {"leading zero first",       "01.2.3.4",                        false,0,0,0,0,       -1},
    {"leading zero last",        "1.2.3.04",                        false,0,0,0,0,       -1},
    {"000 octet",                "000.1.1.1",                       false,0,0,0,0,       -1},
    {"00 octet",                 "1.00.1.1",                        false,0,0,0,0,       -1},
    {"0004 octet",               "1.2.3.0004",                      false,0,0,0,0,       -1},

    // ---- ports ----
    {"port 0",                   "1.2.3.4:0",                       true, 1,2,3,4,       0},
    {"port 1",                   "1.2.3.4:1",                       true, 1,2,3,4,       1},
    {"port 65535",               "1.2.3.4:65535",                   true, 1,2,3,4,       65535},
    {"port 65536",               "1.2.3.4:65536",                   false,0,0,0,0,       -1},
    {"port 6 digits",            "1.2.3.4:123456",                  false,0,0,0,0,       -1},
    {"port 00",                  "1.2.3.4:00",                      false,0,0,0,0,       -1},
    {"port 080",                 "1.2.3.4:080",                     false,0,0,0,0,       -1},
    {"empty port",               "1.2.3.4:",                        false,0,0,0,0,       -1},
    {"empty port + text",        "1.2.3.4: open",                   false,0,0,0,0,       -1},
    {"double colon",             "1.2.3.4::80",                     false,0,0,0,0,       -1},
    {"second colon",             "1.2.3.4:80:90",                   false,0,0,0,0,       -1},
    {"port then period",         "1.2.3.4:80.",                     false,0,0,0,0,       -1},
    {"port with dot inside",     "1.2.3.4:8.0",                     false,0,0,0,0,       -1},
    {"colon not after 4th",      "1.2.3:4.5",                       false,0,0,0,0,       -1},
    {"colon after 1st octet",    "1:2.3.4.5",                       false,0,0,0,0,       -1},

    // ---- structure ----
    {"leading period",           ".1.2.3.4",                        false,0,0,0,0,       -1},
    {"leading colon",            ":1.2.3.4",                        false,0,0,0,0,       -1},
    {"empty octet",              "1..2.3",                          false,0,0,0,0,       -1},
    {"empty octet (4 dots)",     "1.2..4",                          false,0,0,0,0,       -1},
    {"five octets",              "1.2.3.4.5",                       false,0,0,0,0,       -1},
    {"two octets",               "1.2",                             false,0,0,0,0,       -1},
    {"only dots",                "....",                            false,0,0,0,0,       -1},
    {"only colons",              ":::",                             false,0,0,0,0,       -1},
    {"empty string",             "",                                false,0,0,0,0,       -1},
    {"only garbage",             "!@#$%^&*()",                      false,0,0,0,0,       -1},

    // ---- noise / separators ----
    {"letters glued on",         "a1.2.3.4b",                       true, 1,2,3,4,       -1},
    {"comma after",              "ip=1.2.3.4, ok",                  true, 1,2,3,4,       -1},
    {"brackets",                 "[10.20.30.40]:5060",              true, 10,20,30,40,   -1},
    {"tab/space padded",         "\t  8.8.8.8  \t",                 true, 8,8,8,8,       -1},
    {"letter splits octets",     "1.2.3a4.5",                       false,0,0,0,0,       -1},
    {"garbage then valid",       "1-2.3.4.5",                       true, 2,3,4,5,       -1},
    {"port after garbage",       "1.2.3.4:80x",                     true, 1,2,3,4,       80},

    // ---- multiple candidates: first valid token wins ----
    {"two valid",                "1.2.3.4 5.6.7.8",                 true, 1,2,3,4,       -1},
    {"invalid then valid",       "999.1.1.1 8.8.8.8",               true, 8,8,8,8,       -1},
    {"valid then invalid",       "8.8.8.8 999.1.1.1",               true, 8,8,8,8,       -1},
};

int main() {
    int failed = 0;
    const int total = static_cast<int>(sizeof(cases) / sizeof(cases[0]));

    for (const Case& t : cases) {
        unsigned long addr = 0xDEADBEEF;  // poison: must be reset by the function
        int port = 12345;
        bool ok = extractIPv4(t.input, addr, port);

        unsigned long want = 0;
        int wantPort = -1;
        if (t.ok) {
            want = (t.a << 24) | (t.b << 16) | (t.c << 8) | t.d;
            wantPort = t.port;
        }

        if (ok != t.ok || addr != want || port != wantPort) {
            ++failed;
            std::cout << "FAIL: " << t.name << "  input=\"" << t.input << "\"\n"
                      << "      got ok=" << ok << " addr=" << addr << " port=" << port
                      << "  | want ok=" << t.ok << " addr=" << want << " port=" << wantPort << "\n";
        }
    }

    std::cout << (total - failed) << "/" << total << " tests passed\n";
    return failed == 0 ? 0 : 1;
}
