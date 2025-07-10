#ifndef PATH_PLANNING_H
#define PATH_PLANNING_H

#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include "../data_fusion/data_fusion.h"

// Define map dimensions (example values)
// 定义地图尺寸（示例值）
#define MAP_WIDTH 20
#define MAP_HEIGHT 20

// Node structure for pathfinding algorithms
// 路径查找算法的节点结构
typedef struct {
    int node_id; // Add node_id to MapNode_t
    int x, y; // Coordinates
    int area_id; // Area ID for the node
    int g_cost; // Cost from start to current node
    int h_cost; // Heuristic cost from current node to end
    int f_cost; // Total cost (g_cost + h_cost)
    int parent_x, parent_y; // Parent node coordinates
    bool is_obstacle; // Is this node an obstacle?
    bool is_safe;     // Is this node safe (based on environmental data)?
} MapNode_t;

// Structure to represent a path
// 表示路径的结构体
#define MAX_PATH_LENGTH (MAP_WIDTH * MAP_HEIGHT) // Max possible path length
typedef struct {
    MapNode_t nodes[MAX_PATH_LENGTH];
    int num_nodes;
    float total_distance;
    float total_risk;
    time_t timestamp;
} path_t;

// Structure to represent a graph node
// 表示图节点的结构体
#define MAX_NODES 100 // Max number of nodes in the graph
typedef struct {
    int node_id;
    int area_id;
    float x, y;
} graph_node_t;

// Structure to represent a graph edge
// 表示图边的结构体
#define MAX_EDGES 200 // Max number of edges in the graph
typedef struct {
    int edge_id;
    int start_node;
    int end_node;
    float distance;
    float risk_factor; // Risk factor associated with this edge
} graph_edge_t;

// Structure to represent the entire graph
// 表示整个图的结构体
typedef struct {
    graph_node_t nodes[MAX_NODES];
    int num_nodes;
    graph_edge_t edges[MAX_EDGES];
    int num_edges;
} graph_t;

// Function prototypes
// 函数原型
/**
 * @brief Initializes the path planning module and map.
 * @return 0 if successful, -1 otherwise.
 */
int path_planning_init(void);

/**
 * @brief Loads map data from a file.
 * @param filename Path to the map data file.
 * @return 0 if successful, -1 otherwise.
 */
int load_map_data(const char *filename);

/**
 * @brief Adds a node to the graph.
 * @param node_id ID of the node.
 * @param area_id Area ID associated with the node.
 * @param x X coordinate of the node.
 * @param y Y coordinate of the node.
 * @return 0 if successful, -1 otherwise.
 */
int add_node(int node_id, int area_id, float x, float y);

/**
 * @brief Adds an edge to the graph.
 * @param edge_id ID of the edge.
 * @param start_node ID of the start node.
 * @param end_node ID of the end node.
 * @param distance Distance of the edge.
 * @return 0 if successful, -1 otherwise.
 */
int add_edge(int edge_id, int start_node, int end_node, float distance);

/**
 * @brief Updates the risk factors of edges based on environmental data.
 * @param env_data Current environmental data.
 * @return 0 if successful, -1 otherwise.
 */
int update_edge_risks(const EnvironmentalData_t *env_data);

/**
 * @brief Finds the shortest and safest path from start to end using A* algorithm.
 * @param start_area_id Start area ID.
 * @param end_area_id End area ID.
 * @param path Pointer to path_t structure to store the found path.
 * @return 0 if successful, -1 otherwise.
 */
int find_safe_path(int start_area_id, int end_area_id, path_t *path);

/**
 * @brief Cleans up the path planning module.
 */
void path_planning_cleanup(void);

#endif // PATH_PLANNING_H




/**
 * @brief Gets the direction from the path.
 * @param path Pointer to the path_t structure.
 * @return Direction enum value, or -1 if invalid path.
 */
int get_direction_from_path(const path_t *path);


