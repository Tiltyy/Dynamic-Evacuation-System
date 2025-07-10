/**
 * @file path_planning.c
 * @brief Path Planning Module for Dynamic Emergency Evacuation System
 * @version 1.0
 * @date 2025-07-04
 *
 * This module is responsible for calculating optimal evacuation paths
 * based on environmental data and map information.
 *
 * 中文注释：
 * 本模块负责根据环境数据和地图信息计算最佳疏散路径。
 */

 #include "path_planning.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <float.h>
 #include <time.h>
 #include <errno.h> // Include for errno and strerror
 
 // 全局图结构
 static graph_t graph;
 
 // 初始化路径规划模块
 int path_planning_init(void) {
     // 初始化图结构
     memset(&graph, 0, sizeof(graph));
     
     printf("Path planning module initialized.\n");
     return 0;
 }
 
 // 从文件加载地图数据
 int load_map_data(const char *filename) {
     FILE *file;
     char line[256];
     int node_id, area_id, edge_id, start_node, end_node;
     float x, y, distance;
     
     // 打开地图数据文件
     file = fopen(filename, "r");
     if (file == NULL) {
         fprintf(stderr, "Error: Cannot open map data file %s - %s\n", filename, strerror(errno));
         return -1;
     }
     
     // 读取节点数据
     if (fgets(line, sizeof(line), file) == NULL || strncmp(line, "NODES", 5) != 0) {
         fprintf(stderr, "Error: Invalid map data format (missing NODES header).\n");
         fclose(file);
         return -1;
     }
     
     while (fgets(line, sizeof(line), file) != NULL) {
         if (strncmp(line, "EDGES", 5) == 0) {
             break;
         }
         
         if (sscanf(line, "%d %d %f %f", &node_id, &area_id, &x, &y) == 4) {
             if (add_node(node_id, area_id, x, y) != 0) {
                 fprintf(stderr, "Error: Failed to add node %d.\n", node_id);
                 fclose(file);
                 return -1;
             }
         }
     }
     
     // 读取边数据
     while (fgets(line, sizeof(line), file) != NULL) {
         if (sscanf(line, "%d %d %d %f", &edge_id, &start_node, &end_node, &distance) == 4) {
             if (add_edge(edge_id, start_node, end_node, distance) != 0) {
                 fprintf(stderr, "Error: Failed to add edge %d.\n", edge_id);
                 fclose(file);
                 return -1;
             }
         }
     }
     
     fclose(file);
     
     printf("Map data loaded: %d nodes, %d edges.\n", graph.num_nodes, graph.num_edges);
     return 0;
 }
 
 // 添加节点到图
 int add_node(int node_id, int area_id, float x, float y) {
     int i;
     
     // 检查节点ID是否已存在
     for (i = 0; i < graph.num_nodes; i++) {
         if (graph.nodes[i].node_id == node_id) {
             fprintf(stderr, "Warning: Node %d already exists.\n", node_id);
             return -1;
         }
     }
     
     // 检查是否达到最大节点数量
     if (graph.num_nodes >= MAX_NODES) {
         fprintf(stderr, "Error: Maximum number of nodes reached.\n");
         return -1;
     }
     
     // 添加新节点
     graph.nodes[graph.num_nodes].node_id = node_id;
     graph.nodes[graph.num_nodes].area_id = area_id;
     graph.nodes[graph.num_nodes].x = x;
     graph.nodes[graph.num_nodes].y = y;
     graph.num_nodes++;
     
     return 0;
 }
 
 // 添加边到图
 int add_edge(int edge_id, int start_node, int end_node, float distance) {
     int i;
     
     // 检查边ID是否已存在
     for (i = 0; i < graph.num_edges; i++) {
         if (graph.edges[i].edge_id == edge_id) {
             fprintf(stderr, "Warning: Edge %d already exists.\n", edge_id);
             return -1;
         }
     }
     
     // 检查是否达到最大边数量
     if (graph.num_edges >= MAX_EDGES) {
         fprintf(stderr, "Error: Maximum number of edges reached.\n");
         return -1;
     }
     
     // 检查起始节点和终止节点是否存在
     int start_found = 0, end_found = 0;
     for (i = 0; i < graph.num_nodes; i++) {
         if (graph.nodes[i].node_id == start_node) {
             start_found = 1;
         }
         if (graph.nodes[i].node_id == end_node) {
             end_found = 1;
         }
     }
     
     if (!start_found || !end_found) {
         fprintf(stderr, "Error: Start node %d or end node %d not found.\n", start_node, end_node);
         return -1;
     }
     
     // 添加新边
     graph.edges[graph.num_edges].edge_id = edge_id;
     graph.edges[graph.num_edges].start_node = start_node;
     graph.edges[graph.num_edges].end_node = end_node;
     graph.edges[graph.num_edges].distance = distance;
     graph.edges[graph.num_edges].risk_factor = 0.0; // 初始风险因子为0
     graph.num_edges++;
     
     return 0;
 }
 
 // 根据环境模型更新边的风险因子
 int update_edge_risks(const EnvironmentalData_t *env_data) {
     int i, j;
     
     if (env_data == NULL) {
         fprintf(stderr, "Error: Environmental data is NULL.\n");
         return -1;
     }
     
     // 遍历所有边
     for (i = 0; i < graph.num_edges; i++) {
         int start_area_id = -1, end_area_id = -1;
         
         // 获取边的起始和终止节点对应的区域ID
         for (j = 0; j < graph.num_nodes; j++) {
             if (graph.nodes[j].node_id == graph.edges[i].start_node) {
                 start_area_id = graph.nodes[j].area_id;
             }
             if (graph.nodes[j].node_id == graph.edges[i].end_node) {
                 end_area_id = graph.nodes[j].area_id;
             }
         }
         
         // 这里需要一个机制来获取每个area_id对应的危险等级
         // 假设我们有一个函数 get_hazard_level_by_area_id(area_id)
         // 由于目前没有环境模型，我们暂时使用一个简化的逻辑
         // 实际应用中，这里应该从数据融合模块获取最新的环境危险等级
         float start_risk = 0.0; // Placeholder
         float end_risk = 0.0;   // Placeholder
 
         // For demonstration, let\"s assume higher TVOC/eCO2 means higher risk
         // This part needs to be integrated with the actual environment model from data_fusion
         // if (start_area_id != -1) {
         //     start_risk = get_hazard_level_by_area_id(start_area_id);
         // }
         // if (end_area_id != -1) {
         //     end_risk = get_hazard_level_by_area_id(end_area_id);
         // }
 
         // Using environmental data directly for a simplified risk calculation
         // This is a temporary solution until a full environment model is available
         start_risk = (float)(env_data->tvoc_ppb + env_data->eco2_ppm) / 2000.0; // Example risk calculation
         end_risk = start_risk; // Assuming risk is uniform across the edge for simplicity
 
         // 取两个区域风险的最大值作为边的风险因子
         graph.edges[i].risk_factor = (start_risk > end_risk) ? start_risk : end_risk;
 
         // Ensure risk factor is within a reasonable range (e.g., 0.0 to 1.0)
         if (graph.edges[i].risk_factor > 1.0) graph.edges[i].risk_factor = 1.0;
         if (graph.edges[i].risk_factor < 0.0) graph.edges[i].risk_factor = 0.0;
     }
     
     printf("Edge risks updated based on environmental data.\n");
     return 0;
 }
 
 // 启发函数：估计从当前节点到目标节点的距离
 static float heuristic(int current_node_idx, int target_node_idx) {
     // 使用欧几里得距离作为启发函数
     float dx = graph.nodes[current_node_idx].x - graph.nodes[target_node_idx].x;
     float dy = graph.nodes[current_node_idx].y - graph.nodes[target_node_idx].y;
     return sqrt(dx * dx + dy * dy);
 }
 
 // 查找节点在graph.nodes数组中的索引
 static int find_node_index_by_id(int node_id) {
     for (int i = 0; i < graph.num_nodes; i++) {
         if (graph.nodes[i].node_id == node_id) {
             return i;
         }
     }
     return -1;
 }
 
 // 使用A*算法计算最优路径
 int find_safe_path(int start_area_id, int end_area_id, path_t *path) {
     int i, j, u_idx, v_idx;
     int start_node_id = -1, end_node_id = -1;
 
     // Find start and end node IDs based on area IDs
     for (i = 0; i < graph.num_nodes; i++) {
         if (graph.nodes[i].area_id == start_area_id) {
             start_node_id = graph.nodes[i].node_id;
         }
         if (graph.nodes[i].area_id == end_area_id) {
             end_node_id = graph.nodes[i].node_id;
         }
     }
 
     if (start_node_id == -1 || end_node_id == -1) {
         fprintf(stderr, "Error: Start area %d or end area %d not found in map.\n", start_area_id, end_area_id);
         return -1;
     }
 
     u_idx = find_node_index_by_id(start_node_id);
     v_idx = find_node_index_by_id(end_node_id);
 
     if (u_idx == -1 || v_idx == -1) {
         fprintf(stderr, "Error: Internal node index lookup failed.\n");
         return -1;
     }
 
     float g_score[MAX_NODES]; // Actual cost from start to current node
     float f_score[MAX_NODES]; // g_score + heuristic
     int prev_node_idx[MAX_NODES]; // Previous node in the optimal path
     bool open_set[MAX_NODES]; // Nodes to be evaluated
     bool closed_set[MAX_NODES]; // Nodes already evaluated
 
     // Initialize scores and sets
     for (i = 0; i < graph.num_nodes; i++) {
         g_score[i] = FLT_MAX;
         f_score[i] = FLT_MAX;
         prev_node_idx[i] = -1;
         open_set[i] = false;
         closed_set[i] = false;
     }
 
     g_score[u_idx] = 0;
     f_score[u_idx] = heuristic(u_idx, v_idx);
     open_set[u_idx] = true;
 
     while (true) {
         // Find node with the lowest f_score in open_set
         float min_f_score = FLT_MAX;
         int current_idx = -1;
         for (i = 0; i < graph.num_nodes; i++) {
             if (open_set[i] && f_score[i] < min_f_score) {
                 min_f_score = f_score[i];
                 current_idx = i;
             }
         }
 
         if (current_idx == -1) {
             // Open set is empty, no path found
             fprintf(stderr, "Error: No path found from area %d to area %d.\n", start_area_id, end_area_id);
             return -1;
         }
 
         if (current_idx == v_idx) {
             // Path found
             break;
         }
 
         open_set[current_idx] = false;
         closed_set[current_idx] = true;
 
         // Explore neighbors
         for (j = 0; j < graph.num_edges; j++) {
             if (graph.edges[j].start_node == graph.nodes[current_idx].node_id) {
                 int neighbor_node_id = graph.edges[j].end_node;
                 int neighbor_idx = find_node_index_by_id(neighbor_node_id);
 
                 if (neighbor_idx != -1 && !closed_set[neighbor_idx]) {
                     // Calculate tentative g_score
                     float risk_weight = 1.0 + 10.0 * graph.edges[j].risk_factor; // Apply risk factor
                     float tentative_g_score = g_score[current_idx] + graph.edges[j].distance * risk_weight;
 
                     if (!open_set[neighbor_idx] || tentative_g_score < g_score[neighbor_idx]) {
                         prev_node_idx[neighbor_idx] = current_idx;
                         g_score[neighbor_idx] = tentative_g_score;
                         f_score[neighbor_idx] = g_score[neighbor_idx] + heuristic(neighbor_idx, v_idx);
                         open_set[neighbor_idx] = true;
                     }
                 }
             }
         }
     }
 
     // Reconstruct path
     int current_path_node_idx = v_idx;
     int temp_path_nodes_idx[MAX_PATH_LENGTH]; // Store indices of nodes in the path
     int temp_path_len = 0;
 
     while (current_path_node_idx != -1) {
         if (temp_path_len >= MAX_PATH_LENGTH) {
             fprintf(stderr, "Error: Path exceeds maximum length.\n");
             return -1;
         }
         temp_path_nodes_idx[temp_path_len++] = current_path_node_idx;
         current_path_node_idx = prev_node_idx[current_path_node_idx];
     }
 
     // Reverse path and populate path_t structure
     path->num_nodes = temp_path_len;
     path->total_distance = 0.0;
     path->total_risk = 0.0;
     path->timestamp = time(NULL);
 
     for (i = 0; i < temp_path_len; i++) {
         int node_array_idx = temp_path_nodes_idx[temp_path_len - 1 - i];
         path->nodes[i].node_id = graph.nodes[node_array_idx].node_id;
         path->nodes[i].x = graph.nodes[node_array_idx].x;
         path->nodes[i].y = graph.nodes[node_array_idx].y;
         path->nodes[i].area_id = graph.nodes[node_array_idx].area_id;
 
         if (i > 0) {
             // Calculate distance and risk for this segment
             int prev_node_id = path->nodes[i-1].node_id;
             int current_node_id = path->nodes[i].node_id;
             for (j = 0; j < graph.num_edges; j++) {
                 if (graph.edges[j].start_node == prev_node_id && graph.edges[j].end_node == current_node_id) {
                     path->total_distance += graph.edges[j].distance;
                     path->total_risk += graph.edges[j].risk_factor;
                     break;
                 }
             }
         }
     }
     
     printf("Optimal path calculated using A* from area %d to area %d: %d nodes, distance %.2f, risk %.2f.\n", 
            start_area_id, end_area_id, path->num_nodes, path->total_distance, path->total_risk);
     return 0;
 }
 
 // 清理路径规划模块
 void path_planning_cleanup(void) {
     // 释放资源
     memset(&graph, 0, sizeof(graph));
     
     printf("Path planning module cleaned up.\n");
 }
 
 // Example usage (for testing purposes)
 #ifdef PATH_PLANNING_TEST
 #include <unistd.h>
 
 int main() {
     printf("Path Planning Test Start.\n");
 
     // Initialize path planning module
     if (path_planning_init() != 0) {
         fprintf(stderr, "Failed to initialize path planning module.\n");
         return -1;
     }
 
     // Load dummy map data (replace with your actual map file)
     // For testing, we can manually add some nodes and edges if no file is available
     // 假设我们有一个简单的地图：
     // 节点：
     // ID | Area | X | Y
     // ---|------|---|---
     // 1  | 101  | 0 | 0
     // 2  | 102  | 10| 0
     // 3  | 103  | 10| 10
     // 4  | 104  | 0 | 10
     // 边：
     // ID | Start | End | Distance
     // ---|-------|-----|---------
     // 1  | 1     | 2   | 10.0
     // 2  | 2     | 3   | 10.0
     // 3  | 3     | 4   | 10.0
     // 4  | 4     | 1   | 10.0
     // 5  | 1     | 3   | 14.14
 
     // Manually add nodes
     add_node(1, 101, 0, 0);
     add_node(2, 102, 10, 0);
     add_node(3, 103, 10, 10);
     add_node(4, 104, 0, 10);
 
     // Manually add edges
     add_edge(1, 1, 2, 10.0);
     add_edge(2, 2, 3, 10.0);
     add_edge(3, 3, 4, 10.0);
     add_edge(4, 4, 1, 10.0);
     add_edge(5, 1, 3, 14.14);
 
     printf("Dummy map data loaded.\n");
 
     // Create dummy environmental data for risk update
     EnvironmentalData_t dummy_env_data = {
         .tvoc_ppb = 500, // Example high TVOC
         .eco2_ppm = 800, // Example high eCO2
         .mq2_voltage = 1.5, // Example voltage
         .mq2_concentration = 100.0 // Example concentration
     };
 
     // Update edge risks based on dummy environmental data
     if (update_edge_risks(&dummy_env_data) != 0) {
         fprintf(stderr, "Failed to update edge risks.\n");
         return -1;
     }
 
     path_t evacuation_path;
     int start_area = 101; // Example start area
     int end_area = 103;   // Example end area
 
     printf("Finding safe path from area %d to %d...\n", start_area, end_area);
     if (find_safe_path(start_area, end_area, &evacuation_path) == 0) {
         printf("Path found:\n");
         for (int i = 0; i < evacuation_path.num_nodes; i++) {
             printf("  Node ID: %d (Area: %d, X: %.1f, Y: %.1f)\n", 
                    evacuation_path.nodes[i].node_id,
                    evacuation_path.nodes[i].area_id,
                    evacuation_path.nodes[i].x,
                    evacuation_path.nodes[i].y);
         }
         printf("Total Distance: %.2f, Total Risk: %.2f\n", evacuation_path.total_distance, evacuation_path.total_risk);
         
         // int direction = get_direction_from_path(&evacuation_path);
         // printf("Suggested initial direction (LCD mapping): %d\n", direction);
 
     } else {
         fprintf(stderr, "Failed to find a safe path.\n");
     }
 
     path_planning_cleanup();
     printf("Path Planning Test End.\n");
     return 0;
 }
 #endif // PATH_PLANNING_TEST




// 从路径获取方向
int get_direction_from_path(const path_t *path) {
    if (path == NULL || path->num_nodes < 2) {
        return -1; // Invalid path
    }

    // 获取路径中第一个和第二个节点
    float x1 = path->nodes[0].x;
    float y1 = path->nodes[0].y;
    float x2 = path->nodes[1].x;
    float y2 = path->nodes[1].y;

    // 计算方向向量
    float dx = x2 - x1;
    float dy = y2 - y1;

    // 简化方向判断，只返回0-3，分别代表 East, North, West, South
    // 0: East (>)
    // 1: North (^)
    // 2: West (<)
    // 3: South (v)

    if (fabs(dx) > fabs(dy)) {
        // Horizontal movement is dominant
        if (dx > 0) {
            return 0; // East
        } else {
            return 2; // West
        }
    } else {
        // Vertical movement is dominant
        if (dy > 0) {
            return 3; // South
        } else {
            return 1; // North
        }
    }
}


