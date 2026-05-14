#include "chapt4_interfaces/srv/patrol.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "turtlesim/msg/pose.hpp"
#include "rclcpp/rclcpp.hpp"
#include <cmath>
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rclcpp/parameter_event_handler.hpp"
using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

using Patrol = chapt4_interfaces::srv::Patrol;

class TurtleController : public rclcpp::Node {
private:
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr subscription_;
    rclcpp::Service<Patrol>::SharedPtr patrol_server_;
    double k_{1.0};
    double max_speed_{3.0};
    float target_x_ = 6.0f;
    float target_y_ = 6.0f;
     OnSetParametersCallbackHandle::SharedPtr parameters_callback_handle_;

public:
    TurtleController() : rclcpp::Node("turtle_controller") 
    { 
        // 参数声明
        this->declare_parameter("k", 1.0);
        this->declare_parameter("max_speed", 1.0);
        this->get_parameter("k", k_);
        this->get_parameter("max_speed", max_speed_);
        
        // 创建巡逻服务
        patrol_server_ = this->create_service<Patrol>(
            "patrol",
            [&](const std::shared_ptr<Patrol::Request> request,
                std::shared_ptr<Patrol::Response> response) {
                if ((0 < request->target_x && request->target_x < 12.0f) &&
                    (0 < request->target_y && request->target_y < 12.0f)) {
                    this->target_x_ = request->target_x;
                    this->target_y_ = request->target_y;
                    response->result = Patrol::Response::SUCCESS;
                } else {
                    response->result = Patrol::Response::FAIL;
                }
            }
        );
        
        // 添加参数回调（修正位置）
        parameters_callback_handle_ = this->add_on_set_parameters_callback(
            [&](const std::vector<rclcpp::Parameter> &params)
                -> SetParametersResult {
                for (auto param : params) {
                    RCLCPP_INFO(
                        this->get_logger(),
                        "更新参数 %s 值为：%f",
                        param.get_name().c_str(),
                        param.as_double());
                    if (param.get_name() == "k") {
                        k_ = param.as_double();
                    } else if (param.get_name() == "max_speed") {
                        max_speed_ = param.as_double();
                    }
                }
                auto result = SetParametersResult();
                result.successful = true;
                return result;
            }
        );
        // 创建速度发布者和位置订阅者
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("turtle1/cmd_vel", 10);
        subscription_ = this->create_subscription<turtlesim::msg::Pose>(
            "turtle1/pose",
            10,
            std::bind(&TurtleController::pose_callback, this, std::placeholders::_1)
        );
    }

private:
    void pose_callback(const turtlesim::msg::Pose::SharedPtr pose){
        //1、获取当前与龟的位置
        auto current_x = pose->x;
        auto current_y = pose->y;
        auto message = geometry_msgs::msg::Twist();
        RCLCPP_INFO(this->get_logger(), "当前x:%.2f, 当前y:%.2f", current_x, current_y);
        //2、计算与目标点的距离
        auto distance = std::sqrt(std::pow((target_x_ - current_x), 2) + std::pow((target_y_ - current_y), 2));
        RCLCPP_INFO(this->get_logger(), "距离目标点的距离:%.2f", distance);
        //3、计算角度差
        auto angle_to_target = std::atan2(target_y_ - current_y, target_x_ - current_x);
        auto angle_diff = angle_to_target - pose->theta;
        RCLCPP_INFO(this->get_logger(), "角度差:%.2f", angle_diff);
        //4、计算线速度和角速度
        if (distance > 0.1){
            if (fabs(angle_diff) > 0.1) {
                message.angular.z = fabs(angle_diff);
            }else {
                message.linear.x = k_ * distance;
            }
        }
        //5、限制最大值并发布速度
        if (message.linear.x > max_speed_) {
            message.linear.x = max_speed_;
        }
        publisher_->publish(message);
    }
};

int main(int argc, char **argv){
    rclcpp::init(argc, argv);
    auto controller = std::make_shared<TurtleController>();
    rclcpp::spin(controller);
    rclcpp::shutdown();
    return 0;
}