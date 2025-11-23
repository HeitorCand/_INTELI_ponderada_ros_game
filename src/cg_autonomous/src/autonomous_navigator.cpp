#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "cg_interfaces/srv/get_map.hpp"
#include "cg_interfaces/srv/move_cmd.hpp"
#include "cg_autonomous/pathfinding.hpp"

using namespace std::chrono_literals;

class AutonomousNavigator : public rclcpp::Node {
public:
    AutonomousNavigator() : Node("autonomous_navigator_cpp") {
        map_client_ = this->create_client<cg_interfaces::srv::GetMap>("/get_map");
        move_client_ = this->create_client<cg_interfaces::srv::MoveCmd>("/move_command");
        
        waitForServices();
    }
    
    // Executa navegação completa
    void navigate() {
        auto map_data = getMap();
        if (!map_data.has_value()) {
            return;
        }
        
        auto [grid, robot_pos, target_pos] = map_data.value();
        
        auto path = calculatePath(grid, robot_pos, target_pos);
        if (!path.has_value()) {
            return;
        }
        
        executePath(path.value());
    }

private:
    void waitForServices() {
        while (!map_client_->wait_for_service(1s) && rclcpp::ok()) {}
        while (!move_client_->wait_for_service(1s) && rclcpp::ok()) {}
    }
    
    std::optional<std::tuple<
        std::vector<std::vector<std::string>>,
        cg_autonomous::Position,
        cg_autonomous::Position
    >> getMap() {
        auto request = std::make_shared<cg_interfaces::srv::GetMap::Request>();
        auto result_future = map_client_->async_send_request(request);
        
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) !=
            rclcpp::FutureReturnCode::SUCCESS) {
            return std::nullopt;
        }
        
        auto response = result_future.get();
        int rows = response->occupancy_grid_shape[0];
        int cols = response->occupancy_grid_shape[1];
        
        std::vector<std::string> flattened(
            response->occupancy_grid_flattened.begin(),
            response->occupancy_grid_flattened.end()
        );
        
        auto grid = cg_autonomous::Pathfinding::unflattenGrid(flattened, rows, cols);
        auto robot_pos = cg_autonomous::Pathfinding::findInGrid(grid, "r");
        auto target_pos = cg_autonomous::Pathfinding::findInGrid(grid, "t");
        
        if (!robot_pos.has_value() || !target_pos.has_value()) {
            return std::nullopt;
        }
        
        return std::make_tuple(grid, robot_pos.value(), target_pos.value());
    }
    
    std::optional<std::vector<cg_autonomous::Position>> calculatePath(
        const std::vector<std::vector<std::string>>& grid,
        const cg_autonomous::Position& start,
        const cg_autonomous::Position& goal
    ) {
        return cg_autonomous::Pathfinding::astar(grid, start, goal);
    }
    
    bool moveRobot(const std::string& direction) {
        auto request = std::make_shared<cg_interfaces::srv::MoveCmd::Request>();
        request->direction = direction;
        auto result_future = move_client_->async_send_request(request);
        
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) !=
            rclcpp::FutureReturnCode::SUCCESS) {
            return false;
        }
        return result_future.get()->success;
    }
    
    bool executePath(const std::vector<cg_autonomous::Position>& path) {
        if (path.size() < 2) return false;
        
        auto directions = cg_autonomous::Pathfinding::pathToDirections(path);
        
        for (const auto& dir : directions) {
            if (!moveRobot(dir)) {
                return false;
            }
            rclcpp::sleep_for(30ms);
        }
        return true;
    }
    
    rclcpp::Client<cg_interfaces::srv::GetMap>::SharedPtr map_client_;
    rclcpp::Client<cg_interfaces::srv::MoveCmd>::SharedPtr move_client_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto navigator = std::make_shared<AutonomousNavigator>();
    navigator->navigate();
    rclcpp::shutdown();
    return 0;
}
