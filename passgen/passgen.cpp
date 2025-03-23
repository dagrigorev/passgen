// passgen.cpp : Defines the entry point for the application.
//

#include "passgen.h"

#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <stdexcept>
#include <random>

using namespace std;

const string CONSONANTS = "bcdfghjklmnpqrstvwxyz";
const string VOWELS = "aeiou";
const string DEFAULT_SPECIAL = "!@#$%^&*";

struct pass_gen_config {
    int length = 0;
    bool use_numbers = false;
    bool use_special = false;
    string special_chars = DEFAULT_SPECIAL;
};

char get_random_char(const string& charset) {
    return charset[rand() % charset.size()];
}

string generate_pronounceable_base(int length) {
    string base;
    bool start_with_consonant = rand() % 2;

    for (int i = 0; i < length; ++i) {
        bool use_consonant = (i % 2 == 0) ? start_with_consonant : !start_with_consonant;
        base += get_random_char(use_consonant ? CONSONANTS : VOWELS);
    }
    return base;
}

pass_gen_config parse_arguments(int argc, char* argv[]) {
    pass_gen_config config;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "-length") {
            if (++i >= argc) throw runtime_error("Missing value for -length");
            config.length = stoi(argv[i]);
        }
        else if (arg == "-digits") {
            config.use_numbers = true;
        }
        else if (arg == "-special") {
            config.use_special = true;
        }
        else if (arg == "-chars") {
            if (++i >= argc) throw runtime_error("Missing value for -chars");
            config.special_chars = argv[i];
        }
        else {
            throw runtime_error("Unknown argument: " + arg);
        }
    }

    if (config.length <= 0)
        throw runtime_error("Password length (-length) must be specified and positive");

    if (config.use_special && config.special_chars.empty())
        config.special_chars = DEFAULT_SPECIAL;

    return config;
}

string generate_password(const pass_gen_config& config) {
    int num_numbers = config.use_numbers ? 2 : 0;
    int num_specials = config.use_special ? 1 : 0;
    int base_length = config.length - num_numbers - num_specials;

    if (base_length < 4)
        throw runtime_error("Base password length too short. Increase total length.");

    string base = generate_pronounceable_base(base_length);
    string numbers = config.use_numbers ? to_string(rand() % 90 + 10) : "";
    string special = config.use_special ? string(1, get_random_char(config.special_chars)) : "";

    vector<string> insertions;
    if (config.use_numbers) insertions.push_back(numbers);
    if (config.use_special) insertions.push_back(special);

    //random_shuffle(insertions.begin(), insertions.end());
    random_device rd;
    mt19937 g(rd());
    shuffle(insertions.begin(), insertions.end(), g);

    string password = base;
    for (const auto& insertion : insertions) {
        int pos = rand() % (password.length() + 1);
        password.insert(pos, insertion);
    }

    return password;
}

int main(int argc, char* argv[]) {
    try {
        srand(time(nullptr));
        pass_gen_config config = parse_arguments(argc, argv);

        cout << generate_password(config) << endl;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n\n";
        cerr << "Usage: " << argv[0] << " -length LENGTH\n"
            << "Options:\n"
            << "  -digits          Include numbers (adds 2 digits)\n"
            << "  -special         Include special characters\n"
            << "  -chars CHARS    Custom special characters (default: " << DEFAULT_SPECIAL << ")\n";
        return 1;
    }

    return 0;
}
