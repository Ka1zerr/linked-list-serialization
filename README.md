# Doubly Linked List with Random Pointer — Serialization

## Build

```bash
g++ -std=c++17 -O2 -o list_serial main.cpp
```

## Run

```bash
# Uses inlet.in → outlet.out by default
./list_serial

# Or specify custom files
./list_serial my_input.in my_output.out
```

## Input format (`inlet.in`)

Each line describes one node:

```
<data>;<rand_index>
```

- `<data>` — any UTF-8 string (may contain spaces, special chars)
- `<rand_index>` — 0-based index of the node `rand` points to, or `-1` for `nullptr`

Example:
```
apple;2
banana;-1
carrot;1
```

## Binary format (`outlet.out`)

```
[uint32 node_count]
for each node:
  [uint32 data_length][data bytes][int32 rand_index]
```

All integers are native-endian (little-endian on x86/x64).  
`rand_index == -1` means `rand == nullptr`.

## Constraints

- Up to 10⁶ nodes
- `data` up to 1 000 characters (UTF-8)
