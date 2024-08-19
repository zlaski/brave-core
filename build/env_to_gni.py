import sys

# Check if a file name is provided as a command line argument
if len(sys.argv) < 2:
    print("Usage: python script.py <filename>")
    sys.exit(1)

filename = sys.argv[1]

lowercase_map = {}

try:
    with open(filename, 'r') as file:
        for line in file:
            # Split each line by '=' and strip to remove leading/trailing whitespaces
            key, value = map(str.strip, line.split('=', 1))
            print(f'{key}="{value}"')
            print(f'{key.upper()}="{value}"')
except FileNotFoundError:
    open(filename, 'w')

print('extra_env_deps=["//../../.env"]')

print('called script', file=sys.stderr)
