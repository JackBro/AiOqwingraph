### AiO's QWinGraph32 for Qt
    The application has been modified to allow a better user experience.
### Key changes
1. Added a search feature to jump to a node's location
2. Added the ability to right click a node and copy the label to clipboard
3. Added a option to open a gdl file

### Known issues
GWinGraph32 will not run without a file name from the command line.
my solution is to script a launcher with a default gdl file loading large
files might take time Scaling is a problem on large graphs, ie 4K plus
nodes. the jump will not always be centered

### Possible future work
1. Add a list view of all nodes to modify parameters
2. incorporate a API to allow Python filtering of node parameters
3. Standalone python QT gdl editor to perform subgraph generation
