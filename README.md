# ROS2 Face Detection Service

基于 ROS2、OpenCV 与 face_recognition 的人脸检测服务项目。

该项目实现了：

- ROS2 Service 通信
- 人脸检测服务端 / 客户端
- HOG 与 CNN 模型动态切换
- OpenCV 可视化
- Benchmark 性能对比

---

# Features

- ROS2 Python 节点开发
- 自定义 Service 接口
- 动态参数修改
- HOG / CNN 模型切换
- OpenCV 图像处理
- face_recognition 人脸检测
- Client / Server 架构
- 人脸检测结果可视化

---

# Project Structure

```text
chapt4_ws
├── README.md
├── images
│   └── demo.png
├── src
│   ├── chapt4_interfaces
│   │   └── srv
│   │       └── FaceDetector.srv
│   │
│   └── demo_python_service
│       ├── face_detect_node.py
│       ├── face_detect_client_node.py
│       ├── learn_face_detect.py
│       ├── package.xml
│       ├── setup.py
│       └── resource
│           ├── default.jpg
│           └── test1.jpg
```

---

# Environment

- Ubuntu 22.04
- ROS2 Humble
- Python 3
- OpenCV
- face_recognition

---

# Dependencies

安装 OpenCV：

```bash
sudo apt install python3-opencv
```

安装 face_recognition：

```bash
pip3 install face_recognition
```

---

# Build

进入工作空间：

```bash
cd ~/chapt4/chapt4_ws
```

编译：

```bash
colcon build
```

加载环境：

```bash
source install/setup.bash
```

---

# Run Server

启动人脸检测服务端：

```bash
ros2 run demo_python_service face_detect_node
```

---

# Run Client

启动客户端：

```bash
ros2 run demo_python_service face_detect_client_node
```

---

# Dynamic Model Switching

项目支持动态切换：

- HOG 模型
- CNN 模型

客户端会自动：

1. 调用参数服务
2. 修改服务端模型参数
3. 请求人脸检测
4. 返回检测结果
5. 显示耗时与检测框

---

# HOG vs CNN

| Model | Feature |
|---|---|
| HOG | 检测速度快，CPU友好 |
| CNN | 检测精度更高，但耗时明显增加 |

---

# Demo

项目支持：

- 动态切换 HOG / CNN
- ROS2 参数动态更新
- OpenCV 检测结果显示
- Benchmark 性能测试

## Face Detection Result

![demo](images/demo1.png)

---

# Example Output

```text
模型: hog | 检测到 2 张脸 | 耗时 0.376s

模型: cnn | 检测到 2 张脸 | 耗时 58.942s
```

---

# ROS2 Concepts Used

本项目涉及：

- ROS2 Node
- ROS2 Service
- 自定义 srv 接口
- Parameter 动态参数
- Client / Server 通信
- OpenCV 图像处理
- CvBridge 图像转换

---

# Future Improvements

后续计划：

- 摄像头实时检测
- Launch 文件
- Qt GUI
- YOLOv8 集成
- 多线程处理
- GPU 推理加速

---

# Author

ICE

---

# License

Apache License 2.0