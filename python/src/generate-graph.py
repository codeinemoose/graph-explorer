#!/usr/bin/env python3
"""
Generate a sparse undirected simple graph with community structure and
introduce intra-community variation (hubs + weak vertices).

Outputs:
  - edge list (u v per line) by default
  - optionally a JSON describing nodes (with popularity) and edges

Usage examples:
  python generate_communities_with_hubs.py -n 600 -k 6 --p-in 0.05 --p-out 0.001 --pop-influence 0.8
  python generate_communities_with_hubs.py --dist pareto --pop-params "alpha=2.5" --out data/mygraph.txt --json-out data/mygraph.json
"""

import random
import argparse
import json
from math import isfinite

# -------------------- CONFIG (defaults you can tweak) --------------------
DEFAULT_NODES = 600
DEFAULT_COMMUNITIES = 6
DEFAULT_P_IN = 0.05       # baseline probability for intra-community edges
DEFAULT_P_OUT = 0.001     # baseline probability for inter-community edges
DEFAULT_SEED = 42
DEFAULT_OUT = "data/sparse_graph_with_communities.txt"
DEFAULT_JSON_OUT = ""     # set a filename to also dump nodes+edges as JSON
# Popularity (hub) parameters:
# distribution: 'exp' (exponential), 'pareto', 'normal', 'uniform'
DEFAULT_POP_DIST = "exp"
DEFAULT_POP_PARAMS = ""   # optional string for distribution-specific params; parsed as key=val comma list
DEFAULT_POP_INFLUENCE = 0.8   # 0.0 -> no popularity effect, 1.0 -> fully weighted by popularity
DEFAULT_INTER_POP = False     # whether to apply popularity effect to inter-community edges
# -------------------------------------------------------------------------

def parse_pop_params(param_str):
    """
    Parse string like "alpha=2.5,scale=1.0" into dict {'alpha':2.5, 'scale':1.0}
    """
    d = {}
    if not param_str:
        return d
    for part in param_str.split(","):
        if "=" in part:
            k, v = part.split("=", 1)
            k = k.strip()
            v = v.strip()
            try:
                if "." in v:
                    d[k] = float(v)
                else:
                    d[k] = int(v)
            except ValueError:
                d[k] = v
    return d

def sample_popularity(rng, dist, pop_params):
    """
    Return a single positive sample according to the chosen distribution.
    Supported dist: 'exp', 'pareto', 'normal', 'uniform'
    pop_params is a dict with distribution parameters.
    """
    if dist == "exp":
        # lambda (rate) parameter; default 1.0
        rate = float(pop_params.get("rate", 1.0))
        return rng.expovariate(rate)
    elif dist == "pareto":
        # alpha parameter (>0), default 2.0
        alpha = float(pop_params.get("alpha", 2.0))
        # Python's random.paretovariate returns Pareto with xm = 1
        return rng.paretovariate(alpha)
    elif dist == "normal":
        # mu, sigma: ensure positive by clipping small values
        mu = float(pop_params.get("mu", 1.0))
        sigma = float(pop_params.get("sigma", 0.5))
        val = rng.gauss(mu, sigma)
        return max(1e-6, val)
    elif dist == "uniform":
        a = float(pop_params.get("a", 0.0))
        b = float(pop_params.get("b", 1.0))
        val = rng.random() * (b - a) + a
        return max(1e-6, val)
    else:
        # fallback: exponential
        return rng.expovariate(1.0)

def generate_communities(n, k, p_in, p_out, seed=None,
                         pop_dist="exp", pop_params=None,
                         pop_influence=0.8, inter_pop=False, verbose=False):
    """
    Generate undirected edges with community structure and node popularity.
    - Each node gets a positive popularity score.
    - Popularity is normalized per-community to mean = 1 (so baseline probabilities stay around p_in).
    - Final intra-community edge probability between u and v:
         prob = p_in * [ (1 - pop_influence) + pop_influence * ((pop[u] + pop[v]) / 2) ]
      So pop_influence blends baseline p_in with popularity-weighted p_in.
    - If inter_pop is True, the same popularity formula is applied to p_out.
    Returns: edges (set of (u,v) tuples with u < v), communities list, popularity dict
    """
    rng = random.Random(seed)

    # Partition nodes into communities (balanced)
    sizes = [n // k] * k
    for i in range(n % k):
        sizes[i] += 1

    communities = []
    node = 1
    for size in sizes:
        comm = list(range(node, node + size))
        communities.append(comm)
        node += size

    # Sample raw popularity for every node
    pop_params = pop_params or {}
    raw_pop = {}
    for u in range(1, n + 1):
        raw_pop[u] = sample_popularity(rng, pop_dist, pop_params)

    # Normalize popularity per community to mean = 1
    popularity = {}
    for comm in communities:
        mean_raw = sum(raw_pop[u] for u in comm) / len(comm)
        if mean_raw <= 0 or not isfinite(mean_raw):
            mean_raw = 1.0
        for u in comm:
            popularity[u] = raw_pop[u] / mean_raw

    edges = set()

    # Intra-community edges
    for comm in communities:
        L = len(comm)
        # iterate pairs (i,j) i<j
        for i in range(L):
            u = comm[i]
            pop_u = popularity[u]
            for j in range(i + 1, L):
                v = comm[j]
                pop_v = popularity[v]
                # average popularity around 1, so baseline preserved in expectation
                pop_factor = ((pop_u + pop_v) / 2.0)
                prob = p_in * ((1.0 - pop_influence) + pop_influence * pop_factor)
                if rng.random() < prob:
                    a, b = (u, v) if u < v else (v, u)
                    edges.add((a, b))

    # Inter-community edges
    for i in range(len(communities)):
        for j in range(i + 1, len(communities)):
            for u in communities[i]:
                for v in communities[j]:
                    if inter_pop:
                        pop_factor = ((popularity[u] + popularity[v]) / 2.0)
                        prob = p_out * ((1.0 - pop_influence) + pop_influence * pop_factor)
                    else:
                        prob = p_out
                    if rng.random() < prob:
                        a, b = (u, v) if u < v else (v, u)
                        edges.add((a, b))

    if verbose:
        print(f"[generate] n={n}, k={k}, p_in={p_in}, p_out={p_out}, pop_dist={pop_dist}, pop_influence={pop_influence}, inter_pop={inter_pop}")
        # degree summary
        deg = {u: 0 for u in range(1, n + 1)}
        for (a, b) in edges:
            deg[a] += 1
            deg[b] += 1
        degs = sorted(deg.values())
        print(f"[generate] edges={len(edges)}, deg_min={degs[0]}, deg_med={degs[len(degs)//2]}, deg_max={degs[-1]}")
        # top 5 popularity
        top5 = sorted(popularity.items(), key=lambda kv: kv[1], reverse=True)[:5]
        print("[generate] top 5 popular nodes (id, pop):", top5)

    return edges, communities, popularity

def write_edge_list(edges, path):
    with open(path, "w") as f:
        for a, b in sorted(edges):
            f.write(f"{a} {b}\n")

def write_json(edges, popularity, communities, path):
    """
    Write a JSON file containing nodes with popularity and adjacency (neighbours list),
    and edges list. Useful for plotting tools that accept JSON.
    """
    nodes = []
    neighbours = {u: [] for u in popularity.keys()}
    for a, b in edges:
        neighbours[a].append(b)
        neighbours[b].append(a)
    for u, pop in popularity.items():
        nodes.append({
            "id": u,
            "popularity": pop,
            "neighbours": sorted(neighbours[u])
        })
    out = {
        "nodes": nodes,
        "edges": sorted([list(e) for e in edges]),
        "communities": [sorted(c) for c in communities]
    }
    with open(path, "w") as f:
        json.dump(out, f, indent=2)

def main():
    parser = argparse.ArgumentParser(description="Generate sparse undirected graph with community hubs.")
    parser.add_argument("--nodes", "-n", type=int, default=DEFAULT_NODES)
    parser.add_argument("--communities", "-k", type=int, default=DEFAULT_COMMUNITIES)
    parser.add_argument("--p-in", type=float, default=DEFAULT_P_IN)
    parser.add_argument("--p-out", type=float, default=DEFAULT_P_OUT)
    parser.add_argument("--seed", "-s", type=int, default=DEFAULT_SEED)
    parser.add_argument("--out", "-o", type=str, default=DEFAULT_OUT)
    parser.add_argument("--json-out", type=str, default=DEFAULT_JSON_OUT,
                        help="Optional JSON output file with nodes (popularity + neighbours) and edges")
    parser.add_argument("--dist", type=str, default=DEFAULT_POP_DIST,
                        choices=["exp", "pareto", "normal", "uniform"],
                        help="Popularity distribution")
    parser.add_argument("--pop-params", type=str, default=DEFAULT_POP_PARAMS,
                        help="Comma-separated distribution parameters, e.g. 'alpha=2.5,scale=1.0' ")
    parser.add_argument("--pop-influence", type=float, default=DEFAULT_POP_INFLUENCE,
                        help="Blend factor 0..1: how much popularity affects edge probability (default 0.8)")
    parser.add_argument("--inter-pop", action="store_true", default=DEFAULT_INTER_POP,
                        help="Apply popularity influence to inter-community edges too")
    parser.add_argument("--verbose", "-v", action="store_true")
    args = parser.parse_args()

    n = args.nodes
    k = max(1, min(n, args.communities))  # clamp between 1 and n

    pop_params = parse_pop_params(args.pop_params)

    edges, communities, popularity = generate_communities(
        n=n, k=k, p_in=args.p_in, p_out=args.p_out, seed=args.seed,
        pop_dist=args.dist, pop_params=pop_params,
        pop_influence=args.pop_influence, inter_pop=args.inter_pop, verbose=args.verbose
    )

    write_edge_list(edges, args.out)
    if args.verbose:
        print(f"Wrote edge list to: {args.out}")

    if args.json_out:
        write_json(edges, popularity, communities, args.json_out)
        if args.verbose:
            print(f"Wrote JSON graph to: {args.json_out}")

    print(f"Saved {len(edges)} edges for {n} nodes with {k} communities (seed={args.seed}).")

if __name__ == "__main__":
    main()
