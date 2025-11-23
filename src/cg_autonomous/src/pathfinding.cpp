#include "cg_autonomous/pathfinding.hpp"
#include <algorithm>

namespace cg_autonomous {

// Retorna vizinhos válidos de uma posição
std::vector<Position> Pathfinding::getNeighbors(
    const Position& position,
    const std::vector<std::vector<std::string>>& grid
) {
    std::vector<Position> neighbors;
    int row = position.first;
    int col = position.second;

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    for (int k = 0; k < 4; ++k) {
        int nr = row + dr[k];
        int nc = col + dc[k];
        if (nr >= 0 && nr < static_cast<int>(grid.size()) &&
            nc >= 0 && nc < static_cast<int>(grid[0].size())) {
            const std::string& cell = grid[nr][nc];
            if (cell == "f" || cell == "r" || cell == "t") {
                neighbors.emplace_back(nr, nc);
            }
        }
    }
    return neighbors;
}

// Busca caminho usando BFS
std::optional<std::vector<Position>> Pathfinding::astar(
    const std::vector<std::vector<std::string>>& grid,
    const Position& start,
    const Position& goal
) {
    if (start == goal) {
        return std::vector<Position>{start};
    }

    std::queue<Position> q;
    std::unordered_map<Position, Position, PositionHash> parent;
    std::unordered_set<Position, PositionHash> visited;

    q.push(start);
    visited.insert(start);

    bool found = false;
    while (!q.empty()) {
        Position cur = q.front();
        q.pop();

        for (const auto& nb : getNeighbors(cur, grid)) {
            if (visited.count(nb) > 0) continue;
            visited.insert(nb);
            parent[nb] = cur;
            if (nb == goal) {
                found = true;
                break;
            }
            q.push(nb);
        }
        if (found) break;
    }

    if (!found) return std::nullopt;

    // Reconstrói o caminho
    std::vector<Position> path;
    Position cur = goal;
    while (cur != start) {
        path.push_back(cur);
        cur = parent[cur];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

// Converte posições em direções
std::vector<std::string> Pathfinding::pathToDirections(const std::vector<Position>& path) {
    std::vector<std::string> directions;
    if (path.size() < 2) return directions;

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        int dr = path[i + 1].first - path[i].first;
        int dc = path[i + 1].second - path[i].second;
        if (dr == -1) directions.push_back("up");
        else if (dr == 1) directions.push_back("down");
        else if (dc == -1) directions.push_back("left");
        else if (dc == 1) directions.push_back("right");
    }
    return directions;
}

// Converte array 1D em matriz 2D
std::vector<std::vector<std::string>> Pathfinding::unflattenGrid(
    const std::vector<std::string>& flattened,
    int rows,
    int cols
) {
    std::vector<std::vector<std::string>> grid(rows, std::vector<std::string>(cols));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            grid[i][j] = flattened[i * cols + j];
    return grid;
}

// Encontra posição de um elemento no grid
std::optional<Position> Pathfinding::findInGrid(
    const std::vector<std::vector<std::string>>& grid,
    const std::string& target
) {
    for (int i = 0; i < static_cast<int>(grid.size()); ++i)
        for (int j = 0; j < static_cast<int>(grid[i].size()); ++j)
            if (grid[i][j] == target) return Position(i, j);
    return std::nullopt;
}

} // namespace cg_autonomous
