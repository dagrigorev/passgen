# Pronounceable Password Generator

A C++ utility that generates human-readable, memorable passwords while avoiding dictionary words. Creates secure passwords using pronounceable syllables with configurable complexity options.

[![CMake on multiple platforms](https://github.com/dagrigorev/passgen/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/dagrigorev/passgen/actions/workflows/cmake-multi-platform.yml)

## Features

- Generates pronounceable password bases (alternating consonants/vowels)
- Configurable password length
- Batch generation — produce N passwords in one call (`-count`)
- Optional 2-digit number injection (`-digits`)
- Optional uppercase letter injection (`-upper`)
- Optional special character injection (`-special`)
- Customizable special character set (`-chars`)
- Exclude visually ambiguous characters (`-no-ambiguous`)
- Entropy estimate and strength label (`-strength`)
- Cryptographically seeded PRNG throughout (no `rand()`/`srand()`)

## Build Instructions

### Prerequisites
- C++20 compatible compiler (GCC, Clang, MSVC)
- CMake 3.8+

### Building
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Or using presets (recommended):
```bash
cmake --preset linux-release   # Linux / macOS
cmake --preset windows-release # Windows
cmake --build --preset linux-release
```

## Usage

### Basic — 10-character pronounceable password
```bash
./passgen -length 10
```
Example output: `zexovakipo`

### With numbers and special characters
```bash
./passgen -length 12 -digits -special
```
Example output: `vupa41@jelowi`

### With uppercase
```bash
./passgen -length 12 -digits -upper -special
```
Example output: `vuPa41@jelowi`

### Batch generation
```bash
./passgen -length 14 -digits -special -count 5
```
Example output:
```
zupakire93!bevo
nomifatu!47reba
...
```

### Show entropy estimate
```bash
./passgen -length 14 -digits -upper -special -strength
```
Example output: `zupA93!kirebavo  [82.4 bits, Strong]`

### Exclude ambiguous characters
```bash
./passgen -length 12 -digits -no-ambiguous
```
Removes `0`, `O`, `l`, `1`, `I` from all character sets.

### Custom special characters
```bash
./passgen -length 14 -special -chars "£€$"
```
Example output: `jaxu£pokiter`

### Help and version
```bash
./passgen -help
./passgen -version
```

## Options

| Flag            | Value | Description                                              |
|-----------------|-------|----------------------------------------------------------|
| `-length N`     | Yes   | Total password length (required)                         |
| `-count N`      | Yes   | Number of passwords to generate (default: 1)             |
| `-digits`       | No    | Inject 2 random digits                                   |
| `-upper`        | No    | Inject 1 uppercase letter                                |
| `-special`      | No    | Inject 1 special character                               |
| `-chars CHARS`  | Yes   | Custom special character set (default: `!@#$%^&*`)       |
| `-no-ambiguous` | No    | Exclude visually ambiguous chars: `0 O l 1 I`            |
| `-strength`     | No    | Print entropy estimate and strength label                |
| `-version`      | No    | Print version and exit                                   |
| `-help`         | No    | Print this help and exit                                 |

## Password Generation Strategy

1. Computes base length: total length minus slots reserved for injected groups
2. Builds a pronounceable base using alternating consonants/vowels
3. Generates each injected group (digits, uppercase, special) independently
4. Shuffles insertion order, then splices each group at a random position
5. All randomness from a single `mt19937` seeded by `std::random_device` — no `rand()`

### Entropy estimation

`entropy = length × log₂(alphabet_size)`

where alphabet size accumulates: 26 lowercase + 26 uppercase (if `-upper`) + 10 digits (if `-digits`) + special set size (if `-special`), minus ambiguous chars (if `-no-ambiguous`).

| Bits    | Label  |
|---------|--------|
| < 40    | Weak   |
| 40–59   | Fair   |
| 60–79   | Good   |
| ≥ 80    | Strong |

## License
MIT License - see [LICENSE](LICENSE)
