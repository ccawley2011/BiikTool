# File format for Biik archive files

All values are in little-endian for RISC OS and Windows data files, and in big-endian for Macintosh files. Strings are null terminated, then the file size is padded to a multiple of 4 bytes.

| Offset | Description                                                                  |
| ------ | ---------------------------------------------------------------------------- |
| 0x0    | `BIIK-DJC`                                                                   |
| 0x8    | Header size.                                                                 |
| 0xC    | Archive version?                                                             |
| 0x10   | Game ID. Must match ID provided by the interpreter, otherwise it will crash. |
| 0x14   | Offset to file index.                                                        |

The Game ID can be one of the following values:
 - **Dinosaur Discovery:** *0x01*
 - **A Mouse in Holland:** *0x03*
 - **Explore with Flossy the Frog:** *0x05*
 - **Betsi the Tudor Dog:** *0x08*
 - **Guardians of the Greenwood:** *0x0D*
 - **Find It Fix It:** *0x12*
 - **Darryl the Dragon:** *0xFF*

The file index is usually positioned at the end of the file. It consists of a header, followed by one or more entries. The header has the following format:

| Offset | Description                           |
| ------ | ------------------------------------- |
| 0x0    | Always 3?                             |
| 0x4    | Size of file index                    |
| 0x8    | Size of index header                  |
| 0xC    | Title of index (usually `File index`) |

Each entry has the following format:

| Offset | Description           |
| ------ | --------------------- |
| 0x0    | Size of entry         |
| 0x4    | Offset to file header |
| 0x8    | File name             |

The file itself is prefixed by a second header, which contains more infomation about the file. The format of the header is as follows:

| Offset | Description                               |
| ------ | ----------------------------------------- |
| 0x0    | Type of file.                             |
| 0x4    | Combined size of the file and the header. |
| 0x8    | Size of the header.                       |
| 0xC    | File name.                                |

The file type can be one of the following values:
- 1 is an image.
- 2 is an ArcTracker module.
- 9 is a script file.
- 11 is a Draw file.
- 15 is a co-ordinates file.

The embedded file follows immediately after this header.
