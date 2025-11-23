#include "cg_explorer/pathfinder.hpp"
#include <queue>
#include <set>
#include <map>
#include <algorithm>

namespace cg_explorer
{

Pathfinder::Pathfinder(Mapper& mapper) : mapper_(mapper)
{
}

std::vector<std::string> Pathfinder::findPath(Position start, Position goal)
{
    if (start == goal) return {};
    
    // Debug: verificar células de início e fim
    char start_cell = mapper_.getCell(start.x, start.y);
    char goal_cell = mapper_.getCell(goal.x, goal.y);
    
    std::queue<Position> q;
    std::map<Position, Position> parent;
    std::set<Position> visited;
    
    q.push(start);
    visited.insert(start);
    
    bool found = false;
    while (!q.empty())
    {
        Position current = q.front();
        q.pop();
        
        std::vector<Position> neighbors = {
            {current.x, current.y - 1},
            {current.x, current.y + 1},
            {current.x - 1, current.y},
            {current.x + 1, current.y}
        };
        
        for (const auto& neighbor : neighbors)
        {
            if (!mapper_.isValidPosition(neighbor.x, neighbor.y)) continue;
            if (visited.count(neighbor) > 0) continue;
            
            char cell = mapper_.getCell(neighbor.x, neighbor.y);
            
            if (cell != 'f' && cell != 'r' && cell != 't') continue;
            
            visited.insert(neighbor);
            parent[neighbor] = current;
            
            if (neighbor == goal)
            {
                found = true;
                break;
            }
            
            q.push(neighbor);
        }
        
        if (found) break;
    }
    
    if (!found)
    {
        return {};
    }
    
    std::vector<Position> path;
    Position current = goal;
    while (current != start)
    {
        path.push_back(current);
        current = parent[current];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    
    return pathToDirections(path);
}

std::string Pathfinder::findNextExplorationMove(const Position& current_pos)
{
    struct MoveOption {
        std::string direction;
        int dx, dy;
    };
    
    std::vector<MoveOption> moves = {
        {"right", 1, 0},
        {"down", 0, 1},
        {"up", 0, -1},
        {"left", -1, 0}
    };
    
    // Prioridade 1: células desconhecidas
    for (const auto& move : moves)
    {
        int nx = current_pos.x + move.dx;
        int ny = current_pos.y + move.dy;
        
        if (!mapper_.isValidPosition(nx, ny)) continue;
        
        char cell = mapper_.getCell(nx, ny);
        
        if (cell == '?')
        {
            return move.direction;
        }
    }
    
    // Prioridade 2: células livres perto de desconhecidas
    for (const auto& move : moves)
    {
        int nx = current_pos.x + move.dx;
        int ny = current_pos.y + move.dy;
        
        if (!mapper_.isValidPosition(nx, ny)) continue;
        
        char cell = mapper_.getCell(nx, ny);
        
        if (cell != 'f' && cell != 'r') continue;
        
        bool has_unknown_neighbors = false;
        std::vector<Position> check_positions = {
            {nx, ny - 1}, {nx, ny + 1}, {nx - 1, ny}, {nx + 1, ny}
        };
        
        for (const auto& pos : check_positions)
        {
            if (mapper_.isValidPosition(pos.x, pos.y) && 
                mapper_.getCell(pos.x, pos.y) == '?')
            {
                has_unknown_neighbors = true;
                break;
            }
        }
        
        if (has_unknown_neighbors)
        {
            return move.direction;
        }
    }
    
    // BFS para voltar
    std::queue<Position> queue;
    std::map<Position, Position> came_from;
    std::set<Position> visited;
    
    queue.push(current_pos);
    visited.insert(current_pos);
    
    Position target_pos = {-1, -1};
    bool found_target = false;
    
    while (!queue.empty() && !found_target)
    {
        Position current = queue.front();
        queue.pop();
        
        std::vector<Position> neighbors = {
            {current.x, current.y - 1}, {current.x, current.y + 1},
            {current.x - 1, current.y}, {current.x + 1, current.y}
        };
        
        for (const auto& neighbor : neighbors)
        {
            if (mapper_.isValidPosition(neighbor.x, neighbor.y) &&
                mapper_.getCell(neighbor.x, neighbor.y) == '?')
            {
                target_pos = current;
                found_target = true;
                break;
            }
        }
        
        if (found_target) break;
        
        for (const auto& neighbor : neighbors)
        {
            if (!mapper_.isValidPosition(neighbor.x, neighbor.y)) continue;
            
            char cell = mapper_.getCell(neighbor.x, neighbor.y);
            if ((cell == 'f' || cell == 'r') && visited.find(neighbor) == visited.end())
            {
                queue.push(neighbor);
                visited.insert(neighbor);
                came_from[neighbor] = current;
            }
        }
    }
    
    if (!found_target) return "";
    
    std::vector<Position> path;
    Position current = target_pos;
    while (came_from.find(current) != came_from.end())
    {
        path.push_back(current);
        current = came_from[current];
    }
    
    if (path.empty()) return "";
    
    Position next_pos = path.back();
    int dx = next_pos.x - current_pos.x;
    int dy = next_pos.y - current_pos.y;
    
    std::string direction;
    if (dx == 1) direction = "right";
    else if (dx == -1) direction = "left";
    else if (dy == 1) direction = "down";
    else if (dy == -1) direction = "up";
    
    return direction;
}

std::vector<std::string> Pathfinder::pathToDirections(const std::vector<Position>& path)
{
    std::vector<std::string> directions;
    if (path.size() < 2)
        return directions;
    
    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        int dx = path[i + 1].x - path[i].x;
        int dy = path[i + 1].y - path[i].y;
        
        if (dy == -1) directions.push_back("up");
        else if (dy == 1) directions.push_back("down");
        else if (dx == -1) directions.push_back("left");
        else if (dx == 1) directions.push_back("right");
    }
    
    return directions;
}

int Pathfinder::manhattanDistance(Position a, Position b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

} // namespace cg_explorer
