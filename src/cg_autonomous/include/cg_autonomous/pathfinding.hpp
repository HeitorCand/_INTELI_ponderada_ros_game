#ifndef CG_AUTONOMOUS_PATHFINDING_HPP
#define CG_AUTONOMOUS_PATHFINDING_HPP

#include <vector>
#include <string>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace cg_autonomous {

// Representa uma posição (linha, coluna)
using Position = std::pair<int, int>;

// Hash para usar Position em unordered_map/set
struct PositionHash {
    std::size_t operator()(const Position& pos) const {
        return std::hash<int>()(pos.first) ^ (std::hash<int>()(pos.second) << 1);
    }
};

class Pathfinding {
public:
    static std::vector<Position> getNeighbors(
        const Position& position, 
        const std::vector<std::vector<std::string>>& grid
    );

    static std::optional<std::vector<Position>> astar(
        const std::vector<std::vector<std::string>>& grid,
        const Position& start,
        const Position& goal
    );

    static std::vector<std::string> pathToDirections(const std::vector<Position>& path);

    static std::vector<std::vector<std::string>> unflattenGrid(
        const std::vector<std::string>& flattened,
        int rows,
        int cols
    );

    static std::optional<Position> findInGrid(
        const std::vector<std::vector<std::string>>& grid,
        const std::string& target
    );
};

} // namespace cg_autonomous

#endif // CG_AUTONOMOUS_PATHFINDING_HPP
