# Pronounceable Password Generator

A C++ utility that generates human-readable, memorable passwords while avoiding dictionary words. Creates secure passwords using pronounceable syllables with configurable complexity options.

## Features

- Generates pronounceable password bases
- Configurable password length
- Optional 2-digit numbers (`-digits`)
- Optional special characters (`-special`)
- Customizable special character set (`-chars`)
- Avoids dictionary words through random syllable patterns
- Inserts non-alphabet characters at random positions

## Build Instructions

### Prerequisites
- C++20 compatible compiler
- CMake (3.10+)
- Make

### Building
```bash
mkdir build && cd build
cmake -DCMAKE_CXX_STANDARD=20 ..
make
```

### Running
```bash
./passgen -length 12 -digits -special
```

## Usage

### Basic Usage
Generate a 10-character password:
```bash
./passgen -length 10
```
Example output: `zexovakipo`

### With Numbers and Special Characters
```bash
./passgen -length 12 -digits -special
```
Example output: `vupa41@jelowi`

### Custom Special Characters
```bash
./passgen -length 14 -special -chars "£€$"
```
Example output: `jaxu£89pokiter`

### Options
| Flag         | Description                          | Requires Value |
|--------------|--------------------------------------|----------------|
| `-length`    | Total password length (required)     | Yes            |
| `-digits`    | Include 2-digit number               | No             |
| `-special`   | Include special character            | No             |
| `-chars`     | Custom special characters            | Yes            |

## Password Generation Strategy

1. Creates pronounceable base using alternating consonants/vowels
2. Adds complexity elements (numbers/specials) at random positions
3. Uses secure randomization through:
   - Mersenne Twister engine (mt19937)
   - System entropy source (random_device)
   - Cryptographic-grade PRNG for shuffling

## License
MIT License - see [LICENSE](LICENSE)