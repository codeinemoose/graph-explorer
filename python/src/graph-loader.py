import json
import matplotlib.pyplot as plt
import networkx as nx
import matplotlib.patches as mpatches
import numpy as np
from matplotlib.colors import LinearSegmentedColormap

# Load your JSON
with open("data/sparse_graph_with_communities1-fdl.json", "r") as f:
    data = json.load(f)

G = nx.Graph()

# Add nodes with positions and the label attribute from JSON
pos = {}
ranks = {}
for node in data["nodes"]:
    node_id = node["id"]
    node_label = node.get("label", "")   # use JSON label (fallback to "")
    G.add_node(node_id, label=node_label)
    pos[node_id] = (node["x"], node["y"])
    ranks[node_id] = node["neighbours"]

# Change the size of each node according to the number of neighbours
sizes = {n: ranks[n] for n in ranks}
max_size = max(sizes.values())
sizes = {n: (sizes[n] + 1) / max_size for n in sizes}
node_sizes = [sizes[n] * 100 for n in G.nodes()]

# Add edges
for edge in data["edges"]:
    G.add_edge(edge["source"], edge["target"])

# Extract labels from graph and build a stable unique list
labels = nx.get_node_attributes(G, "label")
unique_labels = sorted(set(labels.values()), key=lambda x: str(x))  # deterministic order

# Create custom colormap: dark red → orange → light blue → dark blue
colors = ["#8b0000", "#ff7f00", "#87cefa", "#00008b"]  # darkred, orange, lightblue, darkblue
custom_cmap = LinearSegmentedColormap.from_list("custom", colors, N=len(unique_labels))

# Sample distinct colors for however many labels exist
color_indices = np.linspace(0, 1, len(unique_labels))
label_colors = [custom_cmap(ci) for ci in color_indices]

# Map labels to colors
label_to_color = {lbl: label_colors[i] for i, lbl in enumerate(unique_labels)}
node_colors = [label_to_color[labels[n]] for n in G.nodes()]

# Draw
plt.figure(figsize=(10, 8))
nx.draw(
    G,
    pos,
    with_labels=False,
    labels={n: G.nodes[n]["label"] for n in G.nodes()},
    node_color=node_colors,
    node_size=node_sizes,
    font_color="white",
    edge_color="#dddddd",  # very light grey edges
    linewidths=0.1
)

# Legend: color patches for each label
legend_patches = []
for lbl in unique_labels:
    legend_patches.append(mpatches.Patch(color=label_to_color[lbl], label=str(lbl)))

plt.legend(handles=legend_patches, title="label", bbox_to_anchor=(1.05, 1), loc="upper left")
plt.title("Graph Colored by Label")
plt.tight_layout()
plt.show()
