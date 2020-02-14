# HomuOS File System (unfinalized)

## Boot sector

| Address | Length | Contents                       |
| ------- | ------ | ------------------------------ |
| `0x1F0` |      8 | Address of superblock          |
| `0x1F8` |      2 | Reserved - set to 0            |
| `0x1FA` |      4 | HomuFS signature - `"Homu"`    |
| `0x1FE` |      2 | BIOS boot signature - `0xAA55` |

## Superblock

| Offset  | Length | Contents                     |
| ------- | ------ | ---------------------------- |
| `0x000` |      8 | Number of blocks             |
| `0x008` |   3944 | Reserved - set to 0          |
| `0xF70` |    144 | File entry of root directory |

## Block map

This area immediately follows the superblock. Its length is determined by the number of data blocks.
Each bit represents whether the corresponding block is allocated. Lower bits describe earlier blocks.
It takes up an integer number of blocks.

## File entry

A file entry contains the metadata of a file.

| Offset | Length    | Contents             |
| ------ | --------- | -------------------- |
| `0x00` |         1 | File type            |
| `0x01` |         7 | Reserved             |
| `0x08` |         8 | Size                 |
| `0x10` | 32 (16×8) | 16 block pointers    |

The level of indirection is inferred from the file size and describes how many
layers of block pointers there are and is determined by the file size.
A value of 0 indicates that the block pointers point directly to the data blocks.
Otherwise, they point to blocks containing block pointers with level of indirection lower by one.

# File type

| Value  | Meaning                      |
| ------ | ---------------------------- |
| `0x00` | Regular file                 |
| `0x01` | Directory                    |

## Directory

A directory is a file whose data blocks contain the file entries and names of files contained within.
The first eight bytes of a directory indicate the offset at which the first unallocated space is located.
This is followed by a list of file entries, each followed by a filename.
A filename consists of one byte representing its length and the bytes making it up.
Each file entry in aligned to eight bytes.
Each file entry must be contained within one block.
