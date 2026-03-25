// passgen.cpp : Defines the entry point for the application.
//

#include "passgen.h"

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <cmath>
#include <iomanip>

using namespace std;

static const string VERSION       = "1.1.0";
static const string CONSONANTS    = "bcdfghjklmnpqrstvwxyz";
static const string VOWELS        = "aeiou";
static const string UPPERCASE     = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const string DIGITS        = "0123456789";
static const string DEFAULT_SPECIAL = "!@#$%^&*";
// Characters that are visually ambiguous across fonts
static const string AMBIGUOUS     = "0Ol1I";

// Single seeded RNG used everywhere — cryptographically seeded, no rand()/srand()
static mt19937 rng(random_device{}());

static int random_int(int n) {
    return uniform_int_distribution<int>(0, n - 1)(rng);
}

static char get_random_char(const string& charset) {
    return charset[random_int((int)charset.size())];
}

static string remove_ambiguous(const string& s) {
    string result;
    for (char c : s)
        if (AMBIGUOUS.find(c) == string::npos)
            result += c;
    return result;
}

struct pass_gen_config {
    int    length        = 0;
    int    count         = 1;
    bool   use_numbers   = false;
    bool   use_special   = false;
    bool   use_upper     = false;
    bool   no_ambiguous  = false;
    bool   show_strength = false;
    string special_chars = DEFAULT_SPECIAL;
};

static void print_usage(const char* prog) {
    cerr << "Usage: " << prog << " -length N [options]\n\n"
         << "Options:\n"
         << "  -length N        Password length (required)\n"
         << "  -count N         Number of passwords to generate (default: 1)\n"
         << "  -digits          Include 2 digits\n"
         << "  -upper           Include an uppercase letter\n"
         << "  -special         Include a special character\n"
         << "  -chars CHARS     Custom special character set (default: " << DEFAULT_SPECIAL << ")\n"
         << "  -no-ambiguous    Exclude visually ambiguous chars: " << AMBIGUOUS << "\n"
         << "  -strength        Show entropy estimate next to each password\n"
         << "  -version         Show version and exit\n"
         << "  -help            Show this help and exit\n";
}

static pass_gen_config parse_arguments(int argc, char* argv[]) {
    pass_gen_config config;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "-length") {
            if (++i >= argc) throw runtime_error("Missing value for -length");
            config.length = stoi(argv[i]);
        } else if (arg == "-count") {
            if (++i >= argc) throw runtime_error("Missing value for -count");
            config.count = stoi(argv[i]);
            if (config.count <= 0) throw runtime_error("-count must be a positive integer");
        } else if (arg == "-digits") {
            config.use_numbers = true;
        } else if (arg == "-upper") {
            config.use_upper = true;
        } else if (arg == "-special") {
            config.use_special = true;
        } else if (arg == "-chars") {
            if (++i >= argc) throw runtime_error("Missing value for -chars");
            config.special_chars = argv[i];
        } else if (arg == "-no-ambiguous") {
            config.no_ambiguous = true;
        } else if (arg == "-strength") {
            config.show_strength = true;
        } else {
            throw runtime_error("Unknown argument: " + arg);
        }
    }

    if (config.length <= 0)
        throw runtime_error("Password length (-length) must be specified and positive");

    if (config.use_special && config.special_chars.empty())
        config.special_chars = DEFAULT_SPECIAL;

    return config;
}

// Approximate entropy: length * log2(effective alphabet size)
static double estimate_entropy(const pass_gen_config& config) {
    int alphabet = 26; // lowercase consonants + vowels ≈ full lowercase
    if (config.use_upper)   alphabet += 26;
    if (config.use_numbers) alphabet += 10;
    if (config.use_special) alphabet += (int)config.special_chars.size();
    if (config.no_ambiguous) alphabet -= (int)AMBIGUOUS.size();
    if (alphabet < 2) alphabet = 2;
    return config.length * log2((double)alphabet);
}

static string strength_label(double entropy) {
    if (entropy < 40) return "Weak";
    if (entropy < 60) return "Fair";
    if (entropy < 80) return "Good";
    return "Strong";
}

static string generate_pronounceable_base(int length, bool no_ambiguous) {
    string consonants = no_ambiguous ? remove_ambiguous(CONSONANTS) : CONSONANTS;
    string vowels     = no_ambiguous ? remove_ambiguous(VOWELS)     : VOWELS;

    if (consonants.empty() || vowels.empty())
        throw runtime_error("No characters remain after removing ambiguous chars");

    string base;
    bool start_consonant = random_int(2) == 0;

    for (int i = 0; i < length; ++i) {
        bool use_consonant = (i % 2 == 0) ? start_consonant : !start_consonant;
        base += get_random_char(use_consonant ? consonants : vowels);
    }
    return base;
}

static string generate_password(const pass_gen_config& config) {
    // Reserve slots for injected character groups
    int reserved = (config.use_numbers ? 2 : 0)
                 + (config.use_special ? 1 : 0)
                 + (config.use_upper   ? 1 : 0);
    int base_length = config.length - reserved;

    if (base_length < 4)
        throw runtime_error("Base password length too short after reserving slots. Increase -length.");

    string base = generate_pronounceable_base(base_length, config.no_ambiguous);

    // Build insertion strings
    vector<string> insertions;

    if (config.use_numbers) {
        string dset = config.no_ambiguous ? remove_ambiguous(DIGITS) : DIGITS;
        if ((int)dset.size() < 2)
            throw runtime_error("Not enough digits available after removing ambiguous characters");
        string nums;
        nums += get_random_char(dset);
        nums += get_random_char(dset);
        insertions.push_back(nums);
    }

    if (config.use_special) {
        string sset = config.no_ambiguous ? remove_ambiguous(config.special_chars) : config.special_chars;
        if (sset.empty())
            throw runtime_error("No special characters remain after removing ambiguous ones");
        insertions.push_back(string(1, get_random_char(sset)));
    }

    if (config.use_upper) {
        string uset = config.no_ambiguous ? remove_ambiguous(UPPERCASE) : UPPERCASE;
        if (uset.empty())
            throw runtime_error("No uppercase characters remain after removing ambiguous ones");
        insertions.push_back(string(1, get_random_char(uset)));
    }

    // Randomise insertion order then splice into base at random positions
    shuffle(insertions.begin(), insertions.end(), rng);

    string password = base;
    for (const auto& part : insertions) {
        int pos = random_int((int)password.size() + 1);
        password.insert(pos, part);
    }

    return password;
}

int main(int argc, char* argv[]) {
    // Handle meta-flags before full parse so they always work
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-help" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        if (arg == "-version" || arg == "--version") {
            cout << "passgen " << VERSION << "\n";
            return 0;
        }
    }

    try {
        pass_gen_config config = parse_arguments(argc, argv);

        for (int i = 0; i < config.count; ++i) {
            string password = generate_password(config);
            if (config.show_strength) {
                double entropy = estimate_entropy(config);
                cout << password
                     << "  [" << fixed << setprecision(1) << entropy
                     << " bits, " << strength_label(entropy) << "]\n";
            } else {
                cout << password << "\n";
            }
        }
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n\n";
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
