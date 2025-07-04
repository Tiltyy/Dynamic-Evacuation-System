#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 定义最大区域数和连接数
#define MAX_AREAS 100
#define MAX_CONNECTIONS 500
#define MAX_PATH_LENGTH 50
#define BUFFER_SIZE 256

// 区域结构体
typedef struct {
    int id;
    char name[50];
    int is_exit;
    float hazard_level;
} Area;

// 连接结构体
typedef struct {
    int from;
    int to;
    float distance;
    int blocked;
} Connection;

// 路径结构体
typedef struct {
    int area_ids[MAX_PATH_LENGTH];
    int length;
    float total_distance;
} EvacuationPath;

// 商场地图结构体
typedef struct {
    Area areas[MAX_AREAS];
    int area_count;
    Connection connections[MAX_CONNECTIONS];
    int connection_count;
} ShoppingMallMap;

// 实时监控结构体
typedef struct {
    ShoppingMallMap* mall_map;
    int running;
} RealTimeMonitor;

// 函数声明
int load_areas_from_file(ShoppingMallMap* map, const char* filename);
int load_connections_from_file(ShoppingMallMap* map, const char* filename);
int find_nearest_exit(ShoppingMallMap* map, int current_area);
EvacuationPath find_evacuation_path(ShoppingMallMap* map, int start, int end);
EvacuationPath update_path_dynamically(ShoppingMallMap* map, EvacuationPath path, int current_area);
void print_path(EvacuationPath path);
void start_monitor(RealTimeMonitor* monitor);
void stop_monitor(RealTimeMonitor* monitor);

// 从文件加载区域数据
int load_areas_from_file(ShoppingMallMap* map, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "无法打开文件 %s: %s\n", filename, strerror(errno));
        return 0;
    }
    
    map->area_count = 0;
    char buffer[BUFFER_SIZE];
    
    while (fgets(buffer, BUFFER_SIZE, file) && map->area_count < MAX_AREAS) {
        Area area;
        if (sscanf(buffer, "%d,%49[^,],%d,%f", 
                  &area.id, area.name, &area.is_exit, &area.hazard_level) == 4) {
            map->areas[map->area_count++] = area;
        }
    }
    
    fclose(file);
    return 1;
}

// 从文件加载连接数据
int load_connections_from_file(ShoppingMallMap* map, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "无法打开文件 %s: %s\n", filename, strerror(errno));
        return 0;
    }
    
    map->connection_count = 0;
    char buffer[BUFFER_SIZE];
    
    while (fgets(buffer, BUFFER_SIZE, file) && map->connection_count < MAX_CONNECTIONS) {
        Connection conn;
        if (sscanf(buffer, "%d,%d,%f,%d", 
                  &conn.from, &conn.to, &conn.distance, &conn.blocked) == 4) {
            map->connections[map->connection_count++] = conn;
        }
    }
    
    fclose(file);
    return 1;
}

// 查找最近的安全出口
int find_nearest_exit(ShoppingMallMap* map, int current_area) {
    // 简化实现：实际应使用图算法计算最短路径
    for (int i = 0; i < map->area_count; i++) {
        if (map->areas[i].is_exit) {
            return map->areas[i].id;
        }
    }
    return -1; // 没有找到出口
}

// 查找疏散路径（简化的Dijkstra算法）
EvacuationPath find_evacuation_path(ShoppingMallMap* map, int start, int end) {
    EvacuationPath path = {0};
    
    // 简化实现：实际应使用完整的路径规划算法
    path.area_ids[0] = start;
    path.area_ids[1] = end;
    path.length = 2;
    
    // 计算总距离
    for (int i = 0; i < path.length - 1; i++) {
        for (int j = 0; j < map->connection_count; j++) {
            if ((map->connections[j].from == path.area_ids[i] && 
                map->connections[j].to == path.area_ids[i+1]) ||
                (map->connections[j].from == path.area_ids[i+1] && 
                map->connections[j].to == path.area_ids[i])) {
                path.total_distance += map->connections[j].distance;
                break;
            }
        }
    }
    
    return path;
}

// 动态更新路径
EvacuationPath update_path_dynamically(ShoppingMallMap* map, EvacuationPath path, int current_area) {
    // 模拟动态更新：检查路径上的区域是否有危险
    for (int i = 0; i < path.length; i++) {
        for (int j = 0; j < map->area_count; j++) {
            if (map->areas[j].id == path.area_ids[i] && map->areas[j].hazard_level > 0.8) {
                // 如果区域危险，重新规划路径
                int nearest_exit = find_nearest_exit(map, current_area);
                return find_evacuation_path(map, current_area, nearest_exit);
            }
        }
    }
    
    // 路径无需更新
    return path;
}

// 打印路径
void print_path(EvacuationPath path) {
    printf("路径: ");
    for (int i = 0; i < path.length; i++) {
        printf("%d", path.area_ids[i]);
        if (i < path.length - 1) {
            printf(" -> ");
        }
    }
    printf("\n总距离: %.2f\n", path.total_distance);
}

// 启动实时监控
void start_monitor(RealTimeMonitor* monitor) {
    monitor->running = 1;
    printf("实时监控已启动\n");
    // 实际应用中会启动一个线程持续监控
}

// 停止实时监控
void stop_monitor(RealTimeMonitor* monitor) {
    monitor->running = 0;
    printf("实时监控已停止\n");
}

int main() {
    // 创建商场地图
    ShoppingMallMap mall_map;
    memset(&mall_map, 0, sizeof(ShoppingMallMap));
    
    // 加载地图数据
    if (!load_areas_from_file(&mall_map, "mall_areas.txt") ||
        !load_connections_from_file(&mall_map, "mall_connections.txt")) {
        fprintf(stderr, "无法加载商场地图数据\n");
        return 1;
    }
    
    // 启动实时监控
    RealTimeMonitor monitor = {&mall_map, 0};
    start_monitor(&monitor);
    
    // 模拟疏散请求
    int current_area = 101; // 假设用户当前在区域101
    
    // 第一次路径规划
    int nearest_exit = find_nearest_exit(&mall_map, current_area);
    if (nearest_exit == -1) {
        fprintf(stderr, "没有可用的安全出口!\n");
        return 1;
    }
    
    EvacuationPath path = find_evacuation_path(&mall_map, current_area, nearest_exit);
    printf("初始疏散路径:\n");
    print_path(path);
    
    // 模拟人员移动和动态更新
    for (int step = 0; step < 5; ++step) {
        printf("\n=== 移动到下一个区域 ===\n");
        
        // 移动到路径上的下一个区域
        if (path.length > 0) {
            current_area = path.area_ids[(step + 1) < path.length ? (step + 1) : (path.length - 1)];
        }
        
        // 动态更新路径
        EvacuationPath updated_path = update_path_dynamically(&mall_map, path, current_area);
        
        // 如果路径发生变化
        int path_changed = 0;
        if (updated_path.length != path.length) {
            path_changed = 1;
        } else {
            for (int i = 0; i < path.length; i++) {
                if (updated_path.area_ids[i] != path.area_ids[i]) {
                    path_changed = 1;
                    break;
                }
            }
        }
        
        if (path_changed) {
            printf("路径因环境变化而更新:\n");
            print_path(updated_path);
            path = updated_path;
        } else {
            printf("路径保持不变，继续沿原路径疏散\n");
        }
        
        // 模拟移动耗时
        printf("等待1秒...\n");
        // 在实际C环境中可使用适当的延时函数
    }
    
    // 停止监控
    stop_monitor(&monitor);
    
    return 0;
}