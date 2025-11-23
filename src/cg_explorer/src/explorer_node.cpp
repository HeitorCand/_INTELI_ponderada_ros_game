#include <rclcpp/rclcpp.hpp>
#include <cg_interfaces/msg/robot_sensors.hpp>
#include <cg_interfaces/srv/move_cmd.hpp>
#include <cg_interfaces/srv/reset.hpp>
#include "cg_explorer/mapper.hpp"
#include "cg_explorer/pathfinder.hpp"

#include <thread>
#include <chrono>

namespace cg_explorer
{

// Nó de exploração autônoma
class ExplorerNode : public rclcpp::Node
{
public:
    ExplorerNode() : Node("explorer_node"), 
                     pathfinder_(mapper_),
                     current_pos_{1, 1}, 
                     target_pos_{14, 14},
                     exploring_(true),
                     move_count_(0),
                     waiting_for_move_(false)
    {
        sensor_sub_ = this->create_subscription<cg_interfaces::msg::RobotSensors>(
            "/culling_games/robot_sensors",
            10,
            std::bind(&ExplorerNode::sensorCallback, this, std::placeholders::_1));
        
        move_client_ = this->create_client<cg_interfaces::srv::MoveCmd>("/move_command");
        reset_client_ = this->create_client<cg_interfaces::srv::Reset>("/reset");
        
        RCLCPP_INFO(this->get_logger(), "🤖 Explorador iniciado - Robô em (1,1), Alvo em (14,14)");
        
        while (!move_client_->wait_for_service(std::chrono::seconds(1)))
        {
            if (!rclcpp::ok()) return;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        exploreTimer_ = this->create_wall_timer(
            std::chrono::milliseconds(200),
            std::bind(&ExplorerNode::explorationLoop, this));
    }
    
private:
    // Callback dos sensores do robô
    void sensorCallback(const cg_interfaces::msg::RobotSensors::ConstSharedPtr msg)
    {
        updateMapFromSensors(msg);
        
        static int callback_count = 0;
        callback_count++;
        
        if (callback_count % 10 == 0)
        {
            mapper_.printMap();
        }
    }
    
    // Atualiza o mapa baseado nos dados dos sensores
    void updateMapFromSensors(const cg_interfaces::msg::RobotSensors::ConstSharedPtr msg)
    {
        int x = current_pos_.x;
        int y = current_pos_.y;
        
        mapper_.updateCell(x, y, 'r');
        
        struct SensorData {
            std::string value;
            int dx, dy;
        };
        
        std::vector<SensorData> sensors = {
            {msg->up, 0, -1},
            {msg->down, 0, 1},
            {msg->left, -1, 0},
            {msg->right, 1, 0},
            {msg->up_left, -1, -1},
            {msg->up_right, 1, -1},
            {msg->down_left, -1, 1},
            {msg->down_right, 1, 1}
        };
        
        for (const auto& sensor : sensors)
        {
            int nx = x + sensor.dx;
            int ny = y + sensor.dy;
            
            if (mapper_.isValidPosition(nx, ny))
            {
                char cell_value = '?';
                
                if (sensor.value == "f")
                {
                    cell_value = 'f';
                }
                else if (sensor.value == "b")
                {
                    cell_value = 'b';
                }
                else if (sensor.value == "t")
                {
                    cell_value = 't';
                }
                
                mapper_.updateCell(nx, ny, cell_value);
            }
        }
    }
    
    // Loop principal de exploração
    void explorationLoop()
    {
        if (!exploring_ || waiting_for_move_) return;
        
        if (mapper_.isMapComplete())
        {
            RCLCPP_INFO(this->get_logger(), "Mapa 100%% explorado!");
            mapper_.printMap();
            exploring_ = false;
            findAndExecutePath();
            return;
        }
        
        int unknown_cells = mapper_.countUnknownCells();
        if (unknown_cells <= 5 && unknown_cells > 0 && !map_exploration_complete_)
        {
            RCLCPP_INFO(this->get_logger(), "Mapa ~99%% completo! (%d células)", unknown_cells);
            mapper_.printMap();
            
            map_exploration_complete_ = true;
            exploring_ = false;
            resetGame();
            return;
        }
        
        std::string next_move = pathfinder_.findNextExplorationMove(current_pos_);
        
        if (next_move.empty())
        {
            int remaining = mapper_.countUnknownCells();
            
            if (remaining <= 5 && remaining > 0 && !map_exploration_complete_)
            {
                RCLCPP_INFO(this->get_logger(), "Mapa ~99%% completo! (%d células)", remaining);
                mapper_.printMap();
                
                map_exploration_complete_ = true;
                exploring_ = false;
                resetGame();
                return;
            }
            
            mapper_.printMap();
            exploring_ = false;
            return;
        }
        
        executeMove(next_move);
    }
    
    // Executa um movimento
    void executeMove(const std::string& direction)
    {
        waiting_for_move_ = true;
        pending_direction_ = direction;
        
        auto request = std::make_shared<cg_interfaces::srv::MoveCmd::Request>();
        request->direction = direction;
        
        auto result_future = move_client_->async_send_request(
            request,
            std::bind(&ExplorerNode::moveResponseCallback, this, std::placeholders::_1));
    }
    
    // Callback da resposta do movimento
    void moveResponseCallback(rclcpp::Client<cg_interfaces::srv::MoveCmd>::SharedFuture future)
    {
        auto result = future.get();
        waiting_for_move_ = false;
        
        if (result->success)
        {
            if (pending_direction_ == "up") current_pos_.y--;
            else if (pending_direction_ == "down") current_pos_.y++;
            else if (pending_direction_ == "left") current_pos_.x--;
            else if (pending_direction_ == "right") current_pos_.x++;
            
            move_count_++;
            
            if (move_count_ % 20 == 0)
            {
                mapper_.printMap();
            }
        }
        else
        {
            int target_x = current_pos_.x;
            int target_y = current_pos_.y;
            
            if (pending_direction_ == "up") target_y--;
            else if (pending_direction_ == "down") target_y++;
            else if (pending_direction_ == "left") target_x--;
            else if (pending_direction_ == "right") target_x++;
            
            if (mapper_.isValidPosition(target_x, target_y))
            {
                mapper_.updateCell(target_x, target_y, 'b');
            }
        }
    }
    
    // Reseta o jogo
    void resetGame()
    {
        if (!reset_client_->wait_for_service(std::chrono::seconds(2)))
        {
            RCLCPP_ERROR(this->get_logger(), "Serviço de reset indisponível!");
            return;
        }
        
        auto request = std::make_shared<cg_interfaces::srv::Reset::Request>();
        request->is_random = false;
        
        RCLCPP_INFO(this->get_logger(), "🔄 Resetando jogo...");
        
        auto future = reset_client_->async_send_request(request,
            std::bind(&ExplorerNode::resetResponseCallback, this, std::placeholders::_1));
    }
    
    // Callback da resposta do reset
    void resetResponseCallback(rclcpp::Client<cg_interfaces::srv::Reset>::SharedFuture future)
    {
        try
        {
            auto response = future.get();
            RCLCPP_INFO(this->get_logger(), "Jogo resetado!");
            
            current_pos_ = {1, 1};
            target_pos_ = {14, 14};
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            RCLCPP_INFO(this->get_logger(), "Executando caminho ótimo usando BFS...");
            findAndExecutePath();
        }
        catch (const std::exception& e)
        {
            RCLCPP_ERROR(this->get_logger(), "Erro ao resetar: %s", e.what());
        }
    }
    
    // Calcula e executa o caminho final
    void findAndExecutePath()
    {
        RCLCPP_INFO(this->get_logger(), "Buscando caminho de (%d,%d) para (%d,%d)", 
                    current_pos_.x, current_pos_.y, target_pos_.x, target_pos_.y);
        
        mapper_.printMap();
        
        final_path_ = pathfinder_.findPath(current_pos_, target_pos_);
        
        if (final_path_.empty())
        {
            RCLCPP_ERROR(this->get_logger(), "Caminho não encontrado!");
            RCLCPP_ERROR(this->get_logger(), "Verificando células: start=(%d,%d)='%c', goal=(%d,%d)='%c'",
                        current_pos_.x, current_pos_.y, mapper_.getCell(current_pos_.x, current_pos_.y),
                        target_pos_.x, target_pos_.y, mapper_.getCell(target_pos_.x, target_pos_.y));
            return;
        }
        
        RCLCPP_INFO(this->get_logger(), "Caminho encontrado! %zu movimentos", final_path_.size());
        
        path_index_ = 0;
        executing_final_path_ = true;
        
        finalPathTimer_ = this->create_wall_timer(
            std::chrono::milliseconds(200),
            std::bind(&ExplorerNode::executeFinalPathStep, this));
    }
    
    // Executa um passo do caminho final
    void executeFinalPathStep()
    {
        if (waiting_for_move_) return;
        
        if (path_index_ >= final_path_.size())
        {
            finalPathTimer_->cancel();
            RCLCPP_INFO(this->get_logger(), "ALVO ALCANÇADO! Movimentos totais: %d", move_count_);
            mapper_.printMap();
            return;
        }
        
        executeMove(final_path_[path_index_]);
        path_index_++;
    }
    
    // Variáveis de estado
    Mapper mapper_;
    Pathfinder pathfinder_;
    Position current_pos_;
    Position target_pos_;
    bool exploring_;
    int move_count_;
    bool waiting_for_move_;
    std::string pending_direction_;
    bool map_exploration_complete_ = false;
    
    bool executing_final_path_ = false;
    std::vector<std::string> final_path_;
    size_t path_index_ = 0;
    
    // Interfaces ROS
    rclcpp::Subscription<cg_interfaces::msg::RobotSensors>::SharedPtr sensor_sub_;
    rclcpp::Client<cg_interfaces::srv::MoveCmd>::SharedPtr move_client_;
    rclcpp::Client<cg_interfaces::srv::Reset>::SharedPtr reset_client_;
    rclcpp::TimerBase::SharedPtr exploreTimer_;
    rclcpp::TimerBase::SharedPtr finalPathTimer_;
};

} // namespace cg_explorer

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<cg_explorer::ExplorerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
