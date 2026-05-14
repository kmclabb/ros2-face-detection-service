import rclpy
from rclpy.node import Node
from chapt4_interfaces.srv import FaceDetector
import cv2
from ament_index_python.packages import get_package_share_directory
from cv_bridge import CvBridge
from rcl_interfaces.srv import SetParameters
from rcl_interfaces.msg import (
    Parameter,
    ParameterValue,
    ParameterType
)
import time

class FaceDetectClientNode(Node):
    def __init__(self):
        super().__init__('face_detect_client_node')
        self.bridge = CvBridge()
        self.default_image_path = (
            get_package_share_directory(
                'demo_python_service'
            ) + '/resource/test1.jpg'
        )
        self.get_logger().info(
            "人脸检测客户端已经启动"
        )
        self.client = self.create_client(
            FaceDetector,
            "face_detect"
        )
        self.image = cv2.imread(
            self.default_image_path
        )

        self.current_model = 'hog'

    def call_set_parameters(self, parameters):
        update_param_client = self.create_client(
            SetParameters,
            '/face_detect_node/set_parameters'
        )
        while not update_param_client.wait_for_service(
            timeout_sec=1.0
        ):
            self.get_logger().info(
                "等待参数更新服务端上线"
            )

        request = SetParameters.Request()
        request.parameters = parameters
        future = update_param_client.call_async(
            request
        )
        rclpy.spin_until_future_complete(
            self,
            future
        )
        return future.result()

    def update_detect_model(self, model='hog'):
        self.current_model = model
        param = Parameter()
        param.name = 'model'
        param_value = ParameterValue()
        param_value.string_value = model
        param_value.type = (
            ParameterType.PARAMETER_STRING
        )
        param.value = param_value
        response = self.call_set_parameters(
            [param]
        )
        for result in response.results:
            self.get_logger().info(
                f"设置参数结果:"
                f"{result.successful} "
                f"{result.reason}"
            )

    def send_request(self):
        while not self.client.wait_for_service(
            timeout_sec=1.0
        ):
            self.get_logger().info(
                "等待服务端上线"
            )

        # 每次重新复制原图
        self.show_image = self.image.copy()

        request = FaceDetector.Request()

        request.image = self.bridge.cv2_to_imgmsg(
            self.show_image
        )

        future = self.client.call_async(request)

        rclpy.spin_until_future_complete(
            self,
            future
        )

        response = future.result()

        if response is None:

            self.get_logger().error(
                "服务调用失败"
            )

            return

        self.get_logger().info(
            f'模型: {self.current_model} | '
            f'检测到 {response.number} 张脸 | '
            f'耗时 {response.use_time:.3f}s'
        )

        self.show_response(response)

    def show_response(self, response):

        for i in range(len(response.top)):

            top = response.top[i]
            right = response.right[i]
            bottom = response.bottom[i]
            left = response.left[i]

            cv2.rectangle(
                self.show_image,
                (left, top),
                (right, bottom),
                (0, 0, 255),
                4
            )

        # 添加文字
        cv2.putText(
            self.show_image,
            f'{self.current_model} '
            f'{response.use_time:.3f}s',
            (20, 40),
            cv2.FONT_HERSHEY_SIMPLEX,
            1,
            (0, 255, 0),
            2
        )

        resize_image = cv2.resize(
            self.show_image,
            (800, 600)
        )

        cv2.imshow(
            f'Face Detect Result - {self.current_model}',
            resize_image
        )

        # 等待3秒
        cv2.waitKey(1)

        time.sleep(3)

    def destroy_node(self):

        cv2.destroyAllWindows()

        super().destroy_node()


def main():

    rclpy.init()

    node = FaceDetectClientNode()

    # hog测试
    node.update_detect_model('hog')
    node.send_request()

    # cnn测试
    node.update_detect_model('cnn')
    node.send_request()

    node.destroy_node()

    rclpy.shutdown()