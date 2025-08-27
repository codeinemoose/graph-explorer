#!/usr/bin/env python3
"""
Generate a sparse undirected simple graph with community structure
and write edge list to a text file.

Communities (2–5 of them) are formed such that nodes inside the same
community are more densely connected than across communities.

Default values:
  DEFAULT_NODES = 200
  DEFAULT_COMMUNITIES = 3
  DEFAULT_P_IN = 0.08    # intra-community edge prob
  DEFAULT_P_OUT = 0.005  # inter-community edge prob
"""

import random
import argparse

# ----- Edit these defaults directly in the script -----
DEFAULT_NODES = 600
DEFAULT_COMMUNITIES = 6
DEFAULT_P_IN = 0.15      # probability of edge inside a community
DEFAULT_P_OUT = 0.002    # probability of edge between communities
DEFAULT_SEED = 42
DEFAULT_OUT = "data/sparse_graph_with_communities.txt"
# -----------------------------------------------------

def generate_communities(n, k, p_in, p_out, seed=None, verbose=False):
    """
    Generate undirected edges for a graph with n nodes partitioned into k communities.
    Inside-community edges occur with probability p_in,
    between-community edges occur with probability p_out.
    Returns a set of (u,v) with 1 <= u < v <= n.
    """
    rng = random.Random(seed)

    # Partition nodes into communities
    sizes = [n // k] * k
    for i in range(n % k):
        sizes[i] += 1

    communities = []
    node = 1
    for size in sizes:
        comm = list(range(node, node + size))
        communities.append(comm)
        node += size

    edges = set()

    # Intra-community edges
    for comm in communities:
        for i in range(len(comm)):
            for j in range(i + 1, len(comm)):
                if rng.random() < p_in:
                    edges.add((comm[i], comm[j]))

    # Inter-community edges
    for i in range(len(communities)):
        for j in range(i + 1, len(communities)):
            for u in communities[i]:
                for v in communities[j]:
                    if rng.random() < p_out:
                        edges.add((u, v))

    return edges

def write_edge_list(edges, path):
    """
    Write edges to file in the "u v" format, sorted by (u,v).
    """
    with open(path, "w") as f:
        for a, b in sorted(edges):
            f.write(f"{a} {b}\n")

def main():
    parser = argparse.ArgumentParser(description="Generate sparse undirected graph with communities.")
    parser.add_argument("--nodes", "-n", type=int, default=DEFAULT_NODES,
                        help=f"Number of nodes (default: {DEFAULT_NODES})")
    parser.add_argument("--communities", "-k", type=int, default=DEFAULT_COMMUNITIES,
                        help=f"Number of communities (default: {DEFAULT_COMMUNITIES}, range 2–5)")
    parser.add_argument("--p-in", type=float, default=DEFAULT_P_IN,
                        help=f"Edge probability inside communities (default: {DEFAULT_P_IN})")
    parser.add_argument("--p-out", type=float, default=DEFAULT_P_OUT,
                        help=f"Edge probability between communities (default: {DEFAULT_P_OUT})")
    parser.add_argument("--seed", "-s", type=int, default=DEFAULT_SEED,
                        help=f"Random seed (default: {DEFAULT_SEED})")
    parser.add_argument("--out", "-o", type=str, default=DEFAULT_OUT,
                        help=f"Output filename (default: {DEFAULT_OUT})")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose progress output")
    args = parser.parse_args()

    n = args.nodes
    k = max(2, min(5, args.communities))  # clamp communities between 2 and 5

    if args.verbose:
        print(f"Generating graph with communities: nodes={n}, communities={k}, p_in={args.p_in}, p_out={args.p_out}, seed={args.seed}")

    edges = generate_communities(n, k, args.p_in, args.p_out, seed=args.seed, verbose=args.verbose)
    write_edge_list(edges, args.out)

    print(f"Saved {len(edges)} edges for {n} nodes with {k} communities to: {args.out}")

if __name__ == "__main__":
    main()
