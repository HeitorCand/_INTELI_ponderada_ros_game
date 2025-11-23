#ifndef CG_EXPLORER_PATHFINDER_HPP
#define CG_EXPLORER_PATHFINDER_HPP

#include "cg_explorer/mapper.hpp"
#include <vector>
#include <string>

namespace cg_explorer
{

// Estrutura para representar uma posição no mapa
struct Position
{
    int x, y;
    
    // Operadores de comparação
    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Position& other) const
    {
        return !(*this == other);
    }
    
    bool operator<(const Position& other) const
    {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

// Classe para algoritmos de pathfinding
class Pathfinder
{
public:
    Pathfinder(Mapper& mapper);
    
    // Encontra caminho ótimo usando BFS
    std::vector<std::string> findPath(Position start, Position goal);
    
    // Encontra o próximo movimento durante a exploração
    std::string findNextExplorationMove(const Position& current_pos);
    
private:
    // Converte sequência de posições em direções
    std::vector<std::string> pathToDirections(const std::vector<Position>& path);
    
    // Calcula distância Manhattan entre duas posições
    int manhattanDistance(Position a, Position b);
    
    Mapper& mapper_;
};

} // namespace cg_explorer

#endif // CG_EXPLORER_PATHFINDER_HPP
