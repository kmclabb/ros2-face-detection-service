#include <chrono>
#include <cstdlib>
#include <memory>
#include <random>

#include "rclcpp/rclcpp.hpp"
#include "chapt4_interfaces/srv/patrol.hpp"

#include "rcl_interfaces/msg/parameter.hpp"
#include "rcl_interfaces/msg/parameter_value.hpp"
#include "rcl_interfaces/msg/parameter_type.hpp"
#include "rcl_interfaces/srv/set_parameters.hpp"

using namespace std::chrono_literals;

using Patrol = chapt4_interfaces::srv::Patrol;
using SetP = rcl_interfaces::srv::SetParameters;

class TurtleControlClientNode : public rclcpp::Node
{
public:
    TurtleControlClientNode(const std::string & node_name)
    : Node(node_name)
    {
        RCLCPP_INFO(this->get_logger(), "节点: %s 启动！", node_name.c_str());

        // 巡逻服务客户端
        turtle_client_ = this->create_client<Patrol>("patrol");

        while (!turtle_client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "等待巡逻服务时被打断");
                return;
            }
            RCLCPP_INFO(this->get_logger(), "等待巡逻服务端上线......");
        }

        // 参数服务客户端（只创建一次，规范写法）
        param_client_ = this->create_client<SetP>("/turtle_controller/set_parameters");

        // 定时器
        timer_ = this->create_wall_timer(
            10s,
            std::bind(&TurtleControlClientNode::send_request, this)
        );
    }

    // 对外开放：启动时修改一次参数
    void update_server_param_k(double k)
    {
        auto param = rcl_interfaces::msg::Parameter();
        param.name = "k";

        auto param_value = rcl_interfaces::msg::ParameterValue();
        param_value.type =
            rcl_interfaces::msg::ParameterType::PARAMETER_DOUBLE;
        param_value.double_value = k;

        param.value = param_value;

        auto response = call_set_parameters(param);

        if (response == nullptr) {
            RCLCPP_WARN(this->get_logger(), "参数 k 修改失败");
            return;
        }

        for (const auto & result : response->results) {
            if (result.successful) {
                RCLCPP_INFO(this->get_logger(), "参数 k 已修改为: %f", k);
            } else {
                RCLCPP_WARN(
                    this->get_logger(),
                    "参数 k 修改失败，原因: %s",
                    result.reason.c_str()
                );
            }
        }
    }

private:
    void send_request()
    {
        if (!turtle_client_->wait_for_service(500ms)) {
            RCLCPP_WARN(this->get_logger(), "服务端断线中，暂停请求");
            return;
        }

        auto request = std::make_shared<Patrol::Request>();

        request->target_x = random_double(1.0, 12.0);
        request->target_y = random_double(1.0, 12.0);

        RCLCPP_INFO(
            this->get_logger(),
            "随机目标点: x = %.2f, y = %.2f",
            request->target_x,
            request->target_y
        );

        turtle_client_->async_send_request(
            request,
            std::bind(
                &TurtleControlClientNode::result_callback,
                this,
                std::placeholders::_1
            )
        );

        // ❌ 不再在这里调用参数修改（避免阻塞）
    }

    void result_callback(rclcpp::Client<Patrol>::SharedFuture future)
    {
        try {
            auto response = future.get();
            show_response(response);
        } catch (const std::exception & e) {
            RCLCPP_ERROR(this->get_logger(), "服务回应异常: %s", e.what());
        }
    }

    void show_response(const std::shared_ptr<Patrol::Response> response)
    {
        RCLCPP_INFO(
            this->get_logger(),
            "服务返回 result: %d",
            response->result
        );

        if (response->result == Patrol::Response::SUCCESS) {
            RCLCPP_INFO(this->get_logger(), "目标点设置成功");
        } else {
            RCLCPP_WARN(this->get_logger(), "目标点非法，设置失败");
        }
    }

    std::shared_ptr<SetP::Response> call_set_parameters(
        rcl_interfaces::msg::Parameter & parameter)
    {
        while (!param_client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "等待参数服务被打断");
                return nullptr;
            }
            RCLCPP_INFO(this->get_logger(), "等待参数服务上线...");
        }

        auto request = std::make_shared<SetP::Request>();
        request->parameters.push_back(parameter);

        auto future = param_client_->async_send_request(request);

        auto result = rclcpp::spin_until_future_complete(
            this->get_node_base_interface(),
            future
        );

        if (result != rclcpp::FutureReturnCode::SUCCESS) {
            RCLCPP_WARN(this->get_logger(), "参数服务调用失败");
            return nullptr;
        }

        return future.get();
    }

    double random_double(double min, double max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(min, max);
        return dis(gen);
    }

private:
    rclcpp::Client<Patrol>::SharedPtr turtle_client_;
    rclcpp::Client<SetP>::SharedPtr param_client_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<TurtleControlClientNode>(
        "turtle_control_client_node"
    );

    // 启动时修改一次参数（安全）
    node->update_server_param_k(1.5);

    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}