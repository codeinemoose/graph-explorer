import sys
import numpy as np


def main(argv: list[str]) -> int:
    directory = argv[0]

    with open(directory, "rb") as file:
        type_block      = int(np.fromfile(file, dtype=np.uint8, count=1)[0])
        version_block   = int(np.fromfile(file, dtype=np.uint8, count=1)[0])
        node_count      = int(np.fromfile(file, dtype=np.uint64, count=1)[0])
        edge_count      = int(np.fromfile(file, dtype=np.uint64, count=1)[0])

        offsets = np.fromfile(file, dtype=np.int32, count=node_count + 1)
        targets = np.fromfile(file, dtype=np.int32, count=edge_count)
    
    print("type:", type_block)
    print("version:", version_block)
    print("nodes:", node_count)
    print("edges:", edge_count)
    print("offsets:", offsets)
    print("targets:", targets)

    print("LOADING GRAPH")

    return 1

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
    