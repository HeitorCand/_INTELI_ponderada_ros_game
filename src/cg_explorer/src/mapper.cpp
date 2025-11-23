#include "cg_explorer/mapper.hpp"
#include <iostream>
#include <iomanip>

namespace cg_explorer
{

Mapper::Mapper()
{
    // Inicializa o mapa com células desconhecidas '?'
    for (int i = 0; i < MAP_SIZE; ++i)
    {
        for (int j = 0; j < MAP_SIZE; ++j)
        {
            map_[i][j] = '?';
        }
    }
}

void Mapper::updateCell(int x, int y, char value)
{
    // Atualiza o valor de uma célula no mapa
    if (isValidPosition(x, y))
    {
        map_[y][x] = value;
    }
}

char Mapper::getCell(int x, int y) const
{
    // Retorna o valor de uma célula no mapa
    if (isValidPosition(x, y))
    {
        return map_[y][x];
    }
    return 'b'; // Retorna 'b' para posições inválidas (barreira)
}

bool Mapper::isValidPosition(int x, int y) const
{
    // Verifica se a posição está dentro dos limites do mapa
    return x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE;
}

void Mapper::printMap() const
{
    std::cout << "\n";
    std::cout << "====== ESTADO ATUAL DO MAPA ======\n";
    std::cout << "Células desconhecidas: " << countUnknownCells() << "/" << (MAP_SIZE * MAP_SIZE) << "\n";
    std::cout << "   ";
    
    // Imprime números das colunas
    for (int i = 0; i < MAP_SIZE; ++i)
    {
        std::cout << std::setw(2) << i << " ";
    }
    std::cout << "\n";
    
    // Imprime o mapa linha por linha
    for (int y = 0; y < MAP_SIZE; ++y)
    {
        std::cout << std::setw(2) << y << " ";
        for (int x = 0; x < MAP_SIZE; ++x)
        {
            std::cout << " " << map_[y][x] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "================================\n\n";
}

bool Mapper::isMapComplete() const
{
    // Verifica se o mapa está completamente explorado (sem células desconhecidas)
    return countUnknownCells() == 0;
}

std::vector<std::vector<char>> Mapper::getMapData() const
{
    // Retorna uma cópia dos dados do mapa
    std::vector<std::vector<char>> result(MAP_SIZE, std::vector<char>(MAP_SIZE));
    for (int y = 0; y < MAP_SIZE; ++y)
    {
        for (int x = 0; x < MAP_SIZE; ++x)
        {
            result[y][x] = map_[y][x];
        }
    }
    return result;
}

int Mapper::countUnknownCells() const
{
    // Conta o número de células desconhecidas no mapa
    int count = 0;
    for (int y = 0; y < MAP_SIZE; ++y)
    {
        for (int x = 0; x < MAP_SIZE; ++x)
        {
            if (map_[y][x] == '?')
            {
                ++count;
            }
        }
    }
    return count;
}

} // namespace cg_explorer
