#ifndef CG_EXPLORER_MAPPER_HPP
#define CG_EXPLORER_MAPPER_HPP

#include <array>
#include <string>
#include <vector>

namespace cg_explorer
{

class Mapper
{
public:
    static constexpr int MAP_SIZE = 29;
    
    Mapper();
    
    // Update a cell in the map
    void updateCell(int x, int y, char value);
    
    // Get the value at a cell
    char getCell(int x, int y) const;
    
    // Check if a position is valid
    bool isValidPosition(int x, int y) const;
    
    // Print the current map state
    void printMap() const;
    
    // Check if the map is fully explored (no '?' remaining)
    bool isMapComplete() const;
    
    // Get the map as a 2D vector
    std::vector<std::vector<char>> getMapData() const;
    
    // Count how many cells are still unknown
    int countUnknownCells() const;
    
private:
    std::array<std::array<char, MAP_SIZE>, MAP_SIZE> map_;
};

} // namespace cg_explorer

#endif // CG_EXPLORER_MAPPER_HPP
