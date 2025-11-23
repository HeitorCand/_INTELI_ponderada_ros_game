# Culling Games - Navegação Autônoma (C++)

## Descrição

Este pacote ROS2 implementa navegação autônoma para o jogo Culling Games usando C++ e o algoritmo A*.

## Estrutura do Pacote

```
cg_autonomous/
├── CMakeLists.txt
├── package.xml
├── include/
│   └── cg_autonomous/
│       └── pathfinding.hpp      # Header do algoritmo A*
└── src/
    ├── pathfinding.cpp          # Implementação do A*
    └── autonomous_navigator.cpp # Nó ROS2 principal
```

## Compilação

```bash
cd /Users/heitor.candido/Documents/Ponderadas/culling_games
colcon build --packages-select cg_autonomous
source install/setup.bash
```

## Uso

### Execução Básica

**Terminal 1 - Inicie o jogo:**
```bash
source install/setup.bash
ros2 run cg maze
```

**Terminal 2 - Execute o navegador autônomo:**
```bash
source install/setup.bash
ros2 run cg_autonomous autonomous_navigator
```

### Testando com Diferentes Mapas

**Mapa específico:**
```bash
# Terminal 1
ros2 run cg maze -- --map 5.csv

# Terminal 2
ros2 run cg_autonomous autonomous_navigator
```

**Mapa gerado aleatoriamente:**
```bash
# Terminal 1
ros2 run cg maze -- --generate

# Terminal 2
ros2 run cg_autonomous autonomous_navigator
```

## Funcionalidades

### 1. Pathfinding (A*)

A biblioteca `pathfinding` implementa:

- **Algoritmo A*** para busca de caminho otimizado
- **Heurística Manhattan** para estimativa de distância
- **Exploração de vizinhos** nas 4 direções cardeais
- **Conversão de caminho** para comandos de direção

### 2. Autonomous Navigator

O nó principal:

1. Obtém o mapa completo via serviço `/get_map`
2. Identifica posições do robô e alvo
3. Calcula caminho usando A*
4. Executa movimentos via serviço `/move_command`
5. Monitora progresso até alcançar objetivo

## Detalhes de Implementação

### Classes Principais

#### `Node`
```cpp
class Node {
    Position position;           // (row, col)
    std::shared_ptr<Node> parent; // Para reconstruir caminho
    double g;                     // Custo do início
    double h;                     // Heurística
    double f;                     // f = g + h
};
```

#### `Pathfinding`
Métodos estáticos:
- `astar()`: Implementação do A*
- `heuristic()`: Distância Manhattan
- `getNeighbors()`: Vizinhos válidos
- `pathToDirections()`: Conversão para comandos
- `unflattenGrid()`: Deserialização do mapa
- `findInGrid()`: Busca de elementos

### Algoritmo A*

```cpp
std::optional<std::vector<Position>> astar(
    const std::vector<std::vector<std::string>>& grid,
    const Position& start,
    const Position& goal
)
```

**Características:**
- Usa `std::priority_queue` para lista aberta
- `std::unordered_set` para lista fechada
- `std::unordered_map` para g-scores
- Retorna `std::optional` (caminho ou nullopt)

### ROS2 Integration

**Serviços utilizados:**
- `/get_map` - Obter mapa completo
- `/move_command` - Executar movimento

**Mensagens:**
- `cg_interfaces/srv/GetMap`
- `cg_interfaces/srv/MoveCmd`
- `cg_interfaces/msg/RobotSensors`

## Exemplo de Saída

```
[INFO] [autonomous_navigator_cpp]: Autonomous Navigator (C++) inicializado
[INFO] [autonomous_navigator_cpp]: Todos os serviços estão disponíveis!
[INFO] [autonomous_navigator_cpp]: ==================================================
[INFO] [autonomous_navigator_cpp]: Iniciando navegação autônoma...
[INFO] [autonomous_navigator_cpp]: ==================================================
[INFO] [autonomous_navigator_cpp]: Requisitando mapa...
[INFO] [autonomous_navigator_cpp]: Mapa recebido: 20x20
[INFO] [autonomous_navigator_cpp]: Posição do robô: (1, 1)
[INFO] [autonomous_navigator_cpp]: Posição do alvo: (18, 18)
[INFO] [autonomous_navigator_cpp]: Calculando caminho com A*...
[INFO] [autonomous_navigator_cpp]: Caminho encontrado com 35 passos!
[INFO] [autonomous_navigator_cpp]: Executando 35 movimentos...
[INFO] [autonomous_navigator_cpp]: Sequência: right right down down right ...
[INFO] [autonomous_navigator_cpp]: Passo 1/35: right
[INFO] [autonomous_navigator_cpp]: Movimento right executado com sucesso
...
[INFO] [autonomous_navigator_cpp]: 🎯 OBJETIVO ALCANÇADO! 🎯
[INFO] [autonomous_navigator_cpp]: ==================================================
[INFO] [autonomous_navigator_cpp]: ✅ Navegação concluída com sucesso!
[INFO] [autonomous_navigator_cpp]: ==================================================
```

## Dependências

- `rclcpp` - Cliente ROS2 C++
- `cg_interfaces` - Interfaces personalizadas
- `std_msgs` - Mensagens padrão
- C++17 ou superior

## Vantagens do C++

1. **Performance**: Execução mais rápida que Python
2. **Tipagem forte**: Menos erros em runtime
3. **STL**: Estruturas de dados eficientes
4. **Memory management**: Controle fino com smart pointers
5. **Compatibilidade**: Integração nativa com ROS2

## Troubleshooting

### Erro de compilação
```bash
# Limpe e recompile
rm -rf build/ install/ log/
colcon build --packages-select cg_autonomous
```

### Serviço não encontrado
```bash
# Verifique se o jogo está rodando
ros2 service list | grep -E "(get_map|move_command)"
```

### Caminho não encontrado
- Verifique se o mapa tem solução
- Use um mapa diferente para testar

## Comparação Python vs C++

| Aspecto | Python (cg) | C++ (cg_autonomous) |
|---------|-------------|---------------------|
| Performance | Moderada | Alta |
| Desenvolvimento | Rápido | Médio |
| Tipagem | Dinâmica | Estática |
| Memória | GC | Manual/Smart Ptrs |
| Integração ROS2 | rclpy | rclcpp |

## Licença

MIT
