#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include "geometry_msgs/Twist.h"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"


// DEfine a glibal client that can request services
ros::ServiceClient client;


// This function call the command_robo service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z){
    // TODO: Request a service and pass velocities to it to drive the robot
    ball_chaser::DriveToTarget::Request req;
    ball_chaser::DriveToTarget::Response resp;

    req.angular_z = ang_z;
    req.linear_x = lin_x;


    if(client.call(req, resp)){
        ROS_INFO_STREAM(resp.msg_feedback);
    }else{
        ROS_INFO_STREAM("Failed to request-> linear_x: " << std::to_string(lin_x) << " angular_z: " << std::to_string(ang_z) << "\n");
    };
};

// This callback function continiously executes and read the image data
// if the white ball is in the center of the image the robot should move forward
// if the white ball is in the right side of the image the robot should turn right
// if the white ball is in the left side of the image the robot should turn left
// if there is no ball in the image robot should stop
void process_image_callback(const sensor_msgs::ImageConstPtr& img){
    uint8_t white_pixel = 200; // This values 


    // Convert the ROS image message to an OpenCV image
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(img, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("Could not convert image message to OpenCV image: %s", e.what());
        return;
    }

    // Do something with the OpenCV image
    //cv::Mat image = cv_ptr->image;
    //int width = image.cols;
    //int height = image.rows;

    //ROS_INFO_STREAM("cv::Mat Image width " << width << "\n");
    //ROS_INFO_STREAM("cv::Mat Image height  " << height << "\n");  

    // Define variables white pixel detection loops
    bool white_pixel_found = false;
    int white_pixel_x, white_pixel_y;

    // Find white pixel and save it's x,y values
    for(int y =0; y < cv_ptr->image.rows; y++){
        for(int x = 0; x < cv_ptr->image.cols; x++){
            cv::Vec3b pixel= cv_ptr->image.at<cv::Vec3b>(y,x);
            // CHeck if there is a white pixel at (i,j)
            if(pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255){
                white_pixel_found = true;
                white_pixel_x = x;
                white_pixel_y = y;
                break;
            }
        }
    }

    // Divide the image into three eqaully size vertical sections 
    int section_width = cv_ptr->image.cols /3;
    int section_1_x = section_width;
    int section_2_x = section_width*2;
    int section_3_x = section_width*3;

    if(white_pixel_found){
        if(white_pixel_x < section_1_x){
            ROS_INFO("White Pixel is on the left side");
            drive_robot(0.0, 0.2);
            return;
        }else if (white_pixel_x < section_2_x){
            ROS_INFO("White Pixel is in the middle");
            drive_robot(0.2, 0.0);
            return;

        }else if (white_pixel_x < section_3_x){
            ROS_INFO("White Pixel is on the right side");
            drive_robot(0.0, -0.2);
            return;
        }   
    }else{
        drive_robot(0.0, 0.0);
    }


/*
    if(pixelValue > white_pixel){
                ROS_INFO_STREAM("Detected white Pixel at: (" << i << "," << j<< ")\n");
                if((j % width) < (width / 3) ){
                    drive_robot(0.0, 0.2);
                    return;
                }else if ((j % width) > (width / 3 * 2) ){
                    drive_robot(0.0, -0.2);
                    return;
                }else{
                    drive_robot(0.2, 0.0);
                    return;
                }
        }
     drive_robot(0.0, 0.0);
     return;*/
     return;
};

int main(int argc, char** argv){
    // Init the node and create a handle for it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // DEfine a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to Image /camera/rgb/image_raw 
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 1, process_image_callback);

    ros::spin();

    return 0;


}