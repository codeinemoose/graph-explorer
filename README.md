# Graph-Explorer
A lightweight network visualizer built with a C++ backend and (temporarily) with a Python frontend.

## Features
- Intuitive graph input.
- Community finder and highlighter.
- FDL for a sensible visualizer.
- JSON and binary output.

## Getting Started
Start by cloning the repo
````
git clone https://github.com/codeinemoose/graph-explorer.git
cd graph-explorer
````
Then compile the C++ files
````
.\scripts\compile.bat
````
And start the script
````
.\program.exe process data\data_set.txt
````
You will now have two JSON files, namely `data_set0-fdl.json` (before the application of FDL) and `data_set1-fdl.json` (after the application of FDL) 
as well as two binary files `data_set-communities-0.bin` (the binary with the community labels) and `data_set-graph.bin`(the binary without the community labels).

Your graph can now be visualised by your JSON parser of your choice or with the built-in Python one:
````
python .\python\src\graph-loader.py .\data\my_json.json
````

## Input/Output formats
The `.txt` of your original data set should have the following format:
````
node [space] node
````
which creates an (undirected) edge between the two nodes.

The `.JSON` will have the format:
````
{
  "nodes": [
    {"id": "A", "label": "community1", "x": 0.1, "y": 0.2, "neighbours": 3},
    ...
  ],
  "edges": [
    {"source": "A", "target": "B"},
    ...
  ]
}
````
