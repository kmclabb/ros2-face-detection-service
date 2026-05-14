import face_recognition
import cv2
from ament_index_python.packages import (
    get_package_share_directory
)

def main():

    default_image_path = (
        get_package_share_directory(
            'demo_python_service'
        ) + '/resource/default.jpg'
    )

    # 读取图像
    image = cv2.imread(default_image_path)

    if image is None:

        print("图像加载失败")

        return

    # BGR -> RGB
    rgb_image = cv2.cvtColor(
        image,
        cv2.COLOR_BGR2RGB
    )

    # 人脸检测
    face_locations = face_recognition.face_locations(
        rgb_image,
        number_of_times_to_upsample=1,
        model='hog'
    )

    print(f"检测到 {len(face_locations)} 张脸")

    # 绘制边框
    for top, right, bottom, left in face_locations:

        cv2.rectangle(
            image,
            (left, top),
            (right, bottom),
            (0, 0, 255),
            4
        )

    # 等比例缩放
    scale = 0.5

    resize_image = cv2.resize(
        image,
        None,
        fx=scale,
        fy=scale
    )

    cv2.imshow(
        'Face Detect Result',
        resize_image
    )

    cv2.waitKey(0)

    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()