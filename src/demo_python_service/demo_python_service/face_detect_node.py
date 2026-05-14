import rclpy
from rclpy.node import Node
from chapt4_interfaces.srv import FaceDetector
import face_recognition
import cv2
import time
from ament_index_python.packages import get_package_share_directory
from cv_bridge import CvBridge
from rcl_interfaces.msg import SetParametersResult

class FaceDetectNode(Node):
    def __init__(self):
        super().__init__('face_detect_node')
        self.bridge = CvBridge()
        self.default_image_path = (
            get_package_share_directory('demo_python_service')
            + '/resource/default.jpg'
        )

        self.get_logger().info("人脸检测服务已经启动")

        # 参数
        self.declare_parameter('number_of_times_to_upsample', 1)
        self.declare_parameter('model', 'hog')

        self.number_of_times_to_upsample = (
            self.get_parameter(
                'number_of_times_to_upsample'
            ).value
        )

        self.model = self.get_parameter('model').value

        # 创建服务
        self.service = self.create_service(
            FaceDetector,
            'face_detect',
            self.detect_face_callback
        )

        # 动态参数回调
        self.add_on_set_parameters_callback(
            self.parameters_callback
        )

    def parameters_callback(self, parameters):

        for parameter in parameters:

            self.get_logger().info(
                f"{parameter.name} -> {parameter.value}"
            )

            if parameter.name == 'number_of_times_to_upsample':
                self.number_of_times_to_upsample = parameter.value

            if parameter.name == 'model':

                if parameter.value not in ['hog', 'cnn']:
                    return SetParametersResult(
                        successful=False,
                        reason='model must be hog or cnn'
                    )

                self.model = parameter.value

        return SetParametersResult(successful=True)

    def detect_face_callback(self, request, response):

        try:

            # 接收到图像
            if request.image.data:

                cv_image = self.bridge.imgmsg_to_cv2(
                    request.image
                )

            else:

                cv_image = cv2.imread(
                    self.default_image_path
                )

                self.get_logger().info(
                    "传入图像为空，使用默认图像"
                )

            # 判空
            if cv_image is None:

                self.get_logger().error(
                    "图像加载失败"
                )

                return response

            # BGR -> RGB
            rgb_image = cv2.cvtColor(
                cv_image,
                cv2.COLOR_BGR2RGB
            )

            self.get_logger().info(
                f"开始使用 {self.model} 模型检测人脸"
            )

            start_time = time.time()

            # 人脸检测
            face_locations = face_recognition.face_locations(
                rgb_image,
                number_of_times_to_upsample=self.number_of_times_to_upsample,
                model=self.model
            )

            use_time = time.time() - start_time

            response.use_time = use_time
            response.number = len(face_locations)

            # 返回坐标
            for top, right, bottom, left in face_locations:

                response.top.append(top)
                response.right.append(right)
                response.bottom.append(bottom)
                response.left.append(left)

            self.get_logger().info(
                f"检测完成：{response.number} 张脸 "
                f"耗时 {use_time:.3f}s"
            )

        except Exception as e:

            self.get_logger().error(str(e))

        return response


def main():
    rclpy.init()
    node = FaceDetectNode()
    rclpy.spin(node)
    rclpy.shutdown()